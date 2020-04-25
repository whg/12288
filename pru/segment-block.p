#include "platform.hp"
#include "macros.hp"
#include "common.h"

#define ram_bits r18
#define status	 	ram_bits.b0
#define num_columns	ram_bits.b1
#define num_rows	ram_bits.b2
#define bit_depth	ram_bits.b3
#define buffer_block_addr r19
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

#define WRITE_DATA(channel, value, bit) 	\
	QBBC 	DATA_CLR_##channel, value, bit; \
	SET	r30, DATA_##channel##_BIT; 		\
	QBA	DATA_END_##channel;   				\
DATA_CLR_##channel:; 						\
	CLR	r30, DATA_##channel##_BIT; 		\
DATA_END_##channel:

#define row_data_length r17.w0

.origin 0
.entrypoint START

START:
	RESET_RAM_BLOCK_PTR
	LBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	MULT	num_columns, BITS_IN_BLOCK, r0
	MULT	r0, num_rows, row_data_length

	MOV		scratch, row_data_length
	DELAY	OFF, 100
	SBCO	ram_bits, DATA_BLOCK_PTR, 0, RAM_BITS_LENGTH

	MOV 	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
	HALT
