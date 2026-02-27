# tcp.c / tcp_connect.c

## Overview

`tcp.c` and `tcp_connect.c` together form a complete TCP layer for PussyOS, built on top of the existing Ethernet, IP, and ARP infrastructure. `tcp.c` handles inbound connections, specifically an HTTP server listening on port 80, while `tcp_connect.c` handles outbound connections, providing a callback-driven API for initiating TCP connections to remote hosts. Both files share the same wire format and checksum logic, operating directly over the e1000 driver with no OS abstraction.

---

## Shared Packet Structures

Both files define local packed structs for the Ethernet, IP, and TCP headers. These mirror the structs in `network.h` but are redefined locally to avoid cross-file coupling.

The TCP header captures all standard fields: source and destination ports, sequence and acknowledgement numbers, data offset, flags byte, window size, checksum, and urgent pointer.

```c
struct __attribute__((packed)) tcp_header {
    uint16_t src_port, dst_port;
    uint32_t seq_num, ack_num;
    uint8_t  data_offset, flags;
    uint16_t window, checksum, urgent;
};
```

The pseudo-header is used exclusively for checksum calculation. It is never transmitted. It prepends the TCP segment in a temporary buffer to include IP addressing in the checksum, as required by RFC 793.

```c
struct __attribute__((packed)) tcp_pseudo {
    uint32_t src_ip, dst_ip;
    uint8_t  zero, protocol;
    uint16_t tcp_length;
};
```

Flag constants are defined as single-bit masks covering FIN, SYN, RST, and ACK.

---

## Checksum Calculation

Both files implement a local TCP checksum function that follows the same pattern. A temporary stack buffer is allocated to hold the pseudo-header immediately followed by a copy of the TCP segment. The standard ones-complement checksum is then run over the combined buffer.

```c
static uint16_t tcp_checksum(uint32_t src_ip, uint32_t dst_ip,
                              void *tcp_seg, uint16_t tcp_len) {
    uint8_t buf[sizeof(struct tcp_pseudo) + tcp_len];
    // fill pseudo header, copy segment, run checksum
    return calculate_checksum((uint16_t *)buf, sizeof(buf));
}
```

The IP header checksum uses `checksum_aligned` from `network.c`, which copies the header into a local buffer before computing, avoiding alignment issues with packed structs.

---

## tcp.c — Inbound HTTP Server

### Raw Frame Sender — tcp_send_flags

This is the core transmission primitive for the inbound side of TCP. It constructs a complete Ethernet frame from scratch given a destination IP, destination MAC, port pair, sequence and acknowledgement numbers, flags byte, and optional payload. It allocates the frame on the stack, fills all three headers, copies in any payload, computes the TCP checksum, and calls `e1000_transmit`.

```c
static void tcp_send_flags(uint32_t dst_ip, uint8_t *dst_mac,
                            uint16_t src_port, uint16_t dst_port,
                            uint32_t seq, uint32_t ack, uint8_t flags,
                            uint8_t *payload, uint16_t payload_len);
```

Because the inbound server is stateless with no connection table, all addressing and sequence information must be passed explicitly on every call.

### Main Handler — handle_tcp

`handle_tcp` is called by the main packet dispatcher in `network.c` whenever an IPv4 packet with protocol 6 arrives and is addressed to this host.

The first thing it does is forward the packet to `tcp_conn_handle` from `tcp_connect.c`. If that function returns 1, the packet belongs to an outbound connection already registered in the connection table, and `handle_tcp` returns immediately without further processing.

```c
if (tcp_conn_handle(packet, length)) return;
```

If the packet is not claimed by an outbound connection, `handle_tcp` checks whether the destination port is 80. Any packet to any other port is silently dropped.

From there, three cases are handled based on the TCP flags.

When a SYN arrives, a SYN-ACK is sent back with a fixed ISN of `0x12345678`, acknowledging the client's sequence number plus one. No state is stored.

When an ACK with payload arrives and the payload starts with `GET`, an HTTP response is assembled from a static header string concatenated with the contents of `html_buffer` (provided by `http.c`). The response is sent in a single ACK segment, followed immediately by a FIN-ACK to close the connection. `http_on_request` is called with the client IP for logging purposes.

