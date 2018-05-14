#include "lib.h"
#include "types.h"

int uEntry(void){	

	int i = 4;
    int ret = 0;
    // int value = 2;

    // sem_t sem;
    printf("Father Process: Semaphore Initializing.\n");
    // ret = sem_init(&sem, value);
    if (ret == -1) {
        printf("Father Process: Semaphore Initializing Failed.\n");
        exit(0);
    }

    ret = fork();
    if (ret == 0) {
        while( i != 0) {
            i --;
            printf("Child Process: Semaphore Waiting.\n");
            // sem_wait(&sem);
            printf("Child Process: In Critical Area.\n");
        }
        printf("Child Process: Semaphore Destroying.\n");
        // sem_destroy(&sem);
        exit(0);
    }
    else if (ret != -1) {
        while( i != 0) {
            i --;
            printf("Father Process: Sleeping.\n");
            sleep(3);
            printf("Father Process: Semaphore Posting.\n");
            // sem_post(&sem);
        }
        printf("Father Process: Semaphore Destroying.\n");
        // sem_destroy(&sem);
        exit(0);
    }
    
    return 0;

	// int i, ret;
	// for (i = 0; i < 3; i++) {
	// 	ret = fork();
	// 	printf("Process: ret = %d, i = %d;\n", ret, i);
	// 	sleep(1);
	// }
	// exit(0);

	// return 0;

	// int data = 0;

	// int ret = fork();
	// int i = 8;
	// if (ret == 0) {
	// 	data = 2;
	// 	while( i != 0) {
	// 		i --;
	// 		printf("Child Process: Pong %d, %d;\n", data, i);
	// 		sleep(1);
	// 	}
	// 	exit(0);
	// }
	// else if (ret != -1) {
	// 	data = 1;
	// 	while( i != 0) {
	// 		i --;
	// 		printf("Father Process: Ping %d, %d;\n", data, i);
	// 		sleep(1);
	// 	}
	// 	exit(0);
	// }
	
	// return 0;
}