#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

#define ALLOWED_PORT 4040

SEC("cgroup/connect4")
int allow_only_port_4040(struct bpf_sock_addr *ctx)
{

    char comm[16];
    bpf_get_current_comm(&comm, sizeof(comm));

    // Only filter for process named "myprocess"
    if (comm[0] == 'm' && comm[1] == 'y' && comm[2] == 'p' && comm[3] == 'r' &&
        comm[4] == 'o' && comm[5] == 'c' && comm[6] == 'e' && comm[7] == 's' &&
        comm[8] == 's') {
        if (ctx->user_port != bpf_htons(ALLOWED_PORT)) {
            return 1; // drop
        }
    }
    return 0; // allow
}