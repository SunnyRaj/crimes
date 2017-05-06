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
#include <signal.h>

#include <libvmi/libvmi.h>
//#include <libvmi/events.h>

//#include "./xc_pipe.h"
#include <time.h>
#include <sys/time.h>

#define COUNT 2

#define write_event_setup_ff "/home/sundarcs/event_to_restore"
#define vmi_read_ff "/home/sundarcs/xen_to_vmi"
#define vmi_write_ff "/home/sundarcs/vmi_to_xen"

#if 0
static int interrupted = 0;
static int mem_cb_count = 0;

static void print_mem_event(vmi_event_t *event);
static void close_handler(int sig);

vmi_event_t mem_event;

event_response_t mem_event_cb(vmi_instance_t vmi, vmi_event_t *event);
event_response_t step_cb(vmi_instance_t vmi, vmi_event_t *event);
#endif

struct timeval tv;

struct vmi_requirements
{
    uint64_t *st_addr;
    uint64_t *en_addr;
};

int main (int argc, char **argv)
{
    struct vmi_requirements vmi_req;
    char* name = NULL;
    vmi_instance_t vmi;
    char* bckup_name = NULL;
    unsigned long pid = 0;
    unsigned long long time;
    vmi_instance_t bckup_vmi;
    int ret_count;
    int a = 1;
    int b = 0;
    addr_t vaddr1 = 0;
//    struct sigaction act;

    if (argc < 4) {
        fprintf(stderr, "Usage: libvmi_detect <name of VM> <name of backup> <pid of proc in vm>\n");
        exit(1);
    }

    name = argv[1];
    bckup_name = argv[2];
    pid = strtoul(argv[3], NULL, 10);

/*
    act.sa_handler = close_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP,   &act, NULL);
    sigaction(SIGTERM,  &act, NULL);
    sigaction(SIGINT,   &act, NULL);
    sigaction(SIGALRM,  &act, NULL);
    sigaction(SIGKILL,  &act, NULL);
*/


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

    uint64_t *buf = malloc(sizeof(uint64_t));

    mkfifo(vmi_read_ff, 0666);        //Create Pipe 1
    canary_address = malloc(sizeof(uint64_t) * 20);
    vmi_read_fd = open(vmi_read_ff, O_RDONLY);      //Open Pipe 1 for Read
    vmi_write_fd = open(vmi_write_ff, O_WRONLY);      //open Pipe 2 for Write

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

        vaddr1 = *(vmi_req.st_addr);

        /*
         * Read the address at the starting address of the canary_list
         */
        ret_count = vmi_read_addr_va(vmi, vaddr1, pid, canary_address);
        printf("The address inside canary address: %lu is: %lu\n", vaddr1, *canary_address);
        /*
         * Read Canary from address canary_address[0]
         */
//        ret_count = vmi_read_addr_va(vmi, canary_address[0], pid, &canary);
//        printf("The value inside canary address: %lu is: %lu\n", *canary_address, canary);

        canary = *canary_address;

        if (canary != 100)
        {
            gettimeofday(&tv, NULL);
            time = tv.tv_sec * 1000000 + tv.tv_usec;
            printf("Wrong canary detected at time %llu\n", (unsigned long long) time);

//            printf("Wrong canary detected\n");
#ifndef DEB
            write(vmi_write_fd, f, sizeof(int));             //Write to Pipe 2
            fprintf(stderr, "Overflow encountered!!\n");
            fsync(vmi_write_fd);
        break;
#endif
        }
        else
        {
            write(vmi_write_fd, t, sizeof(int));             //Write to Pipe 2
            fprintf(stderr, "Written Successfully!!\n");
            fsync(vmi_write_fd);
        }

    canary = 0;
    ret_count = vmi_read_addr_va(vmi, /*canary_address[0]*/vaddr1 + 34, pid, canary_address);
        printf("The value inside canary address: %lu is: %lu\n", /*canary_address[0]*/vaddr1 + 34, *canary_address);

    canary = *canary_address;
    if (canary != 100)
        {
            gettimeofday(&tv, NULL);
            time = tv.tv_sec * 1000000 + tv.tv_usec;
            printf("Wrong canary detected at time %llu\n", (unsigned long long) time);
//            printf("Wrong canary detected\n");
#ifndef DEB
            write(vmi_write_fd, f, sizeof(int));             //Write to Pipe 2
            fprintf(stderr, "Overflow encountered!!\n");
            fsync(vmi_write_fd);
            break;
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
    printf("Look at canary address %lu\n", (vaddr1 + 34)/**canary_address*/);
    fsync(vmi_write_fd);

    uint64_t vaddr = vaddr1 + 34;

    vmi_destroy(vmi);

//    close(vmi_read_fd);
//    close(vmi_write_fd);
//
//    unlink(vmi_read_ff);

    /*
     * TODO:
     *  1) send to mem-events name of VM and canary address
     */

    write_event_setup_fd = open(write_event_setup_ff, O_WRONLY);
    write(write_event_setup_fd, &vaddr, sizeof(uint64_t));             //Write to Pipe 3
    printf("Giving address to event monitoring code\n");
    fsync(vmi_write_fd);

    return 0;

#if 0
    /*
     * set-up VMI event monitoring here
     */

    status_t status = VMI_SUCCESS;
    vmi = NULL;

    status = vmi_init(&vmi,
                      //(VMI_XEN | VMI_INIT_PARTIAL | VMI_INIT_EVENTS),
                      (VMI_XEN | VMI_INIT_COMPLETE | VMI_INIT_EVENTS),
                      bckup_name
                      );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to init LibVMI! :(\n");
        return 1;
    }
    else
    {
        fprintf(stdout, "LibVMI init success! :)\n");
    }

    /*
     * TODO:
     * We might not need to do this since the VM to monitor is already
     * paused from the remus side.  This needs to be resolved.
     */
//    status = vmi_pause_vm(vmi);
//    if (status == VMI_FAILURE)
//    {
//        fprintf(stdout, "Failed to pause VM...DIE!\n");
//        goto cleanup;
//    }

    fprintf(stdout,
            "Preparing memory event to monitor PA 0x%lx, page 0x%lx\n",
            *canary_address,
            (*canary_address >> 12)
            );

    memset(&mem_event, 0, sizeof(vmi_event_t));

    SETUP_MEM_EVENT(&mem_event,
                    *canary_address >> 12,
                    VMI_MEMACCESS_RW,
                    mem_event_cb,
                    0
                    );

    status = vmi_register_event(vmi, &mem_event);
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to register mem event...DIE!\n");
        goto cleanup;
    }

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

    while (!interrupted)
    {
        status = vmi_events_listen(vmi, 500);
        if (status != VMI_SUCCESS)
        {
            fprintf(stdout, "Error waiting for events, DIE...!\n");
            interrupted = -1;
        }
    }


cleanup:
    fprintf(stdout, "Finished mem-event test\n");

    vmi_destroy(vmi);

    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Exit with status VMI_FAILURE\n");
    }
    else
    {
        fprintf(stdout, "Exit with status VMI_SUCCESS\n");
    }

    return status;
