CLANG ?= clang
BPFTOOL ?= bpftool

ARCH := $(shell uname -m | sed 's/x86_64/x86/')

# include common user and kernel headers 
BPF_INCLUDES := -I/usr/include -I/usr/include/$(shell dpkg-architecture -qDEB_HOST_MULTIARCH) -I/usr/src/linux-headers-$(shell uname -r)/include

all: filter_traffic.bpf.o

filter_traffic.bpf.o: filter_traffic.bpf.c
	$(CLANG) -O2 -target bpf -D__TARGET_ARCH_$(ARCH) $(BPF_INCLUDES) -c $< -o $@

clean:
	rm -f *.o
	sudo rm -f /sys/fs/bpf/filter_traffic

load_filter: filter_traffic.bpf.o
	sudo bpftool prog load filter_traffic.bpf.o /sys/fs/bpf/filter_traffic
	sudo bpftool prog attach /sys/fs/bpf/filter_traffic kprobe security_socket_connect

unload:
	sudo rm -f / /sys/fs/bpf/filter_traffic

logs:
	sudo cat /sys/kernel/debug/tracing/trace_pipe

.PHONY: all clean load_filter unload logs