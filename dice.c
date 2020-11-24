#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <sys/sem.h>

#define NUM_PLAYERS 4

FILE* logFile;
int seed = 0;
int sums[NUM_PLAYERS];
bool gameOver = false;
int upNext = -1;

sem_t *sem_rolling;
pthread_cond_t winner = PTHREAD_COND_INITIALIZER;
pthread_cond_t upNextCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t dealerMutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t playerMutex = PTHREAD_MUTEX_INITIALIZER; 

void logResults(int player, int rollOne, int rollTwo) {
    char playerL;
    printf("in log with player %d\n", player);

    switch (player) {
    case 0:
        playerL = 'A';
        break;
    case 1:
        playerL = 'B';
        break;
    case 2:
        playerL = 'C';
        break;
    case 3:
        playerL = 'D';
        break;
    default:
        break;
    }

    fprintf(logFile, "Player %c: ", playerL);
    fprintf(logFile, "gets %d and %d with a sum of ", rollOne, rollTwo);
    fprintf(logFile, "%d\n", rollOne + rollTwo);

    printf("after log\n");
}

void finalLog(char player1, char player2) {
    fprintf(logFile, "DEALER: The winning team is %c and %c\n", player1, player2);
}

//checks if anyone has won
void checkStatus() {
    if (sums[0] != 0 && sums[1] != 0 && sums[2] != 0 && sums[3] != 0) {
        if (sums[0] == sums[2]) {
            gameOver = true;
            pthread_cond_signal(&winner);
            finalLog('A', 'C');
        } else if (sums[1] == sums[3]) {
            gameOver = true;
            pthread_cond_signal(&winner);
            finalLog('B', 'D');
        }
    }
}

void* dealer(void* arg) {
   
    sem_rolling = sem_open("rollingsem", IPC_CREAT, 0660, 1);

    upNext = rand() % 4; //gives random number between 0 and 3
    printf("dealer rolled: %d\n", upNext);
    pthread_cond_signal(&upNextCond); // signal the players

    pthread_mutex_lock(&dealerMutex); // wait till winner

    while (gameOver == false)
        pthread_cond_wait(&winner, &dealerMutex);

    pthread_mutex_unlock(&dealerMutex); 
    
    sem_close(sem_rolling);
    pthread_exit(NULL); 
}

void roll(int player) {
    printf("in roll\n");
    //sem_wait(sem_rolling); // block all threads but one (already have a mutex, but is a failsafe)
    int roll1 = rand() % 7;
    printf("roll1: %d\n", roll1);
    int roll2 = rand() % 7;
    printf("roll2: %d\n", roll2);
    sums[player] = roll1 + roll2; 
    logResults(player, roll1, roll2); // log the turn
    //sem_post(sem_rolling); // release
    printf("finished roll\n");
}

void* player(void* arg) {
    int *iptr = (int*) arg;
    printf("player thread started: %d\n", *iptr);
    while (!gameOver) {
        pthread_mutex_lock(&playerMutex); 
        while (upNext != *iptr)
            pthread_cond_wait(&upNextCond, &playerMutex);
        pthread_mutex_unlock(&playerMutex); 
        printf("player: %d rolling\n", *iptr);
        roll(*iptr);
        
        checkStatus();
        upNext >= 3 ? upNext = 0 : upNext++;
        pthread_cond_signal(&upNextCond);
        printf("updated\n");
    }
    pthread_exit(NULL); 
}

int main (int argc, char *argv[]) {
    int players[] = {0, 1, 2, 3};
    logFile = fopen("logfile.txt", "w");
    seed = atoi(argv[1]);
    srand(seed);
    pthread_t playerA;
    pthread_t playerB;
    pthread_t playerC;
    pthread_t playerD;
    pthread_t dealerThread;
    pthread_create(&dealerThread, NULL, dealer, NULL);
    pthread_create(&playerA, NULL, player, &players[0]);
    pthread_create(&playerB, NULL, player, &players[1]);
    pthread_create(&playerC, NULL, player, &players[2]);
    pthread_create(&playerD, NULL, player, &players[3]);

    pthread_join(dealerThread, NULL);
    fclose(logFile);
    pthread_exit(NULL); 
}