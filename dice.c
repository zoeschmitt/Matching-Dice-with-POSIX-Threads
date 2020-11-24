#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

#define NUM_PLAYERS 4

FILE* logFile;
int seed = 0;
int sums[NUM_PLAYERS];

pthread_cond_t winner;
pthread_mutex_t rolling;


void log(int player, int rollOne, int rollTwo) {
    char playerL;

    switch (player) {
    case 0:
        playerL = "A";
        break;
    case 1:
        playerL = "B";
        break;
    case 2:
        playerL = "C";
        break;
    case 3:
        playerL = "D";
        break;
    default:
        break;
    }

    fprintf(logFile, "Player %c: ", playerL);
    fprintf(logFile, "gets %d and %d with a sum of", rollOne, rollTwo);
    fprintf(logFile, "%d\n", rollOne + rollTwo);
}

void finalLog(char player1, char player2) {
    fprintf(logFile, "DEALER: The winning team is %c and %c\n", player1, player2);
}

//checks if anyone has won
void checkStatus() {
    if (sums[0] == sums[2]) {
        finalLog("A", "C");
    } else if (sums[1] == sums[3]) {
        finalLog("B", "D");
    }
}

void* dealer(void* arg) {
//rand() % 4; //gives random number between 0 and 3
}

void* player(void* arg) {
//sum[player] = roll1 + roll2;
//write results
//check status
}

int main (int argc, char *argv[]) {

    logFile = fopen("logfile.txt", "a");
    seed = atoi(argv[1]);
    srand(time(NULL));

    pthread_t dealerThread;
    pthread_t playerA;
    pthread_t playerB;
    pthread_t playerC;
    pthread_t playerD;

    pthread_create(&dealerThread, NULL, dealer, NULL);
    pthread_create(&playerA, NULL, dealer, NULL);
    pthread_create(&playerB, NULL, dealer, NULL);
    pthread_create(&playerC, NULL, dealer, NULL);
    pthread_create(&playerD, NULL, dealer, NULL);

    fclose(logFile);
    pthread_exit(NULL);
    return 0;
}