# PussyOS
A custom x86_64 operating system built from scratch.

---

## Requirements
- QEMU installed
- A TAP network adapter on the host, bridged to your physical Ethernet interface (the one connected to your router via cable). Wi-Fi bridging will not work reliably, use a wired connection.

---

## Network setup

### Windows

Before running the OS you need to create a TAP adapter and bridge it to your Ethernet port.

**1. Install a TAP driver**

Download and install OpenVPN (or just the TAP-Windows driver standalone). This creates a virtual TAP network adapter in your system.

**2. Bridge TAP to Ethernet**

Open Network Connections (`ncpa.cpl`), select both your TAP adapter and your physical Ethernet adapter, right-click and choose "Bridge Connections". Windows will create a Network Bridge device.

Your TAP adapter will now share the same network as your wired connection and the OS will be able to reach the internet and your local network.

**3. Rename the TAP interface to `tap0` (optional)**

In Network Connections, right-click the TAP adapter and rename it to `tap0`. This matches the interface name used in the QEMU launch command.



## Running

### Windows
```powershell
# In PussyOS dir:
& "C:\Program Files\qemu\qemu-system-x86_64.exe" `
    -cdrom dist/x86_64/kernel.iso `
    -drive file=disk.img,format=raw,index=0,media=disk `
    -netdev tap,id=net0,ifname=tap0 `
    -device e1000,netdev=net0
```

Create a blank disk image if you don't have one:
```powershell
qemu-img create disk.img 16M
```

### Linux
```bash
qemu-system-x86_64 \
    -cdrom dist/x86_64/kernel.iso \
    -drive file=disk.img,format=raw,index=0,media=disk \
    -netdev tap,id=net0,ifname=tap0 \
    -device e1000,netdev=net0
```

Create a blank disk image if you don't have one:
```bash
qemu-img create disk.img 16M
```

---

## Login

When the OS boots it will ask for a username and password. The defaults are:

```
user: admin
pass: pass
```

To change them, once you are in the shell run:

```
edit config
```

Change the credentials inside the file, then exit the editor and run:

```
savefs
```

That saves the filesystem to disk so your changes persist on the next boot.

---

## Network configuration inside the OS

Once you are in the shell, run `netsettings` to open the network configuration screen. Use the arrow keys to select a field and Enter to edit it.

There are four fields to configure:

**My IP** — the IP address the OS will use on your network. Pick any free address on your local subnet, for example `192.168.1.150`. Make sure nothing else on your network is using it.

**Gateway IP** — your router's IP address. Find it by running:

Windows:
```powershell
ipconfig
```
Linux:
```bash
ip route | grep default
```
Look for "Default Gateway" (Windows) or the address after "via" (Linux).

**My MAC** — the virtual MAC address for the OS. The default is fine to leave as is.

**Gateway MAC** — your router's MAC address. Find it by running:

Windows:
```powershell
arp -a
```
Linux:
```bash
arp -n
```
Look for the entry matching your router's IP and copy the physical address.

netsettings are not saved to disk yet, you will need to redo them on each reboot. This will be fixed in a future update.
and the gateway mac will be solved too on boot.

---

## Architecture
The kernel is written in C with the boot and mode-switching code in NASM assembly. The screen is driven by direct writes to VGA text mode memory at `0xB8000`. There is no external runtime, no standard library, and no borrowed kernel subsystem anywhere.

**Boot layer** handles Multiboot2 validation, CPU feature checks, page table construction, PAE + long mode activation, and GDT setup before handing off to C.

**VGA / UI layer** manages the text mode display, colors, cursor positioning, and the shell UI layout.

**Filesystem** is an in-memory structure supporting files, directories, and nested paths. The entire state can be snapshotted to the raw disk image and reloaded on next boot.

**Network stack** is built on a hand-written e1000 NIC driver that talks directly to hardware registers. Supports TCP, UDP, HTTP serving, file transfer, and raw traffic monitoring.

**Shell** reads keyboard input line by line and dispatches to command handlers. Type `help` inside the OS for the full command reference.

**Scripting** supports two runtimes: a standard Brainfuck interpreter and Pussy Lang, a custom scripting language that can call shell commands and launch programs.

---

Built by Myura.
Any questions or help:
Discord-  mynuramy
X-  MyuraASM
