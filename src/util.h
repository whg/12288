#pragma once

#include "stdlib.h"
#include "stdio.h"

#define die(fmt, ...) printf(fmt"\n", ##__VA_ARGS__); exit(EXIT_FAILURE);
