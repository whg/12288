#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <argp.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include "util.h"
#include "display.h"
#include "common.h"

typedef struct {
	 uint8_t status, num_columns, num_rows, bit_depth;
	 uint16_t buffer_addr, buffer_offset;
	 uint32_t enable_ticks;
	 uint32_t scratch;
} __attribute__((__packed__)) pru_ram_data_t;

typedef struct {
	 uint8_t num_columns, num_rows, bit_depth;
	 uint32_t enable_ticks;
	 int quiet;
} options_t;

struct argp_option argp_options[] = {
	 { "num_columns", 'c', "cols", 0, "number of block columns" },
	 { "num_row", 'r', "rows", 0, "number of block rows" },
	 { "bit_depth", 'b', "bits", 0, "grayscale bit depth" },
	 { "enable_ticks", 'e', "ticks", 0, "number of ticks LSB of enabled for" },
	 { "quiet", 'q', 0, 0, "Stop being so chatty" },
	 { 0 }
};

#define CASE_ARG_INT(param, chr) case chr: options->param = atoi(arg); break;
#define CASE_ARG_FLOAT(param, chr) case chr: options->param = atof(arg); break;

static char args_doc[] = "";

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	 options_t *options = (options_t*) state->input;
	 switch (key) {
		  CASE_ARG_INT(num_columns, 'c');
		  CASE_ARG_INT(num_rows, 'r');
		  CASE_ARG_INT(bit_depth, 'b');
		  CASE_ARG_INT(enable_ticks, 'e');
	 case 'q':
		  options->quiet = 1;
		  break;
	 case ARGP_KEY_ARG:
		  if (state->arg_num >= 0) {
			   argp_usage(state); // too many args
		  }
		  break;
	 default:
		  return ARGP_ERR_UNKNOWN;
	 }
	 return 0;
}

pru_ram_data_t *pru_ram;

void stop() {
	 pru_ram->status = STATUS_EXIT;
}

int main(int argc, char *argv[]) {
	 signal(SIGTERM, stop);
	 signal(SIGINT, stop);

	 options_t options = {
		  .num_columns = 1,
		  .num_rows = 1,
		  .bit_depth = 8,
		  .enable_ticks = 250,
		  .quiet = 0,
	 };

	 struct argp argp = { argp_options, parse_opt, args_doc, 0 };
	 argp_parse(&argp, argc, argv, 0, 0, &options);


	 prussdrv_init();
	 if (prussdrv_open(PRU_EVTOUT_0)) {
		  die("prussdrv_open failed");
	 }

	 tpruss_intc_initdata pruss_init_data = PRUSS_INTC_INITDATA;
	 prussdrv_pruintc_init(&pruss_init_data);

	 pixel_t *shared_ram;

	 prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**) &pru_ram);
	 prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, (void**) &shared_ram);

	 uint32_t ram0_offset = sizeof(pru_ram_data_t);
	 display_set_buffer(0, ((uint8_t*) pru_ram) + ram0_offset, 0, ram0_offset);
	 display_set_buffer(1, shared_ram, 0x100, 0);

	 pru_ram->status = STATUS_NONE;
	 pru_ram->num_columns = options.num_columns;
	 pru_ram->num_rows = options.num_rows;
	 pru_ram->bit_depth = options.bit_depth;
	 pru_ram->buffer_addr = display_read_addr();
	 pru_ram->enable_ticks = options.enable_ticks;
	 pru_ram->scratch = 0;

	 display_configure(options.num_columns, options.num_rows);
	 display_debug();
	 display_swap_buffers();
	 display_debug();
	 display_swap_buffers();

	 pru_ram->buffer_addr = display_read_addr();
	 pru_ram->buffer_offset = display_read_offset();

	 pixel_t *data = display_read_ptr();
	 for (int i = 0; i < 8; i++) {
		  printf("%d, ", data[i]);
	 }
	 printf("\n");

	 printf("offset = %u, %d\n", pru_ram->buffer_addr, pru_ram->buffer_offset);

	 int exec_fail = prussdrv_exec_program(PRU0, "./build/segment-block.bin");
	 if (exec_fail) {
		  die("can't exec pru program");
	 }

	 pru_ram->status = STATUS_RENDER;
	 while (pru_ram->status == STATUS_RENDER) {
	 	  display_debug();
	 	  display_swap_buffers();
		  pru_ram->buffer_addr = display_read_addr();
	 	  pru_ram->buffer_offset = display_read_offset();
	 	  usleep(1000000 / 30);
	 }


	 prussdrv_pru_wait_event(PRU_EVTOUT_0);
	 prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);


	 printf("scratch = %u\n", pru_ram->scratch);

	 prussdrv_pru_disable(PRU0);
	 prussdrv_exit();

	 return EXIT_SUCCESS;
}
