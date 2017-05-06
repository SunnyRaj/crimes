#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

struct timeval tv;

#define DEBUG 1

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

void exploit(char *ptr1)
{
    unsigned long long time;
    int i = 0;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec * 1000000 + tv.tv_usec;
/* 
    strcpy(ptr1, "abchijkl");
    usleep(200);
    strcpy(ptr1, "fghijkl");
    usleep(200);
    strcpy(ptr1, "hijkl");
    usleep(200);
    strcpy(ptr1, "jkl");
    usleep(200);
    strcpy(ptr1, "ijkl");
    usleep(200);
    strcpy(ptr1, "kl");
    usleep(200);
*/

    printf("Timestamp 1 %llu\n", (unsigned long long) time);
    printf("Address of ptr1[10] is %p\n", &ptr1[10]); 
    strcpy(ptr1, "abcdefghijkl");
    printf("Address of ptr1[10] is %p\n", &ptr1[10]); 

    gettimeofday(&tv, NULL);
    time = tv.tv_sec * 1000000 + tv.tv_usec;
    printf("Timestamp 2 %llu\n", (unsigned long long) time);
    while(1)
    {
	usleep(1000);
	printf("Counter %d\n", i++); 
    }
}

int main()
{
    int i;
    unsigned long long time;

    printf("$$$ test-malloc running with PID = %d\n", getpid());

    while(1)
    {
        int *ptr = malloc(sizeof(int) * 10);
        printf("Address of ptr is %p\n", &ptr); 
        char *ptr1 = malloc(sizeof(char) * 10);
        printf("Address of ptr1 is %p\n", ptr1); 
        printf("Address of ptr1[10] is %p\n", &ptr1[10]); 
        char *ptr2 = malloc(sizeof(char) * 10);
        char *ptr3 = malloc(sizeof(char) * 10);
        char *ptr4 = malloc(sizeof(char) * 10);
        char *ptr5 = malloc(sizeof(char) * 10);
        char *ptr6 = malloc(sizeof(char) * 10);
        char *ptr7 = malloc(sizeof(char) * 10);
        char *ptr8 = malloc(sizeof(char) * 10);
        char *ptr9 = malloc(sizeof(char) * 10);
        char *ptr10 = malloc(sizeof(char) * 10);
        char *ptr11 = malloc(sizeof(char) * 10);
        char *ptr12 = malloc(sizeof(char) * 10);
        char *ptr13 = malloc(sizeof(char) * 10);
        char *ptr14 = malloc(sizeof(char) * 10);
        char *ptr15 = malloc(sizeof(char) * 10);
        char *ptr16 = malloc(sizeof(char) * 10);
        int size = sizeof(ptr);
        int size1 = sizeof(ptr1);
        int size2 = sizeof(ptr2);
        int size3 = sizeof(ptr3);
        int size4 = sizeof(ptr4);
        int size5 = sizeof(ptr5);
        int size6 = sizeof(ptr6);
        int size7 = sizeof(ptr7);
        int size8 = sizeof(ptr8);
        int size9 = sizeof(ptr9);
        int size10 = sizeof(ptr10);
        int size11 = sizeof(ptr11);
        int size12 = sizeof(ptr12);
        int size13 = sizeof(ptr13);
        int size14 = sizeof(ptr14);
        int size16 = sizeof(ptr16);
//        int size16 = sizeof(ptr16);

        fprintf(stdout, "\n$$$ Buffers allocated, ready to be hacked!\n");

        DEBUG_PAUSE();

        int count = 0;
        while (count++ <= 5000) {
            usleep(1000);
        }

        exploit(ptr1);
//        free(ptr);
//        free(ptr1);
//        free(ptr2);
//        free(ptr3);
//        free(ptr4);
//        free(ptr5);
//        free(ptr6);
//        free(ptr7);
//        free(ptr8);
//        free(ptr9);
//        free(ptr10);
//        free(ptr11);
//        free(ptr12);
//        free(ptr13);
//        free(ptr14);
//        free(ptr15);
//        free(ptr16);
    }
}

