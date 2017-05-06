#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <libvmi/libvmi.h>
#include <libvmi/events.h>

#define DEBUG 0
#define PIPE_BUF_TO_MEM_FD "/home/sundarcs/buf_to_mem"

#define DEBUG_PAUSE()                                                       \
    do {                                                                    \
        if (DEBUG) {                                                        \
            printf("[%d] Hit [ENTER] to continue exec...\n", __LINE__);     \
                char enter = 0;                                             \
                while (enter != '\r' && enter != '\n') {                    \
                    enter = getchar();                                      \
                }                                                           \
        }                                                                   \
    } while(0)


static int interrupted = 0;
vmi_event_t mem_event;
typedef long long NANOSECONDS;
typedef struct timespec TIMESPEC;

static void print_mem_event(vmi_event_t *event);
static inline NANOSECONDS ns_timer(void);
static void close_handler(int sig);
event_response_t mem_event_cb(vmi_instance_t vmi, vmi_event_t *event);
event_response_t step_cb(vmi_instance_t vmi, vmi_event_t *event);

int main(int argc, char **argv)
{
    status_t status = VMI_SUCCESS;
    vmi_instance_t vmi = NULL;
    struct sigaction act;
    int pipe_buf_to_mem = 0;
//    char* vm_name = NULL;
    char* vm_name = "ubuntu-hvm";
    unsigned long pid = 0UL;
    addr_t vaddr = 0ULL;
    addr_t paddr = 0ULL;

//    if (argc < 4) {
//        fprintf(stderr, "Usage: mem-event <name of VM> <pid>  <vaddr>\n");
//        exit(1);
//    }

    fprintf(stdout, "Started Mem-Events Program\n");

    act.sa_handler = close_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP,   &act, NULL);
    sigaction(SIGTERM,  &act, NULL);
    sigaction(SIGINT,   &act, NULL);
    sigaction(SIGALRM,  &act, NULL);
    sigaction(SIGKILL,  &act, NULL);

//    vm_name = argv[1];
//    pid = strtoul(argv[2], NULL, 10);
//    vaddr = strtoull(argv[3], NULL, 10);
    pid = strtoul(argv[1], NULL, 10);

    fprintf(stdout, "Waiting to receive vaddr to monitor from pipe\n");

    mkfifo(PIPE_BUF_TO_MEM_FD, 0666);
    pipe_buf_to_mem = open(PIPE_BUF_TO_MEM_FD, O_RDONLY);
    read(pipe_buf_to_mem, &vaddr, sizeof(addr_t));

    fprintf(stdout, "Received vaddr from buffer overflow module\n");

    DEBUG_PAUSE();

    fprintf(stdout,
            "Attempting to monitor vaddr %lx in PID %lx on VM %s",
            vaddr,
            pid,
            vm_name);

    fprintf(stdout, "[TIMESTAMP] Received vaddr, initializing VMI. %lld ns", ns_timer());

    status = vmi_init(&vmi,
                      (VMI_XEN | VMI_INIT_COMPLETE | VMI_INIT_EVENTS),
                      vm_name);
    if (status == VMI_FAILURE) {
        fprintf(stdout, "Failed to init LibVMI! :( %m\n");
        return 1;
    } else {
        fprintf(stdout, "LibVMI init success! :)\n");
    }

//    status = vmi_pause_vm(vmi);
//    if (status == VMI_FAILURE) {
//        fprintf(stdout, "Failed to pause VM...DIE! %m\n");
//        goto cleanup;
//    }

    paddr = vmi_translate_uv2p(vmi, vaddr, pid);
    if (paddr == 0) {
        fprintf(stdout, "Failed to translate uv2p...DIE! %m\n");
        status = VMI_FAILURE;
        goto cleanup;
    }

    fprintf(stdout, "Monitoring paddr %lx on \"%s\"\n", paddr, vm_name);

    fprintf(stdout,
            "Preparing memory event to monitor PA 0x%lx, page 0x%lx\n",
            paddr,
            (paddr >> 12));

    memset(&mem_event, 0, sizeof(vmi_event_t));
    SETUP_MEM_EVENT(&mem_event,
                    (paddr >> 12),
                    VMI_MEMACCESS_RW,
                    mem_event_cb,
                    0);

    status = vmi_register_event(vmi, &mem_event);
    if (status == VMI_FAILURE) {
        fprintf(stdout, "Failed to register mem event...DIE! %m\n");
        goto cleanup;
    }

    fprintf(stdout, "[TIMESTAMP] VMI and event handler setup, resuming VM. %lld ns", ns_timer());

    status = vmi_resume_vm(vmi);
    if (status == VMI_FAILURE) {
        fprintf(stdout, "Failed to resume VM...DIE! %m\n");
        goto cleanup;
    }

    while (!interrupted) {
        status = vmi_events_listen(vmi, 500);
        if (status != VMI_SUCCESS) {
            fprintf(stdout, "Error waiting for events...DIE! %m\n");
            interrupted = -1;
        }
    }

cleanup:
    fprintf(stdout, "Finished mem-event test\n");

    vmi_destroy(vmi);

    if (status == VMI_FAILURE) {
        fprintf(stdout, "Exit with status VMI_FAILURE\n");
    } else {
        fprintf(stdout, "Exit with status VMI_SUCCESS\n");
    }

    return status;
}

event_response_t
mem_event_cb(vmi_instance_t vmi, vmi_event_t *event)
{
    status_t status = VMI_SUCCESS;

    fprintf(stdout, "[TIMESTAMP] Mem event found on vaddr. %lld ns", ns_timer());

    print_mem_event(event);

    status = vmi_clear_event(vmi, event, NULL);
    if (status == VMI_FAILURE) {
        fprintf(stdout, "Failed to clear mem event in cb...DIE! %m\n");
        return 1;
    }

//    status = vmi_step_event(vmi,
//                            event,
//                            event->vcpu_id,
//                            1,
//                            NULL);
//    if (status == VMI_FAILURE) {
//        fprintf(stdout, "Failed to step event...DIE! %m\n");
//        return 1;
//    }

    interrupted = 6;

    return 0;
}


static void
print_mem_event(vmi_event_t *event)
{
    fprintf(stdout,
            "PAGE %" PRIx64 " ACCESS: %c%c%c for GFN %" PRIx64 " (offset %06" PRIx64 ") gla %016" PRIx64 " (vcpu %u)\n",
            (event->mem_event.gfn >> 12),
            (event->mem_event.out_access & VMI_MEMACCESS_R) ? 'r' : '-',
            (event->mem_event.out_access & VMI_MEMACCESS_W) ? 'w' : '-',
            (event->mem_event.out_access & VMI_MEMACCESS_X) ? 'x' : '-',
            event->mem_event.gfn,
            event->mem_event.offset,
            event->mem_event.gla,
            event->vcpu_id
            );
}

static inline NANOSECONDS
ns_timer(void)
{
    TIMESPEC curr_time;
    clock_gettime(CLOCK_MONOTONIC, &curr_time);

    return (NANOSECONDS) (curr_time.tv_sec * 1000000000LL) +
           (NANOSECONDS) (curr_time.tv_nsec);
}

static void
close_handler(int sig)
{
    interrupted = sig;
}
