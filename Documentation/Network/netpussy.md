# netpussy (netcat.c)

## Overview

`netcat.c` implements netpussy, an interactive TCP client for PussyOS accessible from the shell via `np <ip> <port>`. It lets you open a TCP connection to any remote host, send and receive data in real time from the keyboard, and close the connection cleanly. It builds on top of `tcp_connect.c` for the connection lifecycle and `network.c` for ARP resolution, adding its own keyboard polling loop and line buffering on top.

---

## Keyboard Scancode Tables

Two lookup tables translate PS/2 scancodes into ASCII characters. `sc_map` covers the unshifted layout and `sc_map_shift` covers the shifted layout. Both are 128 entries wide, indexed directly by the raw scancode byte. Entries with no printable mapping are zero.

```c
static const char sc_map[128] = {
    0,0,'1','2','3','4','5','6','7','8','9','0','-','=', ...
};
```

The keyboard is read directly from the PS/2 ports using a bare `inb` inline, since there is no keyboard driver or interrupt handler.

```c
static inline uint8_t inb(uint16_t port) {
    uint8_t v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(port)); return v;
}
```

---

## Connection State and Callbacks

Three volatile flags track the current connection state across the main loop and the TCP callbacks.

```c
static volatile uint8_t nc_connected = 0;
static volatile uint8_t nc_closed    = 0;
static volatile uint8_t nc_got_data  = 0;
```

`on_connected` sets `nc_connected` to 1 and prints a green `[connected]` message to the terminal.

`on_data` receives incoming bytes and prints them directly to the screen, stripping bare carriage returns to avoid double newlines.

`on_closed` sets `nc_closed` to 1 and prints a yellow `[connection closed]` message, which causes the main interactive loop to exit.

---

## Input Parsing

Two small parsers convert the command line arguments into usable values.

`parse_ip` walks a dotted-decimal string character by character, accumulating each octet and shifting it into a 32-bit host-order integer.

`parse_port` reads a decimal string and returns a 16-bit port number.

Neither function performs any validation beyond stopping at non-numeric characters, so malformed input results in a zero return value which `nc_start` checks before proceeding.

---

## ARP Resolution — `arp_resolve`

Before a TCP connection can be opened, the MAC address of the target must be known. `arp_resolve` handles this, and is aware of routing.

If the target IP is on a different subnet from `my_ip`, the function ARPs for the gateway instead of the destination directly. The Ethernet frame will be sent to the router's MAC while the IP header still carries the real destination, allowing the router to forward it correctly.

```c
uint32_t arp_target = (my_subnet != tgt_subnet) ? my_gateway : target_ip;
```

If the target is the gateway and `gateway_mac_resolved` is already set, the function skips the ARP exchange entirely and copies the cached MAC straight into `out_mac`.

Otherwise a standard ARP request is broadcast, `arp_resolved_ip` is set to the target being waited on, and `e1000_poll_rx` is called in a loop up to 500,000 times waiting for `handle_arp` in `network.c` to set `arp_resolved`. If the reply arrives in time, the resolved MAC is copied out and, if the target was the gateway, cached into `gateway_mac` for future connections. If the loop exhausts without a reply, the function returns -1.

---

## Line Buffering — `nc_flush_line`

Keystrokes are accumulated into a 256-byte buffer and sent as a single TCP segment when the user presses enter. `nc_flush_line` calls `tcp_conn_send` with the current buffer contents and resets the length counter to zero.

```c
static void nc_flush_line(struct tcp_conn *conn, uint8_t *buf, uint16_t *len);
```

Sending line by line rather than character by character reduces the number of TCP segments and avoids sending a separate packet for every keystroke.

---

## Main Entrypoint — `nc_start`

`nc_start` is called by the shell with the IP and port strings as arguments. It parses both values, rejects zeroes, resolves the target MAC via `arp_resolve`, and calls `tcp_connect` to initiate the handshake.

After that it polls `e1000_poll_rx` up to 1,000,000 times waiting for `nc_connected` to be set by the callback. If the connection does not come up in that window, it closes the half-open connection and prints an error.

Once connected, the function enters the interactive loop. On each iteration it polls the network first, then checks the PS/2 status register. If no scancode is ready it continues immediately rather than blocking.

```c
if (!(inb(0x64) & 0x01)) continue;
uint8_t sc = inb(0x60);
```

Modifier key state is tracked manually. Shift press and release scancodes (0x2A, 0x36, 0xAA, 0xB6) toggle the `shift` flag. Ctrl press and release (0x1D, 0x9D) toggle the `ctrl` flag. Key release events (high bit set) are otherwise ignored.

Two control sequences are handled. Ctrl+X (scancode 0x2D) prints a disconnect notice, calls `tcp_conn_close`, and breaks out of the loop. Ctrl+C (scancode 0x2E) sends a single 0x03 byte to the remote host, which most Unix programs interpret as an interrupt signal.

All other scancodes are looked up in the appropriate table, echoed to the local screen, and added to the line buffer. Backspace decrements the buffer length without sending anything. A newline flushes the buffer with a CRLF terminator appended, since most remote servers expect CRLF line endings.

The loop exits either when `nc_closed` is set by the `on_closed` callback or when the user sends Ctrl+X.
