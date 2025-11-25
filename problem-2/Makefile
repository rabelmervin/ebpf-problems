CLANG ?= clang
BPFTOOL ?= bpftool

ARCH := $(shell uname -m | sed 's/x86_64/x86/')
BPF_HEADERS := /usr/include

all: filter_traffic.bpf.o

filter_traffic.bpf.o: filter_traffic.bpf.c
	$(CLANG) -O2 -target bpf -D__TARGET_ARCH_$(ARCH) -I$(BPF_HEADERS) -c $< -o $@

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