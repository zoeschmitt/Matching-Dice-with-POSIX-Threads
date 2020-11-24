#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <sys/sem.h>

#define NUM_PLAYERS 4
#define SEM_NAME "rollingsem"

FILE* logFile;
int seed = 0;
int sums[NUM_PLAYERS]; // holds sums of each player
bool gameOver = false;
int upNext = -1; // dealer sets first player in dealer()

sem_t *sem_rolling;
pthread_cond_t winner = PTHREAD_COND_INITIALIZER; 
pthread_cond_t upNextCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t dealerMutex = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t playerMutex = PTHREAD_MUTEX_INITIALIZER; 

void logResults(int player, int rollOne, int rollTwo) {
    char playerL;

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

    printf("Player %c: ", playerL);
    printf("%d %d\n", rollOne, rollTwo);

    fprintf(logFile, "Player %c: ", playerL);
    fprintf(logFile, "gets %d and %d with a sum of ", rollOne, rollTwo);
    fprintf(logFile, "%d\n", rollOne + rollTwo);
}

void finalLog(char player1, char player2) {
    fprintf(logFile, "DEALER: The winning team is %c and %c\n", player1, player2);
}

//checks if anyone has won
bool checkStatus() {
    if (sums[0] != 0 && sums[1] != 0 && sums[2] != 0 && sums[3] != 0) {
        if (sums[0] == sums[2]) {
            gameOver = true;
            pthread_cond_broadcast(&winner);
            finalLog('A', 'C');
            printf("winner A & C\n");
            return true;
        } else if (sums[1] == sums[3]) {
            gameOver = true;
            pthread_cond_broadcast(&winner);
            finalLog('B', 'D');
            printf("winner B & D\n");
            return true;
        }
    }
    return false;
}

void* dealer(void* arg) {
   
    upNext = rand() % 4; //gives random number between 0 and 3
    printf("dealer rolled a %d\n", upNext);
    pthread_cond_broadcast(&upNextCond); // signal the players

    pthread_mutex_lock(&dealerMutex); // wait till winner
    while (!gameOver)
        pthread_cond_wait(&winner, &dealerMutex);
    pthread_mutex_unlock(&dealerMutex); 
    
    pthread_exit(NULL); 
}

void roll(int player) {
    int roll1 = rand() % 7;
    int roll2 = rand() % 7;
    sums[player] = roll1 + roll2; // update sums array
    logResults(player, roll1, roll2); // log the turn to file
}

void* player(void* arg) {
    int *iptr = (int*) arg;
    bool status = false;
    printf("player thread started: %d\n", *iptr);
    while (!gameOver) {
        pthread_mutex_lock(&playerMutex); 
        while (upNext != *iptr)
            pthread_cond_wait(&upNextCond, &playerMutex);
        pthread_mutex_unlock(&playerMutex); 
        status =  checkStatus(); // make sure someone else hasn't won already
        if (!status) {
            sem_wait(sem_rolling);  // make sure only one person is rolling at a time
            roll(*iptr);
            sem_post(sem_rolling);
            upNext >= 3 ? upNext = 0 : upNext++; // next turn
            pthread_cond_broadcast(&upNextCond); // let other players know its next turn
        } else 
            pthread_exit(NULL); 
    }
    pthread_exit(NULL); 
}

int main (int argc, char *argv[]) {
    sem_unlink(SEM_NAME); // just incase leftover sem from other run
    int players[] = {0, 1, 2, 3};
    logFile = fopen("logfile.txt", "w");
    seed = atoi(argv[1]);
    srand(seed);
    sem_rolling = sem_open(SEM_NAME, O_RDWR, 0777, 1); // init semaphore
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
    sem_close(sem_rolling);
    fclose(logFile);
    exit(EXIT_SUCCESS);
}