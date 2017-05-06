#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

#include "prot_malloc.h"

static void removeEntry(uint64_t temp) 
{
    int i;
    printf("Address to be freed is %lu\n", temp);
    for( i = counter - 1; i >= 0; i-- ) 
    {
	if(canary_list[i] == temp)
	{
	    for(; i < counter - 1; i++ )
	    {
		canary_list[i] = canary_list[i + 1];
		size[i] = size[i + 1];
	    }
	}
	break;	
    }
    counter--;
}

static void write_to_file(void *canary_address)
{
 /*
  * canary list takes in the address as its value
  */
    canary_list[counter++] = (unsigned long)canary_address;
    printf("The address inside canary_list at index %d is %lu\n", counter - 1, (unsigned long) canary_list[counter - 1]);
    //sleep(5);
    return;
}

static void read_from_file()
{
    int i;

    for(i = 0; i < counter; i++)
	printf("Value of canary list %d is %lu\n", i, canary_list[i]);
}

static int addToList(void *c_addr)
{
    int rc = 0;
    int i;
    void *canary_address = NULL;
    canary_address = c_addr;

 /*
  * canary list starts at this address
  */
    printf("Address of canary list %p\n", canary_list);

 /*
  * canary is at the address specified here
  */
    printf("Value of canary address %p\n", canary_address);
    rc = 1;
    write_to_file(canary_address);
    
    read_from_file();

    for(i = 0; i < counter; i++)
    {
        printf("Value of canary list %d is %lu\n", i, canary_list[i]);
    }

    return rc;
}

void* malloc(size_t sz)
{
    void *temp_addr = NULL;

    uint64_t canary_value = 100;
    void *(*protected_malloc)(size_t size) = dlsym(RTLD_NEXT, "malloc");
    printf("malloc memory size %d\n", (int)sz);

 /*
  * allocated x + 8 bytes of memory
  */
    void *ptr = protected_malloc(sz + CANARY_SIZE);
    size[counter] = sz;
 /*
  * store canary's value at ptr + sz
  */
    *(uint64_t *)(ptr + sz) = canary_value;
    printf("The value at ptr + size[counter] sz is %lu, %lu, %lu\n", size[counter], sz, *(unsigned long *)(ptr + size[counter]));

 /*
  * temp_addr has the address of ptr + sz
  */
    temp_addr = ptr + sz;

    printf("The value of canary_address is %lu\n", (unsigned long)temp_addr);

 /*
  * add the address to the list as a value
  */
    if(!addToList(temp_addr))
    {
        fprintf(stderr, "Failed to add the canary to the list\n");
    }
    else
    {
	fprintf(stdout, "Counter Value: %d\n", counter);
    }
    return ptr;
}

void free(void *p)
{
    void (*libc_free)(void*) = dlsym(RTLD_NEXT, "free");
    printf("free\n");
    printf("Value of p + size[counter] is %lu\n", *(uint64_t *)(p + size[counter]));
    printf("Address of p + size[counter] is %lu\n", (uint64_t)(p + size[counter]));
    *(uint64_t *)(p + size[counter]) = 0;
    printf("Value of p + size[counter] is %lu\n", *(uint64_t *)(p + size[counter]));
    printf("Address of p + size[counter] is %lu\n", (uint64_t)(p + size[counter]));
    uint64_t temp = (uint64_t)(p + size[counter]);
    printf("Freeing counter %d\n", counter);
    printf("Value of temp is %lu\n", temp);
    libc_free(p);

    removeEntry(temp);
    /*
     * TODO:
     * Once the canary table is built, we need to remove the freed
     * canary from it.
     */
}

