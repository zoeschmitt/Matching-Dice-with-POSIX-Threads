#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>


//pthread functions need to take in a void pointer argument,
//and return a void pointer (or just NULL)
void* threadone(void* arg) {
    while(1) {
        sleep(1);
        printf("thread one\n");
    }
    return NULL;
}

void threadtwo() {
    while(1) {
        sleep(1);
        printf("thread two\n");
    }
}

int main() {
    pthread_t newthread;
    pthread_create(&newthread, NULL, threadone, NULL);
    threadtwo();
}