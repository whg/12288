

Offsets are:

| Start address | For PRU0 |
|---|---|
| 0x00000  | 8KB RAM 0 |
| 0x02000  | 8KB RAM 1 |
| 0x10000  | 12KB RAM 2 (Shared) 
| 0x22000  | Control registers |

Offsets:

| Offset | register |
|---|---|
| 0x28  | CTPPR0 |


C28 is constant table for PRU Shared RAM. This is a 256 byte page index, i.e. a pointer with a stride of 256. Because:


page 243: `0x00nn_nn00, nnnn = c28_pointer[15:0]`

You set C28 through the **CTPPR0** register (CTPPR: Constant Table Programmable Pointer Register). The least significant 16 bits of this register set C28, page 304.

