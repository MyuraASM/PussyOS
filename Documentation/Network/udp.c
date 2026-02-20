# network.c

## Overview

`network.c` is the core networking layer for PussyOS. It defines the machine's network identity, implements byte order conversion, and handles the three foundational protocols that make IP networking possible at the kernel level: ARP, ICMP, and the top-level packet dispatch that ties them together.

---

## Network Configuration

The file opens by declaring the system's network identity. The MAC address and IP address are hardcoded globals that the rest of the network stack references via `extern`.

```c
uint8_t  my_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
uint32_t my_ip     = 0xC0A800C8;   // 192.168.0.200 (host order)
```

The IP is stored in host byte order throughout, with conversion to network order happening at the point of packet construction. A `network_silent` flag is also declared here, intended to suppress output when set.

---

## Byte Order Conversion

Four functions handle the standard host-to-network and network-to-host conversions that are required whenever values cross the wire. Since x86 is little-endian and network protocols are big-endian, every multi-byte field in an outgoing packet must be swapped before transmission and swapped back on receipt.

`htons` and `ntohs` handle 16-bit values; `htonl` and `ntohl` handle 32-bit values. Because the byte-swap operation is its own inverse, each pair is implemented symmetrically — `ntohs` simply calls `htons`.

```c
uint16_t htons(uint16_t n) { return (n << 8) | (n >> 8); }
```

---

## Utility Functions

Two internal helpers support the rest of the file.

`mempci` is a simple byte-by-byte memory copy used in place of the standard library's `memcpy`, which is unavailable in a bare-metal context. It copies `len` bytes from `src` to `dst` in a straightforward loop.

`checksum_aligned` exists to safely compute checksums over packed structs, which may not be word-aligned in memory. It first copies the data into a local stack buffer, then passes that to `calculate_checksum`. This avoids undefined behavior from unaligned 16-bit reads on the original struct.

`calculate_checksum` implements the standard one's complement Internet checksum used by IP, ICMP, and UDP. It sums all 16-bit words in the data, folds any carry bits back into the lower 16 bits, and returns the bitwise complement of the result.

```c
while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
return (uint16_t)(~sum);
```

---

## ARP Handling — `handle_arp`

`handle_arp` responds to ARP requests that are asking for the system's MAC address. It first validates that the packet is long enough to contain a full ARP payload, then checks that the opcode is a request and that the target IP matches `my_ip`. Packets not addressed to this machine are silently dropped.

If the request passes those checks, a 42-byte reply is constructed in place. The reply swaps the sender and target roles, fills in the local MAC address as the answer, and sends it back using `e1000_transmit`. The fixed size of 42 bytes reflects the exact layout of an Ethernet header plus a standard IPv4 ARP packet with no padding.

---

## ICMP Handling — `handle_icmp`

`handle_icmp` handles ping requests. It extracts the IP header length from the `version_ihl` field to correctly locate the ICMP header, since IP headers are variable-length. It validates that the packet is large enough and that the ICMP type is an echo request before doing any work.

The reply is built into a 2048-byte stack buffer. The Ethernet, IP, and ICMP headers are filled out in sequence, with the source and destination addresses swapped from the original. The original echo payload is copied verbatim into the reply using `mempci`.

Checksums are computed last, after all fields are populated. The IP checksum covers only the IP header, while the ICMP checksum covers both the ICMP header and the payload together. Both are calculated using `checksum_aligned` to handle potential alignment issues with the packed structs.

```c
r_icmp->checksum = checksum_aligned(r_icmp, sizeof(struct icmp_header) + payload_size);
```

---

## Main Packet Dispatcher — `handle_packet`

`handle_packet` is the single entry point that the driver calls for every received frame. It reads the EtherType field to determine what kind of packet arrived and routes accordingly.

ARP packets are dispatched immediately regardless of IP destination. IPv4 packets are first filtered by destination IP — anything not addressed to `my_ip` is dropped — and then dispatched to either `handle_icmp` or `handle_udp` based on the protocol field in the IP header.
