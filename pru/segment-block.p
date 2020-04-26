#include "platform.hp"
#include "macros.hp"
#include "common.h"

#define ram_bits r18
#define status	 	ram_bits.b0
#define num_columns	ram_bits.b1
#define num_rows	ram_bits.b2
#define bit_depth	ram_bits.b3
#define read_buffer_addr r19.w0
#define read_buffer_offset r19.w2
#define enable_ticks0 r20
#define scratch r21
#define RAM_BITS_LENGTH 16

#define STRIDE	num_rows

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

#define WRITE_DATA(channel, value, bit) 	\
	QBBC 	DATA_CLR_##channel, value, bit; \
	SET	r30, DATA_##channel##_BIT; 			\
	QBA	DATA_END_##channel;   				\
DATA_CLR_##channel:; 						\
	CLR	r30, DATA_##channel##_BIT; 			\
	QBA	DATA_END_##channel; 				\
DATA_END_##channel:

#define bit_counter		r17.w0
#define bits_in_row		r17.w2
#define enable_ticks	r16
#define blk_addr		r15
#define blk_addr0		r14.w0
#define blk_offset0		r14.w2
#define blk_offset		r13.w0
#define bcm_bit			r13.b2
#define csel_counter	r13.b3

.origin 0
.entrypoint START

START:
	RESET_RAM_BLOCK_PTR
	LBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	MULT	num_columns, BITS_IN_BLOCK, bits_in_row

	CLR		r30, LATCH_BIT

RENDER:
	// store values here, we don't switch buffers mid BCM
	MOV		blk_addr0, read_buffer_addr
	MOV		blk_offset0, read_buffer_offset
	MOV		bcm_bit, 0

BCM_LOOP:
 	MOV		blk_offset, blk_offset0
	MOV		blk_addr, blk_addr0
	SET_RAM_BLOCK_PTR blk_addr

	MOV		enable_ticks, enable_ticks0
	LSL		enable_ticks, enable_ticks, bcm_bit
	MOV		csel_counter, 0

ROW_LOOP:
	MOV		bit_counter, 0

BIT_LOOP:
	CLR		r30, CLK_BIT

	MOV		r0.b0, num_rows
	LBCO	r1, DATA_BLOCK_PTR, blk_offset, b0

	WRITE_DATA (1, r1.b0, bcm_bit)
	WRITE_DATA (2, r1.b0, bcm_bit)
	WRITE_DATA (3, r1.b0, bcm_bit)
	WRITE_DATA (4, r1.b0, bcm_bit)
	WRITE_DATA (5, r1.b0, bcm_bit)
	WRITE_DATA (6, r1.b0, bcm_bit)

	SET		r30, CLK_BIT
	DELAY	CLK_DELAY, 10

	ADD		scratch, scratch, 1

	ADD		blk_offset, blk_offset, STRIDE
	QBLT	INCREMENT_BLOCK, blk_offset, 255
	//	delay?
	QBA		CONTINUE_BIT

INCREMENT_BLOCK:
	ADD		blk_addr, blk_addr, 1
	SET_RAM_BLOCK_PTR blk_addr
	AND		blk_offset, blk_offset, 255

CONTINUE_BIT:
	ADD		bit_counter, bit_counter, 1
	QBGT	BIT_LOOP, bit_counter, bits_in_row

ROW_DONE:
	SET		r30, BLANK_BIT

	INIT_GPIO1
	WRITE_GPIO1	csel_counter, 0, CSEL0_GPIO1_BIT
	WRITE_GPIO1	csel_counter, 1, CSEL1_GPIO1_BIT
	WRITE_GPIO1	csel_counter, 2, CSEL2_GPIO1_BIT
	COMMIT_GPIO1

	SET		r30, LATCH_BIT
	DELAY	LATCH_DELAY, 3
	CLR		r30, LATCH_BIT

	CLR		r30, BLANK_BIT
	DELAY	ENABLE, enable_ticks
//	SET		r30, BLANK_BIT

	ADD		csel_counter, csel_counter, 1
	QBGT	ROW_LOOP, csel_counter, COMMON_OUTPUTS

CSEL_DONE:
	ADD		bcm_bit, bcm_bit, 1
	QBGT	BCM_LOOP, bcm_bit, bit_depth



RENDER_DONE:

	QBA		RENDER

	// MOV		blk_offset, blk_offset0
	// MOV		blk_addr, blk_addr0
	// SET_RAM_BLOCK_PTR blk_addr

	// MOV		r0, 5 //num_rows
	// LBCO	r1, DATA_BLOCK_PTR, blk_offset, b0

	MOV		scratch, r2.b0
	DELAY	OFF, 100

	SET		r30, BLANK_BIT

	RESET_RAM_BLOCK_PTR
	SBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	MOV 	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	HALT
