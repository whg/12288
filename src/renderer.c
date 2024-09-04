#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "renderer.h"
#include "util.h"
#include "common.h"

#define BUFFER_SIZE 32768
#define PORT 5050

static int g_socket_fd;
static struct pollfd g_poll_fd;
static pthread_t g_network_thread;
static pthread_mutex_t g_mutex;
static pthread_cond_t g_condv = PTHREAD_COND_INITIALIZER;
static uint8_t *g_data0, *g_data1;
static bool g_alive = true;
static int g_rows, g_cols, g_bit_depth;
static renderer_status_t g_status = RENDERER_NONE;
static uint32_t g_value;

static void* network_thread(void *_);

void renderer_init(int cols, int rows, int bit_depth) {
	g_rows = rows;
	g_cols = cols;
	g_bit_depth = bit_depth;

    struct sockaddr_in serveraddr;

    g_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_socket_fd < 0)
		die("can't open socket");

    int optval = 1;
    setsockopt(g_socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval , sizeof(int));
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);

	int binded = bind(g_socket_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if (binded < 0)
		die("error on socket bind");

    g_poll_fd.fd = g_socket_fd;
    g_poll_fd.events = POLLIN;

    int r = pthread_create(&g_network_thread, NULL, &network_thread, NULL);
	if (r != 0)
		die("can't create renderer thread");

	printf("listening on port %d\n", PORT);
}

static void* network_thread(void *_) {
	static uint8_t buffer[BUFFER_SIZE];
	for (;;) {
		int rv = poll(&g_poll_fd, 1, 100);
		if (rv > 0) {
			memset(buffer, 0, BUFFER_SIZE);
			int n = recv(g_socket_fd, buffer, BUFFER_SIZE, 0);
			if (n < 0)
				die("recv error");

			printf("received %d bytes: %c\n", n, (char) buffer[0]);
			
			bool ok = true;
			uint16_t size;
			uint8_t *d;
			int bcm_step = 32 * g_cols;
			int csel_step = bcm_step * 6;
			int half_size = bcm_step * 5;
			int k;
			
			switch (buffer[0]) {
			case 'F':
				size = buffer[2];
				size <<= 8;
				size |= buffer[1];
				d = buffer + 3;

				memcpy(g_data0, d, 10240);
				memcpy(g_data1, d + 10240, 10240);
				
				/* for (int csel = 0; csel < 8; csel++) { */
				/* 	k = csel * csel_step; */
				/* 	/\* printf("%p %p %d %d %d\n", g_data0, d, k, bcm_step, half_size); *\/ */
				/* 	//memcpy(g_data0, d + k + bcm_step, half_size); */
				/* 	memcpy(g_data1, d + k + bcm_step, half_size); */

				/* 	for (int i = 0; i < bcm_step; i++) { */
				/* 		if (d[k + i] */
				/* 		printf("%d %d\n", g_data0[k1 + i], d[k0 + i]); */
				/* 		g_data0[k1 + i] |= d[k0 + i]; */

				/* 	} */
				/* } */

				g_status = RENDERER_NEW_FRAME;
				
				/* if (buffer[2] == g_rows && buffer[1] == g_cols) { */
				/* 	memcpy(g_data, buffer + 3, g_rows * g_cols * SEGMENTS_IN_BLOCK); */
				/* 	puts("writing data"); */
				/* 	g_status = RENDERER_NEW_FRAME; */
				/* } else { */
				/* 	printf("size mismatch %d %d, %d, %d\n", */
				/* 		   (int) buffer[2], g_rows, (int) buffer[1], g_cols); */
				/* 	ok = false; */
				/* } */
				break;
			case 'B':
				g_value = buffer[1];
				g_status = RENDERER_BIT_DEPTH;
				break;
			case 'E':
				g_value = ((uint32_t) buffer[2]) << 8 | buffer[1];
				g_status = RENDERER_ENABLE_TICKS;
				break;

			default:
				printf("unknown %c\n", buffer[0]);
				ok = false;
			}
				
			if (ok)
				pthread_cond_signal(&g_condv);
		}

		pthread_mutex_lock(&g_mutex);
		if (!g_alive)
			break;
		pthread_mutex_unlock(&g_mutex);

	}
	return NULL;
}

renderer_status_t renderer_write_frame(uint8_t *data0, uint8_t *data1) {
	g_data0 = data0;
	g_data1 = data1;
	pthread_mutex_lock(&g_mutex);
	pthread_cond_wait(&g_condv, &g_mutex);
	pthread_mutex_unlock(&g_mutex);
	return g_alive ? g_status : RENDERER_NONE;
}

uint32_t renderer_get_value() {
	return g_value;
}

void renderer_stop() {
	pthread_mutex_lock(&g_mutex);
	g_alive = 0;
	pthread_mutex_unlock(&g_mutex);
	pthread_cond_signal(&g_condv);
}

void renderer_close() {
	pthread_join(g_network_thread, NULL);
}
