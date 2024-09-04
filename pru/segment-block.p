#include "platform.hp"
#include "macros.hp"
#include "common.h"

#define ram_bits r18
#define status	 	ram_bits.b0
#define num_columns	ram_bits.b1
#define num_rows	ram_bits.b2
#define bit_depth	ram_bits.b3
#define bcm_bits_buffer_addr r19.w0
#define bcm_bits_buffer_offset r19.w2
#define enable_ticks0 r20
#define scratch r21
#define RAM_BITS_LENGTH 16

#define CLK_BIT		7
#define LATCH_BIT	3
#define BLANK_BIT	2

#define DATA_1_BIT	0
#define DATA_2_BIT	14
#define DATA_3_BIT	1
#define DATA_4_BIT	6
#define DATA_5_BIT	4
#define DATA_6_BIT	5
#define DATA_7_BIT	15

#define CSEL0_GPIO1_BIT	11
#define CSEL1_GPIO1_BIT	9
#define CSEL2_GPIO1_BIT	10

#define WRITE_SEGMENT_COLUMN(reg, byte) 		\
	CLR	r30, CLK_BIT; 				\
	MOV	r0, r##reg.b##byte; 			\
	QBBC	SET_CLK##reg##byte, r0, 7; 		\
	SET	r0, DATA_2_BIT; 			\
	QBA	WRITE_SEGMENT_COLUMN_DONE##reg##byte; 	\
SET_CLK##reg##byte:; 					\
	SET	r0, CLK_BIT; 				\
WRITE_SEGMENT_COLUMN_DONE##reg##byte:; 			\
	MOV	r30, r0; 				\	
	DELAY	1
	
#define WRITE_SEGMENT_COLUMN_REG(reg) 			\
	WRITE_SEGMENT_COLUMN(reg, 0); 			\
	WRITE_SEGMENT_COLUMN(reg, 1); 			\
	WRITE_SEGMENT_COLUMN(reg, 2); 			\
	WRITE_SEGMENT_COLUMN(reg, 3)

	
#define column_counter		r17.w0
#define bits_in_row		r17.w2
#define enable_ticks		r16
#define block_address		r15
#define dither_counter		r14
#define block_offset		r13.w0
#define bcm_bit_counter		r13.b2
#define csel_counter		r13.b3
#define bcm_bit			r12.b0
#define csel_set		r12.b1
	
.origin 0
.entrypoint START

START:
	ENABLE_OCP_MASTER_PORT
	RAMBLK	0
	LBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	MULT	num_columns, BITS_IN_BLOCK, bits_in_row

	CLR	r30, LATCH_BIT
	SET	r30, BLANK_BIT

	LDI	dither_counter, 0

	
WAIT_FOR_FRAME:
	LBCO	status, DATA_BLOCK_PTR, 0, 1
	DELAY	65000
	QBEQ	EXIT, status, STATUS_EXIT
	QBEQ	WAIT_FOR_FRAME, status, STATUS_NONE


LOAD_FRAME:
	RAMBLK 	0
	LBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	
RENDER:
	MOV	csel_counter, 0
	LDI	csel_set, COMMON_OUTPUTS
	
	QBEQ	DITHER_DATA_A, dither_counter, 0
	QBEQ	DITHER_DATA_B, dither_counter, 1
DITHER_DATA_A:
	LDI	block_address, 0x001
	QBA	SET_BLOCK_ADDRESS
DITHER_DATA_B:
	LDI	block_address, 0x100

SET_BLOCK_ADDRESS:
	RAMBLK 	block_address
	LDI	block_offset, 0
	
	XOR	dither_counter, dither_counter, 1
	
CSEL_LOOP:
	
	MOV	bcm_bit_counter, 0

BCM_LOOP:
	MOV	column_counter, 0
	
	MOV	enable_ticks, enable_ticks0
	LSL	enable_ticks, enable_ticks, bcm_bit_counter
	SUB	enable_ticks, enable_ticks, enable_ticks0
	ADD	enable_ticks, enable_ticks, 1

	
BLOCK_LOOP:	
	LBCO	r1, DATA_BLOCK_PTR, block_offset, 32

	WRITE_SEGMENT_COLUMN_REG(1)
	WRITE_SEGMENT_COLUMN_REG(2)
	WRITE_SEGMENT_COLUMN_REG(3)
	WRITE_SEGMENT_COLUMN_REG(4)
	WRITE_SEGMENT_COLUMN_REG(5)
	WRITE_SEGMENT_COLUMN_REG(6)
	WRITE_SEGMENT_COLUMN_REG(7)
	WRITE_SEGMENT_COLUMN_REG(8)
	
	ADD	block_offset, block_offset, 32
	QBLT	INCREMENT_BLOCK, block_offset, 255
	QBA	NEXT_BLOCK

INCREMENT_BLOCK:
	ADD	block_address, block_address, 1
	RAMBLK 	block_address
	AND	block_offset, block_offset, 255
	DELAY	100

NEXT_BLOCK:
	ADD	column_counter, column_counter, 1
	QBGT	BLOCK_LOOP, column_counter, num_columns

ROW_DONE:

	QBEQ	LATCH, csel_set, csel_counter
	
SET_CSEL:
	SET	r30, BLANK_BIT

	INIT_GPIO1
	WRITE_GPIO1	csel_counter, 0, CSEL0_GPIO1_BIT
	WRITE_GPIO1	csel_counter, 1, CSEL1_GPIO1_BIT
	WRITE_GPIO1	csel_counter, 2, CSEL2_GPIO1_BIT
	COMMIT_GPIO1

	MOV	csel_set, csel_counter
	
LATCH:	
	SET	r30, LATCH_BIT
	DELAY	3
	CLR	r30, LATCH_BIT

	CLR	r30, BLANK_BIT
	DELAY	enable_ticks	
	
	ADD	bcm_bit_counter, bcm_bit_counter, 1
	QBGT	BCM_LOOP, bcm_bit_counter, bit_depth
	
BCM_DONE:
	ADD	csel_counter, csel_counter, 1
	QBGT	CSEL_LOOP, csel_counter, COMMON_OUTPUTS

RENDER_DONE:
	RAMBLK 0
	LBCO	status, DATA_BLOCK_PTR, 0, 1
	QBEQ	LOAD_FRAME, status, STATUS_NEW_FRAME
	QBEQ	RENDER, status, STATUS_RENDER

	RAMBLK 	0x3f
	LBCO	r0, DATA_BLOCK_PTR, 252, 4
	
	MOV	scratch, dither_counter
	DELAY	100
EXIT:	
	SET	r30, BLANK_BIT

	RAMBLK 	0
	SBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	DELAY	50000
	MOV 	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	HALT
