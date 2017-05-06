#ifdef _GNU_SOURCE
#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <inttypes.h> 
#define CANARY_SIZE 8
#define LIST_SIZE 256

uint64_t canary_list[2048];
size_t size[2048];
//uint64_t canary_value = -100;
static int counter = 0;
//const char* file_name = "valid_canaries";

static int addToList(void * );

static void removeEntry(uint64_t );

static void write_to_file(void *canary_address);

static void read_from_file();

#endif
