**Overview**
- **Repo:** eBPF assignment problems.
- **Location:** `problem-1/` and `problem-2/`.

**Prerequisites**
- **Root:** many commands require `sudo`.
- **Packages:** install development and eBPF tooling:
  - `sudo apt update && sudo apt install -y build-essential clang llvm libelf-dev libbpf-dev bpftool linux-headers-$(uname -r) cgroup-tools`
- **Notes:** On some cloud images kernel headers are partial; follow the "Troubleshooting" section if `make` complains about missing kernel headers.

**Problem 1 (XDP TCP-drop)**
- **Statement:** - "Drop packets using eBPF — Write an eBPF code to drop the TCP packets on a port (def: 4040). Additionally, if you can make the port number configurable from the userspace, that will be a big plus."

Quick test (minimal):

- Build and run the test client/server (minimal steps):
  - `cd problem-1`
  - `make`
  - In terminal A (server): `python3 -m http.server 4040`
  - In terminal B (client / in the same VM): get the VM IP with `hostname -I` and use that IP from the host, or run the client locally:
    - `sudo ./tcp_drop_user <server-ip-or-iface> 4040`
    - Example using an interface name: `sudo ./tcp_drop_user enp0s3 4040`
  - If you're testing across VM/host, run `hostname -I` inside the VM to get its IP address and use that from the host to send traffic so the XDP program sees packets on the interface.

- Test:
  - Run a TCP server: `python3 -m http.server 4040`
  - Run client: `./tcp_drop_user <server-ip> 4040` (should succeed) and `./tcp_drop_user <server-ip> 8080` (should be dropped if configured)


**Problem 2 (cgroup / inet4_connect filter)**
- **Statement:**"Drop packets only for a given process — Write an eBPF code to allow traffic only at a specific TCP port (default 4040) for a given process name (for e.g, \"myprocess\"). All the traffic to all other ports for only that process should be dropped."

- Build:
  - `cd problem-2`
  - `make`

- Prepare cgroup and pin program:
  - Create a cgroup: `sudo mkdir -p /sys/fs/cgroup/mygrp`
  - (Optional) put a process into the cgroup manually: `echo <pid> | sudo tee /sys/fs/cgroup/mygrp/cgroup.procs`
  - Load & pin the BPF program:
    - `sudo bpftool prog load filter_traffic.bpf.o /sys/fs/bpf/filter_traffic`
  - Attach to the cgroup (socket connect hook):
    - `sudo bpftool cgroup attach /sys/fs/cgroup/mygrp cgroup_inet4_connect pinned /sys/fs/bpf/filter_traffic`

- Test:
  - Start a server on allowed port: `python3 problem-2/tcp_server.py 4040`
  - Run your client inside the cgroup: `./myprocess <server-ip> 4040`

- Detach / cleanup:
  - Detach pinned program from cgroup: `sudo bpftool cgroup detach /sys/fs/cgroup/mygrp cgroup_inet4_connect all`
  - Remove pinned program: `sudo rm /sys/fs/bpf/filter_traffic`
  - Remove cgroup: `sudo rmdir /sys/fs/cgroup/mygrp`

**Troubleshooting**
- If `make` fails with `asm/types.h` or similar missing-header errors:
  - Ensure `linux-headers-$(uname -r)` is installed: `sudo apt install linux-headers-$(uname -r)`.
  - Ensure `libbpf-dev` and `bpftool` are installed (`libbpf-dev` provides `bpf_helpers.h` on some distros).
  - If headers are missing on the image, you can copy needed headers from `/usr/src/linux-source-*` into a local `include/` and add `-I$(PWD)/include` to the Makefile, but this is brittle — installing the proper dev packages is preferred.

- If `bpftool cgroup attach` fails with "failed to attach program":
  - Confirm the pinned program type is `cgroup_sock_addr` and has the correct section (e.g., `cgroup/inet4_connect`).
  - Verify cgroup path exists and is the correct cgroup fs (v2 vs v1 differences).
  - Use `sudo bpftool prog show` and `sudo bpftool prog show pinned /sys/fs/bpf/filter_traffic` to inspect program type and ID.