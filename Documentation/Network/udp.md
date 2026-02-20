# udp.c

## Overview

`udp.c` implements UDP send and receive for PussyOS. It provides a small port registry that lets other parts of the kernel register handlers for specific UDP ports, two transmission functions for sending datagrams, and an inbound dispatcher that demultiplexes incoming packets to the right handler.

---

## Port Registry

The file maintains a static table of up to eight port-to-handler mappings. Each entry pairs a `uint16_t` port number with a `udp_handler_t` function pointer.

```c
static struct {
    uint16_t port;
    udp_handler_t handler;
} port_table[MAX_UDP_PORTS];
```

`udp_register_port` adds an entry to this table. If the table is already full it returns silently, dropping the registration. Ports are matched in registration order during dispatch, so earlier registrations take priority if duplicates are ever added.

---

## Sending — `udp_send_reply` and `udp_send`

Two functions handle outbound datagrams, covering the two common cases.

`udp_send_reply` is a static helper used internally when responding to an incoming packet. It extracts the original sender's MAC and IP from the received packet and uses those as the destination for the reply, so the caller does not need to supply addressing information explicitly.

`udp_send` is the public-facing function for sending an arbitrary datagram to a known destination. The caller provides the destination IP, destination port, destination MAC, source port, and payload directly.

Both functions construct packets identically: they lay out an Ethernet header, IPv4 header, and UDP header consecutively in a 2048-byte stack buffer, copy the payload in behind them, compute the IP checksum using `checksum_aligned`, and call `e1000_transmit`. The UDP checksum field is left as zero, which is legal under the UDP specification.

```c
r_udp->checksum = 0;
```

The total length of the packet is computed as the sum of all three header sizes plus the payload length, and this value is written into both the IP total length field and the UDP length field with appropriate byte-order conversion.

---

## Receiving — `handle_udp`

`handle_udp` is called by `handle_packet` in `network.c` whenever an incoming IPv4 packet carries protocol number 17. It locates the UDP header by accounting for the variable IP header length, then extracts the source port, destination port, and payload length.

The destination port is first checked against the registered port table. If a matching handler is found, it is called with the full packet pointer, the payload pointer, the payload length, the sender's IP in host order, the sender's source port, and the sender's MAC address. The function then returns immediately.

```c
port_table[i].handler(packet, payload, payload_len,
                      ntohl(ip->src_ip), src_port,
                      eth->src_mac);
```

If no registered handler matches, two built-in fallback handlers are checked. Port 7 is the standard echo port — any payload received there is sent straight back to the sender unchanged. Port 1234 is a debug port that responds with a fixed greeting string. If the destination port matches neither of those, a message is printed and the packet is discarded.