#endif
}

#if 0
event_response_t
mem_event_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    status_t status = VMI_SUCCESS;

    print_mem_event(event);

    status = vmi_clear_event(vmi,
                             event,
                             NULL
                             );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to clear mem event in cb...DIE!\n");
        return 1;
    }

    /*
     * TODO:
     * This might not be needed since we are not interested in monitoring many
     * consecutive events.  This functionality has not been found to work before.
     */
//    status = vmi_step_event(vmi,
//                            event,
//                            event->vcpu_id,
//                            1,
//                            NULL
//                            );
//    if (status == VMI_FAILURE)
//    {
//        fprintf(stdout, "Failed to step event...DIE!\n");
//        return 1;
//    }


    /*
     * TODO:
     * Once we are here, an event has been detected at the canary_address.
     * Here, the VM is suspended and we need to figure out what exactly to do:
     *   1) leave it suspended and enter post-mortem phase
     *   2) do something more
     */

    return 0;
}

static void print_mem_event(vmi_event_t* event)
{
    fprintf(stdout,
            "PAGE %" PRIx64 " ACCESS: %c%c%c for GFN %" PRIx64 " (offset %06" PRIx64 ") gla %016" PRIx64 " (vcpu %u)\n",
                    event->mem_event.gfn >> 12,
            (event->mem_event.out_access & VMI_MEMACCESS_R) ? 'r' : '-',
            (event->mem_event.out_access & VMI_MEMACCESS_W) ? 'w' : '-',
            (event->mem_event.out_access & VMI_MEMACCESS_X) ? 'x' : '-',
            event->mem_event.gfn,
            event->mem_event.offset,
            event->mem_event.gla,
            event->vcpu_id
            );
}

static void close_handler(int sig)
{
    interrupted = sig;
}
#endif
