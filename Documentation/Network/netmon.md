# Network Monitor

Simple network packet monitor for Intel e1000 network cards(for now).

## Starting

```
netmon
```

Starts monitoring incoming network packets. Displays received packets in terminal.

## Controls

- ESC - Exit monitor and return to shell

## Usage

**Start monitor:**
```
netmon
```

**Send test packet from host:**
```
ping 192.168.0.200
```

Monitor will display received packets as they arrive.

## Display

**Startup:**
```
Network Monitor Started
Try: ping 192.168.0.200
Press ESC to exit
```

**While running:**
- Shows incoming packet information
- Displays packet contents and headers
- Updates in real-time

**Exit:**
```
Network Monitor Stopped
```

## Network Setup

**Requirements:**
- Intel e1000 network card (8086:100E)
- Card must be detected at boot
- Network initialized by `netmon_init()`

**IP Configuration:**
- Guest IP: 10.0.2.15 (default)
- Test target: 192.168.0.200

**Card Detection:**
If network card not found at boot:
```
ERROR: e1000 network card not found!
```

## Packet Display

Monitor shows received packets with:
- Source/destination MAC addresses
- Source/destination IP addresses
- Packet type (ARP, ICMP, IP)
- Packet length
- Raw data (for debugging)



## Limitations

**Display only:**
Monitor is read-only. Does not send packets or respond to requests.

**No filtering:**
Shows all received packets. No option to filter by type or address.

**Single card:**
Only supports one e1000 card (first detected).

**Terminal output:**
All output goes to console. No packet capture file or statistics.

## Technical Details

**Polling:**
Checks for packets every 100,000 CPU cycles. 

**Card support:**
Intel e1000 (vendor ID 0x8086, device ID 0x100E) only.

**PCI location:**
Automatically detected during init. Bus/slot/function stored for driver access.

**Exit behavior:**
Clean exit via ESC. Returns to shell prompt immediately.

## Integration

**Initialization at boot:**
```c
// In kernel_main()
netmon_init();
```

**Shell command:**
```c
if (strcmp(command, "netmon") == 0) {
    netmon_start();
}
```



## Common Issues

**Card not detected:**
- Check QEMU/VM network adapter settings
- Verify e1000 emulation enabled


**No packets received:**
- Verify host can reach guest IP
- Check network mode (NAT vs bridged)
- Ensure firewall allows ICMP
- Try different packet types (ARP, UDP)



