#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>

static int ifindex = 0;
static struct bpf_object *obj = NULL;
static struct bpf_link *bpf_link = NULL;

static void cleanup(int sig)
{
    printf("\n\nCleaning up...\n");
    
    if (bpf_link) {
        bpf_link__destroy(bpf_link);
        bpf_link = NULL;
    }
    
    if (obj) {
        bpf_object__close(obj);
        obj = NULL;
    }
    
    printf("Detached XDP program from interface\n");
    exit(0);
}

int main(int argc, char **argv)
{
    struct bpf_program *prog;
    int port_map_fd, stats_map_fd;
    int err;
    __u16 port = 4040;
    __u32 key = 0;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <interface> [port]\n", argv[0]);
        fprintf(stderr, "Example: sudo %s eth0 4040\n", argv[0]);
        return 1;
    }

    /* Get interface index */
    ifindex = if_nametoindex(argv[1]);
    if (ifindex == 0) {
        fprintf(stderr, "Error: Interface %s not found\n", argv[1]);
        return 1;
    }

    /* Get port if specified */
    if (argc == 3) {
        port = (unsigned short)atoi(argv[2]);
        if (port == 0) {
            fprintf(stderr, "Error: Invalid port number\n");
            return 1;
        }
    }

    printf("===========================================\n");
    printf("eBPF TCP Port Dropper\n");
    printf("===========================================\n");
    printf("Interface: %s (index: %d)\n", argv[1], ifindex);
    printf("Dropping TCP packets on port: %u\n", port);
    printf("===========================================\n\n");

    /* Set libbpf strict mode */
    libbpf_set_strict_mode(LIBBPF_STRICT_ALL);

    /* Open BPF object */
    obj = bpf_object__open_file("tcp_drop.bpf.o", NULL);
    err = libbpf_get_error(obj);
    if (err) {
        fprintf(stderr, "Error: Failed to open BPF object: %d\n", err);
        return 1;
    }

    /* Load BPF object into kernel */
    err = bpf_object__load(obj);
    if (err) {
        fprintf(stderr, "Error: Failed to load BPF object: %d\n", err);
        goto cleanup;
    }

    /* Find XDP program */
    prog = bpf_object__find_program_by_name(obj, "xdp_drop_tcp_port");
    if (!prog) {
        fprintf(stderr, "Error: XDP program not found\n");
        goto cleanup;
    }

    /* Get map file descriptors */
    port_map_fd = bpf_object__find_map_fd_by_name(obj, "port_map");
    stats_map_fd = bpf_object__find_map_fd_by_name(obj, "stats_map");
    
    if (port_map_fd < 0 || stats_map_fd < 0) {
        fprintf(stderr, "Error: Failed to find maps\n");
        goto cleanup;
    }

    /* Update port in map */
    err = bpf_map_update_elem(port_map_fd, &key, &port, BPF_ANY);
    if (err) {
        fprintf(stderr, "Error: Failed to update port map: %d\n", err);
        goto cleanup;
    }

    /* Attach XDP program */
    bpf_link = bpf_program__attach_xdp(prog, ifindex);
    err = libbpf_get_error(bpf_link);
    if (err) {
        fprintf(stderr, "Error: Failed to attach XDP: %d\n", err);
        fprintf(stderr, "Hint: Make sure you have root privileges\n");
        bpf_link = NULL;
        goto cleanup;
    }

    printf("✓ XDP program attached successfully!\n");
    printf("✓ Now dropping TCP traffic on port %u\n\n", port);
    printf("Press Ctrl+C to stop and detach\n\n");

    /* Setup signal handlers */
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    /* Display statistics every 5 seconds */
    while (1) {
        sleep(5);
        
        __u64 count = 0;
        err = bpf_map_lookup_elem(stats_map_fd, &key, &count);
        if (err == 0) {
            printf("[Stats] Packets dropped: %llu\n", count);
        }
    }

cleanup:
    if (bpf_link)
        bpf_link__destroy(bpf_link);
    if (obj)
        bpf_object__close(obj);
    return 1;
}
