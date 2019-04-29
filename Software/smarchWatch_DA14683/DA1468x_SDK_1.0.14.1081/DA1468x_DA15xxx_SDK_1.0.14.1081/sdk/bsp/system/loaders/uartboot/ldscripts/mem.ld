/* Linker script to configure memory regions.
 * Need modifying for a specific board.
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 *   BUFFER.ORIGIN: starting address of buffer area
 *   BUFFER.LENGTH: length of buffer area
 *   REBOOT.ORIGIN: place for function that moves a binary from BUFFER section to address 0 and starts executing it.
 */

MEMORY
{
        RAM (rwx)    : ORIGIN = 0x00000000, LENGTH = 37K
        BUFFER (rw)  : ORIGIN = 0x07FC9400, LENGTH = 106K
        REBOOT (rwx) : ORIGIN = 0x07FE3C00, LENGTH = 1K
}
