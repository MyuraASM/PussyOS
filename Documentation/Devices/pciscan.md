# pciscan (pci.c / pci_browser.c)

## Overview

`pci.c` and `pci_browser.c` together implement pciscan, an interactive PCI device browser for PussyOS. `pci.c` provides the low-level configuration space access used by drivers across the kernel. `pci_browser.c` builds an interactive TUI on top of it, letting you explore every PCI device on the system without leaving the OS.

---

## pci.c — PCI Configuration API

These functions are the building blocks used by device drivers to locate and configure hardware. They communicate with the PCI controller directly through the x86 I/O ports `0xCF8` (address) and `0xCFC` (data).

### Reading Configuration Space

```c
uint32_t pci_config_read_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t pci_config_read_word (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint8_t  pci_config_read_byte (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
```

Read a value from the PCI configuration space of the device at the given bus, slot, and function. `offset` is the byte offset into the 256-byte configuration header (e.g. `0x00` for vendor ID, `0x10` for the first BAR).

### Writing Configuration Space

```c
void pci_config_write_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
void pci_config_write_word (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint16_t value);
void pci_config_write_byte (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint8_t  value);
```

Write a value to configuration space. Word and byte writes do a read-modify-write internally so they do not disturb adjacent fields in the same dword.

### Finding a Device — `pci_find_device`

```c
int pci_find_device(uint16_t vendor_id, uint16_t device_id,
                    uint8_t *bus, uint8_t *slot, uint8_t *func);
```

Scans the entire PCI bus for a device matching the given vendor and device ID. On success writes the location into `bus`, `slot`, and `func` and returns 1. Returns 0 if not found. This is how drivers like `e1000.c` locate their hardware at startup.

### Debug Dump — `pci_scan`

```c
void pci_scan(void);
```

Prints a one-line summary of every PCI device to the terminal. Useful for a quick sanity check when no graphical browser is needed.

---

## pci_browser.c — Interactive Browser

Launched from the shell, the browser scans all PCI devices and presents them in a navigable list. Selecting a device shows a detailed breakdown of everything in its configuration space.

### Controls

| Key | Action |
|-----|--------|
| Up / Down | Navigate the device list |
| Enter | Open device details |
| Enter (in details) | Back to list |
| R | Rescan the PCI bus |
| ESC | Exit the browser |

### List View

Shows all detected devices in a scrollable table with their BDF address (bus:device.function), vendor name, device ID, and class. The currently selected row is highlighted.

### Detail View

Pressing Enter on any device opens a scrollable detail panel covering everything read from its configuration space.

**Identification** shows the vendor name and ID, device ID, revision, subsystem IDs, and the full class, subclass, and programming interface with human-readable labels where known (e.g. "AHCI 1.0" for a SATA controller with prog-if `0x01`).

**Control and Status** decodes the command register bit by bit, showing whether I/O space, memory space, bus mastering, and interrupts are enabled or disabled, with green and red coloring for at-a-glance reading.

**Interrupt** shows the IRQ line number and pin (INTA through INTD).

**Bridge Configuration** appears only for PCI-to-PCI bridge devices and shows the primary, secondary, and subordinate bus numbers.

**Base Address Registers** lists each non-zero BAR with its type (I/O or memory), base address, memory width (32-bit or 64-bit), prefetchable flag, and the probed size formatted as bytes, KB, or MB.

**Timing** shows cache line size and latency timer when set.
