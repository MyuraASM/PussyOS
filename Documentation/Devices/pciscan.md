# pciscan (pci.c / pci_browser.c)

## Overview

`pci.c` and `pci_browser.c` together implement pciscan, an interactive PCI device browser for PussyOS. `pci.c` provides the low-level foundation: port I/O primitives, configuration space read and write functions, device existence detection, and a basic text dump for debugging. `pci_browser.c` builds on top of that to present a full interactive TUI, letting you navigate every PCI device on the system, inspect its registers, BARs, interrupt routing, bridge topology, and timing configuration in detail.

---

## pci.c — Low Level PCI Access

### Port I/O Primitives

Six static inline functions wrap x86 I/O instructions directly, covering byte, word, and dword reads and writes. These are the only way to communicate with the PCI configuration mechanism since there is no OS abstraction layer available.

```c
static inline uint32_t inl(uint16_t port);
static inline void     outl(uint16_t port, uint32_t val);
static inline uint16_t inw(uint16_t port);
static inline void     outw(uint16_t port, uint16_t val);
static inline uint8_t  inb(uint16_t port);
static inline void     outb(uint16_t port, uint8_t val);
```

### Configuration Address — `pci_make_addr`

PCI configuration space is accessed through two fixed I/O ports: `PCI_CONFIG_ADDRESS` (0xCF8) and `PCI_CONFIG_DATA` (0xCFC). Before reading or writing, a 32-bit address must be written to the address port identifying the target bus, slot, function, and register offset. `pci_make_addr` constructs that value, setting bit 31 to enable configuration access and packing the remaining fields into their correct positions.

```c
static uint32_t pci_make_addr(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    return (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)(slot & 0x1F) << 11) |
           ((uint32_t)(func & 0x07) << 8) | (offset & 0xFC);
}
```

The offset is masked to `0xFC` because PCI configuration reads are always dword-aligned.

### Read Functions

`pci_config_read_dword` is the base read operation. It writes the address to the address port and reads back the full 32-bit value from the data port.

`pci_config_read_word` and `pci_config_read_byte` both call the dword variant and then extract the relevant portion by shifting based on the lower bits of the offset. This avoids needing separate hardware access paths for narrower reads.

```c
uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t dword = pci_config_read_dword(bus, slot, func, offset & 0xFC);
    return (dword >> ((offset & 2) * 8)) & 0xFFFF;
}
```

### Write Functions

`pci_config_write_dword` mirrors the dword read, writing directly to the data port after setting the address.

`pci_config_write_word` and `pci_config_write_byte` perform a read-modify-write: they read the enclosing dword, mask out the target field, OR in the new value at the correct bit position, and write the result back. This preserves the surrounding fields that are not being modified.

### Device Existence — `pci_device_exists`

Reads the vendor ID register for a given bus, slot, and function. If the result is `0xFFFF` the slot is empty and the function returns 0. Any other value indicates a real device.

### Debug Scan — `pci_scan`

Iterates all 256 buses, 32 slots, and 8 functions and prints a one-line summary for each device that exists. The output includes the BDF address, vendor ID, device ID, class code, subclass, and programming interface, all in hex. This is intended as a quick debugging tool and is separate from the interactive browser.

### Device Search — `pci_find_device`

Scans the full PCI space looking for a device matching a specific vendor and device ID pair. On success it writes the bus, slot, and function into the provided output pointers and returns 1. Returns 0 if no match is found. This is used by drivers like `e1000.c` to locate their hardware at boot.

```c
int pci_find_device(uint16_t vendor_id, uint16_t device_id,
                    uint8_t *bus, uint8_t *slot, uint8_t *func);
```

---

## pci_browser.c — Interactive TUI

### Device Storage

All discovered devices are stored in a static array of `pci_device_info_t` structs capped at 256 entries. Each struct holds everything read from configuration space during the scan: BDF address, vendor and device IDs, class codes, all six BARs with their sizes, subsystem IDs, interrupt routing, capabilities pointer, and bridge bus numbers.

```c
static pci_device_info_t devices[MAX_DEVICES];
static int device_count  = 0;
static int selected_device = 0;
static int scroll_offset = 0;
static int details_scroll = 0;
static uint8_t show_details = 0;
```

`show_details` switches between the device list view and the detail view. Both scroll independently.

### Device Scan — `scan_all_devices`

Iterates the full PCI space calling `pci_device_exists` at each slot. For every device found it reads all relevant configuration registers into the corresponding `pci_device_info_t`. BAR sizes are calculated by calling `get_bar_size` for each of the six BARs.

