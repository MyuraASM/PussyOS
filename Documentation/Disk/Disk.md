# Disk.c

## Overview

`Disk.c` is a low-level ATA (Advanced Technology Attachment) disk driver written in C for a bare-metal or OS kernel environment. It provides the minimal functionality needed to read and write 512-byte sectors from a hard drive using direct port I/O and LBA (Logical Block Addressing) mode.

---

## Port I/O Primitives

The file opens with four static inline functions that serve as the foundation for all hardware communication. These wrap x86 assembly instructions directly, since there is no operating system abstraction layer to rely on.

`outb` sends a single byte to a given I/O port. `inb` reads a single byte back from a port. These two are the core building blocks for sending commands and reading status registers on the ATA controller.

`inw_rep` and `outw_rep` are bulk transfer variants. They use the x86 `rep insw` and `rep outsw` instructions to move a block of 16-bit words in or out of a port in one efficient burst. These are specifically used when transferring sector data, since a 512-byte sector is read or written as 256 consecutive 16-bit words.

---

## Wait Helpers

Two small internal functions handle synchronization with the drive.

`disk_wait_ready` polls the ATA status register in a tight loop until the BSY (busy) bit clears. It is called before sending any command to ensure the drive is not already processing something.

`disk_wait_data` also polls the status register, but it waits until the DRQ (data request) bit is set. This signals that the drive has data ready to be transferred, or in the write case, that it is ready to accept data from the CPU.

---

## Initialization

`disk_init` is intentionally minimal. It simply calls `disk_wait_ready` to confirm the drive is responsive before the rest of the system tries to use it. No complex detection or configuration is performed here.

---

## Reading a Sector — `disk_read_sector`

This function takes a 32-bit LBA address and a pointer to a buffer where the sector data will be stored. The process follows the standard ATA PIO read sequence:

First it waits for the drive to be idle. Then it writes to the drive select register, encoding the top 4 bits of the LBA address and setting the mode flags to indicate LBA mode on the primary master drive. After that, it writes the sector count (always 1), followed by the lower 24 bits of the LBA address split across three separate registers.

With the address fully set up, it issues the read command. The driver then waits for DRQ, checks the error bit in the status register, and if all is well, reads 256 words (512 bytes) from the data port into the caller's buffer using `inw_rep`. It returns 0 on success and -1 on error.

---

## Writing a Sector — `disk_write_sector`

The write function mirrors the read function almost exactly in its setup. The same wait, drive select, sector count, and LBA address sequence is performed. The write command is then issued.

The key difference is what happens after the command: the driver waits for DRQ to confirm the drive is ready to receive data, then pushes 256 words out to the data port using `outw_rep`. After the transfer, it waits once more for the drive to finish committing the data before checking for errors. It returns 0 on success and -1 on error.

---

## General Design Notes

The driver uses PIO (Programmed I/O) mode rather than DMA, meaning the CPU is directly involved in every byte of the transfer and spins in busy-wait loops during delays. This is simple and reliable for an early-stage kernel or bootloader context, but would not be appropriate for a performance-sensitive system. All functions operate on exactly one sector at a time, and there is no caching, buffering, or interrupt handling.
