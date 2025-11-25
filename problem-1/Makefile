# Makefile for eBPF TCP Packet Dropper

CLANG ?= clang
ARCH := $(shell uname -m | sed 's/x86_64/x86/')

BPF_OBJ = tcp_drop.bpf.o
USER_BIN = tcp_drop_user

all: $(BPF_OBJ) $(USER_BIN)

$(BPF_OBJ): tcp_drop.bpf.c
	$(CLANG) -O2 -g -target bpf -D__TARGET_ARCH_$(ARCH) -c $< -o $@

$(USER_BIN): tcp_drop_user.c
	gcc -O2 -g -Wall $< -o $@ -lbpf

clean:
	rm -f $(BPF_OBJ) $(USER_BIN)

.PHONY: all clean
