# pciscan

Interactive PCI device browser with detailed hardware inspection.

## Starting

```
pciscan
```

Fullscreen browser showing all PCI devices with navigation.

## Controls

**Device List:**
- UP/DOWN arrows - Navigate devices
- ENTER - View device details
- R - Rescan bus
- ESC - Exit

**Details View:**
- UP/DOWN arrows - Scroll details
- ENTER - Return to list
- ESC - Exit

## Device List

Shows all detected PCI devices in table format:

```
BDF          Vendor         Device    Class
00:00.0      Intel          1237      Host
00:01.0      Intel          7000      Bridge
00:02.0      QEMU           1111      Display
```

**Columns:**
- BDF: Bus:Device.Function in hex
- Vendor: Manufacturer name (or "Unknown")
- Device: Device ID in hex
- Class: Device type

**Colors:**
- Selected device: White on black
- Normal devices: Gray on black
- Headers: White on gray

## Device Details

Press ENTER on any device to see full information.

**Sections shown:**

**Identification:**
- Location (bus:device.function)
- Vendor name and ID
- Device and revision IDs
- Subsystem IDs
- Class codes with descriptions

**Control & Status:**
- Command register bits
- I/O space enable/disable
- Memory access enable/disable
- Bus mastering status
- Interrupt disable status
- Status register
- 66MHz capability
- Fast back-to-back capable
- Capabilities list pointer

**Interrupt:**
- IRQ line number
- IRQ pin (INTA/INTB/INTC/INTD)

**Bridge Configuration:**
(For PCI-to-PCI bridges only)
- Primary bus number
- Secondary bus number
- Subordinate bus number

**Base Address Registers (BARs):**
- BAR number (0-5)
- Type (I/O or Memory)
- Base address
- Size in bytes/KB/MB
- Memory type (32-bit/64-bit)
- Prefetchable flag

**Timing:**
- Cache line size
- Latency timer

## Vendor Names

Common vendors detected:
- Intel (0x8086)
- AMD (0x1022)
- NVIDIA (0x10DE)
- ATI/AMD (0x1002)
- VirtualBox (0x80EE)
- QEMU (0x1234)
- VMware (0x15AD)
- VirtIO (0x1AF4)
- Realtek (0x10EC)
- Broadcom (0x14E4)

## Device Classes

```
0x00 - Unclassified
0x01 - Mass Storage (SCSI, IDE, SATA, NVMe)
0x02 - Network (Ethernet, WiFi)
0x03 - Display (VGA, XGA, 3D)
0x04 - Multimedia (Video, Audio)
0x05 - Memory
0x06 - Bridge (Host, ISA, PCI-to-PCI)
0x07 - Communication (Serial, Parallel)
0x08 - System (PIC, DMA, Timer)
0x09 - Input (Keyboard, Mouse)
0x0C - Serial Bus (USB, FireWire)
0x0D - Wireless
```

## Subclass Examples

**Mass Storage:**
- 0x00: SCSI
- 0x01: IDE
- 0x06: SATA
- 0x08: NVMe

**Network:**
- 0x00: Ethernet
- 0x80: WiFi

**Serial Bus:**
- 0x03: USB (with prog_if for UHCI/OHCI/EHCI/xHCI)

## Programming Interface

Shows specific implementation for certain device types:

**USB Controllers:**
- 0x00: UHCI
- 0x10: OHCI
- 0x20: EHCI (USB 2.0)
- 0x30: xHCI (USB 3.0)

**SATA:**
- 0x01: AHCI 1.0

**IDE:**
- 0x8F: PCI Native (full featured)

## BAR Types

**I/O Space:**
- Used for port I/O
- Address masked with 0xFFFC
- Small sizes (typically < 256 bytes)

**Memory Space:**
- Used for MMIO
- Address masked with 0xFFFFFFF0
- Can be 32-bit or 64-bit
- May be prefetchable

**Size Calculation:**
Automatically calculated by writing 0xFFFFFFFF and reading back.

## Command Register Bits

```
Bit 0: I/O Space Enable
Bit 1: Memory Space Enable
Bit 2: Bus Master Enable
Bit 4: Memory Write and Invalidate
Bit 10: Interrupt Disable
```

## Status Register Bits

```
Bit 4: Capabilities List
Bit 5: 66MHz Capable
Bit 7: Fast Back-to-Back Capable
```


## Bridge Information

PCI-to-PCI bridges show bus routing:
- Primary: Upstream bus
- Secondary: Downstream bus (directly behind bridge)
- Subordinate: Highest bus number behind bridge



## Color Coding

**Headers:** Blue on white (0x1F)
**Selected:** White on gray (0x70)
**Normal text:** White on black (0x07)
**Labels:** Gray (0x08)
**Values:** Bright white (0x0F)
**Secondary info:** Cyan (0x0B)
**Type indicators:** Magenta (0x0D)
**Status enabled:** Green (0x0A)
**Status disabled:** Red (0x0C)

## Integration



Requires VGA buffer and PCI functions.

