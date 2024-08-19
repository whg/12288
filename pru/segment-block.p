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

// #define WRITE_DATA(channel, value, bit) 	\
// 	QBBC 	DATA_CLR_##channel, value, bit; \
// 	SET	r30, DATA_##channel##_BIT; 			\
// 	QBA	DATA_END_##channel;   				\
// DATA_CLR_##channel:; 						\
// 	CLR	r30, DATA_##channel##_BIT; 			\
// 	QBA	DATA_END_##channel; 				\
// DATA_END_##channel:

#define WRITE_SEGMENT_COLUMN(reg, byte) 			\
	CLR		r30, CLK_BIT; 				\
	MOV		r0, r##reg.b##byte; 			\
	QBBC		SET_CLK##reg##byte, r0, 7; 		\
	SET		r0, DATA_2_BIT; 			\
	QBA		WRITE_SEGMENT_COLUMN_DONE##reg##byte; 	\
SET_CLK##reg##byte:; 						\
	SET		r0, CLK_BIT; 				\
WRITE_SEGMENT_COLUMN_DONE##reg##byte:; 				\
	MOV		r30, r0
	
#define WRITE_SEGMENT_COLUMN_REG(reg) 				\
	WRITE_SEGMENT_COLUMN(reg, 0); 				\
	WRITE_SEGMENT_COLUMN(reg, 1); 				\
	WRITE_SEGMENT_COLUMN(reg, 2); 				\
	WRITE_SEGMENT_COLUMN(reg, 3)

	
#define column_counter		r17.w0
#define bits_in_row		r17.w2
#define enable_ticks	r16
#define block_address		r15
#define block_address0		r14.w0
#define block_offset0		r14.w2
#define block_offset		r13.w0
#define bcm_bit			r13.b2
#define csel_counter	r13.b3

.origin 0
.entrypoint START

START:
	ENABLE_OCP_MASTER_PORT
	RESET_RAM_BLOCK_PTR
	LBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	MULT	num_columns, BITS_IN_BLOCK, bits_in_row

	CLR		r30, LATCH_BIT
	SET		r30, BLANK_BIT

WAIT_FOR_FRAME:
	LBCO	status, DATA_BLOCK_PTR, 0, 1
	DELAY	65000
	QBEQ	EXIT, status, STATUS_EXIT
	QBEQ	WAIT_FOR_FRAME, status, STATUS_NONE

LOAD_FRAME:
	// store values here, we don't switch buffers mid BCM
	RESET_RAM_BLOCK_PTR
	LBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH
	MOV		block_address0, read_buffer_addr
	MOV		block_offset0, read_buffer_offset

RENDER:
	MOV		bcm_bit, 0

BCM_LOOP:
 	MOV		block_offset, block_offset0
	MOV		block_address, block_address0
	SET_RAM_BLOCK_PTR block_address
	DELAY	60
	MOV		enable_ticks, enable_ticks0
	LSL		enable_ticks, enable_ticks, bcm_bit
	SUB		enable_ticks, enable_ticks, enable_ticks0
	ADD		enable_ticks, enable_ticks, 1
	MOV		csel_counter, 0


ROW_LOOP:
	MOV		column_counter, 0
	
BLOCK_LOOP:	
	LBCO		r1, DATA_BLOCK_PTR, block_offset, 32

BIT_LOOP:
	WRITE_SEGMENT_COLUMN_REG(1)
	WRITE_SEGMENT_COLUMN_REG(2)
	WRITE_SEGMENT_COLUMN_REG(3)
	WRITE_SEGMENT_COLUMN_REG(4)
	WRITE_SEGMENT_COLUMN_REG(5)
	WRITE_SEGMENT_COLUMN_REG(6)
	WRITE_SEGMENT_COLUMN_REG(7)
	WRITE_SEGMENT_COLUMN_REG(8)

	ADD		block_offset, block_offset, 32
	QBLT		INCREMENT_BLOCK, block_offset, 255
	QBA		CONTINUE_BIT

INCREMENT_BLOCK:
	ADD		block_address, block_address, 1
	SET_RAM_BLOCK_PTR block_address
	AND		block_offset, block_offset, 255
	DELAY	100


CONTINUE_BIT:
	ADD		column_counter, column_counter, 1
	QBGT		BLOCK_LOOP, column_counter, num_columns

ROW_DONE:
	SET		r30, BLANK_BIT

	INIT_GPIO1
	WRITE_GPIO1	csel_counter, 0, CSEL0_GPIO1_BIT
	WRITE_GPIO1	csel_counter, 1, CSEL1_GPIO1_BIT
	WRITE_GPIO1	csel_counter, 2, CSEL2_GPIO1_BIT
	COMMIT_GPIO1

	SET		r30, LATCH_BIT
	DELAY	3
	CLR		r30, LATCH_BIT

	CLR		r30, BLANK_BIT
	DELAY	enable_ticks	
//	SET		r30, BLANK_BIT
	
	ADD		csel_counter, csel_counter, 1
	QBGT	ROW_LOOP, csel_counter, COMMON_OUTPUTS

CSEL_DONE:
	ADD		bcm_bit, bcm_bit, 1
	QBGT	BCM_LOOP, bcm_bit, bit_depth


RENDER_DONE:
	RESET_RAM_BLOCK_PTR
	LBCO	status, DATA_BLOCK_PTR, 0, 1
	QBEQ	LOAD_FRAME, status, STATUS_NEW_FRAME
	QBEQ	RENDER, status, STATUS_RENDER

//	MOV		scratch, 
	DELAY	100
EXIT:	
	SET		r30, BLANK_BIT
//	CLR		r30, BLANK_BIT

	RESET_RAM_BLOCK_PTR
	SBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	DELAY	50000
	MOV 	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	HALT