The function respects the multi-function device flag in the header type register. If bit 7 of `header_type` is clear for function 0, the inner function loop breaks early and only one function is scanned for that slot.

Bridge devices (class `0x06`, subclass `0x04`) get their primary, secondary, and subordinate bus numbers read from the bridge-specific register offsets `0x18`, `0x19`, and `0x1A`.

### BAR Size Calculation — `get_bar_size`

Determines the size of a BAR by using the standard PCI technique: save the original value, write all 1s, read back the result, then restore the original. The readback value has the lower bits forced to zero by the hardware, indicating the alignment requirement, which is also the size.

```c
pci_config_write_dword(bus, slot, func, bar_offset, 0xFFFFFFFF);
uint32_t size_mask = pci_config_read_dword(bus, slot, func, bar_offset);
pci_config_write_dword(bus, slot, func, bar_offset, original);
```

For I/O BARs bits 0-1 are masked off before computing. For memory BARs bits 0-3 are masked off. The size is then `~size_mask + 1`. Returns 0 if the BAR is unimplemented.

### Lookup Tables

Three functions return human-readable strings by switching on numeric codes read from configuration space.

`get_vendor_name` maps known vendor IDs to names covering Intel, AMD, NVIDIA, QEMU, VirtualBox, VMware, VirtIO, Realtek, and others. Unknown vendors fall through to "Unknown".

`get_class_name` maps the single-byte class code to a category name such as "Mass Storage", "Network", "Bridge", and so on.

`get_subclass_name` maps class and subclass together to a more specific description, for example distinguishing SCSI from IDE from NVMe within the mass storage class.

`get_prog_if_name` goes one level deeper for device classes where the programming interface byte is meaningful: IDE controller modes, SATA AHCI vs vendor-specific, USB host controller types (UHCI, OHCI, EHCI, xHCI), PCI bridge decode modes, and PIC variants.

### Screen Rendering

All rendering writes directly to the VGA text buffer via the `buffer` pointer exported from the print subsystem. The display is fixed at 80x25. Three helpers handle formatted output without relying on any standard library.

`write_string` writes a null-terminated string at a given column and row with a specified color attribute byte.

`write_hex8`, `write_hex16`, and `write_hex32` write hexadecimal values of 1, 2, and 4 bytes respectively at a given position.

`write_dec` converts an integer to decimal and writes it at the given position.

`clear_screen` fills the entire buffer with spaces in the default white-on-black color.

### List View — `render_device_list`

Displays up to 20 devices at a time with a scrollable list. Each row shows the BDF address in hex, the vendor name, the device ID, and the class name. The selected row is highlighted with an inverted color attribute. `scroll_offset` tracks which device is at the top of the visible window and is adjusted when the selection moves outside the visible range.

### Detail View — `render_device_details`

Renders the full detail panel for the currently selected device. The content is divided into labeled sections: identification, control and status, interrupt routing, bridge configuration (only for bridge devices), base address registers, and timing.

The detail view uses a virtual line counter and a `details_scroll` offset so the content can scroll independently of the list. A `WRITE_LINE` macro increments the virtual line counter and converts it to a screen row only if it falls within the visible window, otherwise setting `y` to -1 so the rendering calls below it are skipped cleanly.

The command register is decoded bit by bit, showing the I/O space enable, memory space enable, bus master, memory write and invalidate, and interrupt disable bits as individual enabled/disabled labels with green and red coloring.

BARs are printed with their type (I/O or memory), base address, width (32-bit or 64-bit for memory BARs), prefetchable flag, and size formatted as bytes, kilobytes, or megabytes depending on the magnitude.

### Header and Footer — `render_header` / `render_footer`

`render_header` draws a title bar across the top row with the total device count.

`render_footer` draws a help bar on row 23 with the relevant key bindings for the current view, and a status bar on row 24 with either a device count summary or a "Viewing device details" message.

### Main Entrypoint — `pci_browser_start`

Calls `scan_all_devices`, rejects immediately if nothing was found, then enters the main event loop. The loop waits on the PS/2 status register for a scancode, ignores key release events (high bit set), and dispatches on the following keys.

ESC exits the browser. Up and down arrows navigate the device list in list view, or scroll the detail panel in detail view. Enter toggles between the two views, resetting `details_scroll` to zero when entering the detail view. R rescans the bus from scratch when in list view, resetting the selection to the first device.

On exit the function restores the terminal by clearing the screen, resetting the cursor position, printing the shell prompt, and calling `print_set_prompt`.
