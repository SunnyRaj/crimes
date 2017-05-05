#include <assert.h>
#include <arpa/inet.h>


#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libvmi/libvmi.h>

//#include "./xc_pipe.h"


struct vmi_requirements
{
    uint64_t *st_addr;
    uint64_t *en_addr;
};

#define COUNT 2
int main (char** argv, int argc)
{
    struct vmi_requirements vmi_req;
    char* name = "suse-web";      /* Change the name to your VM */
    vmi_instance_t vmi;
    char* bckup_name = "machine";
    vmi_instance_t bckup_vmi;
    int ret_count;
    int a = 1;
    int b = 0;

    unsigned long pid;
    pid = 2517;                     /* Add the pid of the process */
    int *t = malloc(sizeof(int));
    int *f = malloc(sizeof(int));
    t = &a;
    f = &b;
//    int jump = 32;
    uint64_t canary = 0;
    addr_t *canary_address = NULL;
    uint64_t counter = 0;


/*---------------------Linux Pipe---------------------------*/
    int vmi_read_fd;             //Linux Pipe 1
    int vmi_write_fd;            //Linux Pipe 2
    int write_event_setup_fd;

    char *write_event_setup_ff = "/tmp/event_to_restore";
    char * vmi_read_ff = "/tmp/xen_to_vmi";        //Linux Pipe
    char * vmi_write_ff = "/tmp/vmi_to_xen";
    uint64_t *buf = malloc(sizeof(uint64_t));

    mkfifo(vmi_read_ff, 0666);        //Create Pipe 1
    canary_address = malloc(sizeof(uint64_t) * 20);
    vmi_read_fd = open(vmi_read_ff, O_RDONLY);      //Open Pipe 1 for Read
    vmi_write_fd = open(vmi_write_ff, O_WRONLY);      //open Pipe 2 for Write
    write_event_setup_fd = open(write_event_setup_ff, O_WRONLY);

    if (vmi_init(&vmi, VMI_AUTO | VMI_INIT_COMPLETE, name) == VMI_FAILURE) {
        printf("Failed to init LibVMI library.\n");
            return 1;
    }

    printf("success to init LibVMI\n");

    while(1)
    {
        /*
         * Read the address of canary_list; buf has the address of canary_list
         */
	printf("Process ID is %lu\n", pid);
        read(vmi_read_fd, buf, sizeof(void *));
        fprintf(stderr,"Address of canary list received from Save: %lu\n", *buf);

        vmi_req.st_addr = buf;

    	addr_t vaddr1 = *(vmi_req.st_addr);

	/*
	 * Read the address at the starting address of the canary_list
	 */
	ret_count = vmi_read_addr_va(vmi, vaddr1, pid, canary_address);
    	printf("The address inside canary address: %lu is: %lu\n", vaddr1, *canary_address);
	/*
         * Read Canary from address canary_address[0]
         */
	ret_count = vmi_read_addr_va(vmi, canary_address[0], pid, &canary);
    	printf("The value inside canary address: %lu is: %lu\n", *canary_address, canary);

	if (canary != 100)
        {
	    printf("Wrong canary detected\n");
#ifndef DEB
            write(vmi_write_fd, t, sizeof(int));             //Write to Pipe 2
	    fprintf(stderr, "Overflow encountered!!\n");
            fsync(vmi_write_fd);
//	    break;
#endif
	}
 	else
	{
            write(vmi_write_fd, t, sizeof(int));             //Write to Pipe 2
            fprintf(stderr, "Written Successfully!!\n");
            fsync(vmi_write_fd);
	}

	canary = 0;
	ret_count = vmi_read_addr_va(vmi, canary_address[0] + 34, pid, &canary);
    	printf("The value inside canary address: %lu is: %lu\n", canary_address[0] + 34, canary);

	if (canary != 100)
        {
	    printf("Wrong canary detected\n");
#ifndef DEB
            write(vmi_write_fd, t, sizeof(int));             //Write to Pipe 2
	    fprintf(stderr, "Overflow encountered!!\n");
            fsync(vmi_write_fd);
//	        break;
#endif
	}
 	else
	{
            write(vmi_write_fd, t, sizeof(int));             //Write to Pipe 2
            fprintf(stderr, "Written Successfully!!\n");
            fsync(vmi_write_fd);
	}

/*
	loop_addr = canary_address[0] + 34;
	for(i = 2; i < 17; i++)
	{
	    ret_count = vmi_read_addr_va(vmi, loop_addr + jump, pid, &canary);
	    jump += 32;

	    if (canary != 100)
            {
	        printf("Wrong canary detected\n");
                write(vmi_write_fd, f, sizeof(int));             //Write to Pipe 2
	        fprintf(stderr, "Overflow encountered!!\n");
                fsync(vmi_write_fd);
	        //break;
	    }
 	    else
	    {
                write(vmi_write_fd, t, sizeof(int));             //Write to Pipe 2
                fprintf(stderr, "Written Successfully!!\n");
                fsync(vmi_write_fd);
	    }
        }
	jump = 32;
*/
        canary = 0;
    }

    write(vmi_write_fd, f, sizeof(int));             //Write to Pipe 2
    fprintf(stderr, "Overflow encountered!!\n");
    printf("Look at canary address %lu\n", *canary_address);
    fsync(vmi_write_fd);

    vmi_destroy(vmi);

    close(vmi_read_fd);
    close(vmi_write_fd);

    unlink(vmi_read_ff);



/*
 * set-up VMI event monitoring here
 */

/*
 * Tell the restore code that event monitoring has been set-up in order to
 * unpause the back-up VM
 */
    write(write_event_setup_fd, t, sizeof(int));
    fsync(write_event_setup_fd);

/*
 * Perform event-monitoring here
 */
    close(write_event_setup_fd);

/*
 * Set up backup VMs vmi here
 */

    if (vmi_init(&bckup_vmi, VMI_AUTO | VMI_INIT_COMPLETE, bckup_name) == VMI_FAILURE) {
        printf("Failed to init LibVMI library for Backup VM.\n");
            return 1;
    }
    printf("success to init LibVMI for Backup VM\n");

    return 1;
}