When a FIN arrives, a bare ACK is sent to acknowledge the client's close. No teardown state is maintained.

Because no connection state is stored between packets, this server handles only simple single-exchange HTTP/1.0 requests. It does not support pipelining, keep-alive, or out-of-order delivery.

---

## tcp_connect.c — Outbound Connection API

### Connection Table

Up to `MAX_CONNS` (4) simultaneous outbound connections are tracked in a static array of `struct tcp_conn` pointers. Each `tcp_conn` (defined in `tcp_connect.h`) holds the full connection state: remote IP, remote MAC, port pair, local and remote sequence numbers, a receive buffer, and three callback pointers.

```c
#define MAX_CONNS 4
static struct tcp_conn *conn_table[MAX_CONNS];
static int conn_count = 0;
```

Local ports are assigned sequentially from an ephemeral range starting at 49152, incrementing with each new connection.

### Raw Frame Sender — send_tcp

`send_tcp` is the outbound equivalent of `tcp_send_flags`. Rather than taking explicit addressing parameters, it reads all connection details directly from a `struct tcp_conn`. It builds the full frame, sets the ACK number only when the `FL_ACK` flag is present, copies any payload, and transmits.

```c
static void send_tcp(struct tcp_conn *c, uint8_t flags,
                     uint8_t *payload, uint16_t plen);
```

### Initiating a Connection — tcp_connect

`tcp_connect` initialises a caller-provided `tcp_conn` struct, registers it in the connection table, and sends the opening SYN.

```c
int tcp_connect(struct tcp_conn *conn,
                uint32_t remote_ip, uint16_t remote_port,
                uint8_t *remote_mac,
                tcp_connected_cb_t on_connected,
                tcp_data_cb_t on_data,
                tcp_closed_cb_t on_closed,
                void *userdata);
```

The ISN is hardcoded to `0xDEAD1234`. After sending the SYN, the local sequence number is incremented by one to account for the SYN consuming a sequence slot. The connection enters `TCP_STATE_SYN_SENT`. Returns -1 if the connection table is full.

### Sending Data — tcp_conn_send

Sends a payload over an established connection. Only valid in `TCP_STATE_ESTABLISHED`, returning -1 otherwise. After transmitting, the local sequence number is advanced by the payload length.

```c
int tcp_conn_send(struct tcp_conn *conn, uint8_t *data, uint16_t len);
```

### Closing a Connection — tcp_conn_close

Sends a FIN-ACK if the connection is established, advances the sequence counter, removes the connection from the table, and marks the state as `TCP_STATE_CLOSED`.

```c
void tcp_conn_close(struct tcp_conn *conn);
```

### Inbound Packet Dispatch — tcp_conn_handle

This function is called by `handle_tcp` for every inbound TCP packet, before inbound server logic runs. It scans the connection table for an entry matching the packet's source IP, source port (as remote), and destination port (as local). If no match is found it returns 0 immediately.

When a match is found, four cases are handled.

A SYN-ACK completes the three-way handshake. The remote sequence is captured, the local sequence is updated to match the peer's ACK, the state advances to `TCP_STATE_ESTABLISHED`, a final ACK is sent, and `on_connected` is fired.

An RST tears the connection down, removes it from the table, and calls `on_closed`.

An ACK with payload updates the remote sequence, ACKs the data, appends the payload to the receive buffer, and calls `on_data` with a pointer to the raw payload.

A FIN sends an ACK, closes the connection, removes it from the table, and calls `on_closed`.

```c
int tcp_conn_handle(uint8_t *packet, uint16_t length);
```

Returns 1 if the packet was consumed, 0 if it did not match any outbound connection.

---

## General Design Notes

The inbound server in `tcp.c` is entirely stateless. Sequence numbers from prior packets are not tracked between exchanges, which is sufficient for HTTP/1.0 but would break any protocol requiring reliable multi-packet sessions on the server side.

The outbound stack in `tcp_connect.c` maintains proper per-connection state and handles the full handshake and teardown lifecycle, making it suitable for use as a TCP client for things like an HTTP client, port scanner, or netcat-style tool.

Neither implementation handles retransmission, timeouts, window scaling, or TCP options. Both process packets synchronously in the polling loop that calls `handle_packet`, with no interrupt-driven or DMA-based receive path.
