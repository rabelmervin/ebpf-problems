#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

char LICENSE[] SEC("license") = "Dual BSD/GPL";

SEC("kprobe/tcp_connect") 
int kprobe_tcp_connect(void *ctx)
{
    char comm[16];
    bpf_get_current_comm(&comm, sizeof(comm));
    
    // Simple string comparison for "myprocess"
    if (comm[0] == 'm' && comm[1] == 'y' && comm[2] == 'p' && comm[3] == 'r' &&
        comm[4] == 'o' && comm[5] == 'c' && comm[6] == 'e' && comm[7] == 's' &&
        comm[8] == 's') {
        bpf_printk("Process myprocess detected - would block non-4040 ports");
    }
    
    return 0;
}