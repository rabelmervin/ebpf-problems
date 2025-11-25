// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* tcp_drop.bpf.c - Drop TCP packets on configurable port */

#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

/* Map to store the port number (configurable from userspace) */
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32);
    __type(value, __u16);
} port_map SEC(".maps");

/* Statistics map */
struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(max_entries, 1);
    __type(key, __u32);
    __type(value, __u64);
} stats_map SEC(".maps");

SEC("xdp")
int xdp_drop_tcp_port(struct xdp_md *ctx)
{
    void *data_end = (void *)(long)ctx->data_end;
    void *data = (void *)(long)ctx->data;
    struct ethhdr *eth;
    struct iphdr *ip;
    struct tcphdr *tcp;
    __u32 key = 0;
    __u16 *target_port;
    __u16 port_to_drop;
    __u16 dst_port, src_port;
    __u64 *counter;

    /* Parse Ethernet header */
    eth = data;
    if ((void *)(eth + 1) > data_end)
        return XDP_PASS;

    /* Only process IPv4 */
    if (eth->h_proto != bpf_htons(ETH_P_IP))
        return XDP_PASS;

    /* Parse IP header */
    ip = (void *)(eth + 1);
    if ((void *)(ip + 1) > data_end)
        return XDP_PASS;

    /* Only process TCP */
    if (ip->protocol != IPPROTO_TCP)
        return XDP_PASS;

    /* Parse TCP header */
    tcp = (void *)ip + (ip->ihl * 4);
    if ((void *)(tcp + 1) > data_end)
        return XDP_PASS;

    /* Get configured port */
    target_port = bpf_map_lookup_elem(&port_map, &key);
    port_to_drop = target_port ? *target_port : 4040;

    /* Get source and destination ports */
    dst_port = bpf_ntohs(tcp->dest);
    src_port = bpf_ntohs(tcp->source);

    /* Drop if port matches */
    if (dst_port == port_to_drop || src_port == port_to_drop) {
        counter = bpf_map_lookup_elem(&stats_map, &key);
        if (counter)
            __sync_fetch_and_add(counter, 1);
        return XDP_DROP;
    }

    return XDP_PASS;
}

char _license[] SEC("license") = "GPL";
