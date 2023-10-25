#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <getopt.h>
#include <math.h>

typedef struct {
    int atomID;
    char atomTYPE;
} Atom;

int M = 60;
int g = 100;
int atom_count = 0;
// Counter for molecule composition order
int molecule_order = 0;
// Semaphore declarations
sem_t C, N, S, Th, O, CO2, NO2, SO2, ThO2;
sem_t synch_C, synch_N, synch_S, synch_Th, synch_O;
sem_t synch_order;


void *Produce_C(void *arg);
void *Produce_N(void *arg);
void *Produce_S(void *arg);
void *Produce_Th(void *arg);
void *Produce_O(void *arg);
void *Composer_CO2(void *arg);
void *Composer_NO2(void *arg);
void *Composer_SO2(void *arg);
void *Composer_THO2(void *arg);
double rand_exponential(double lambda);

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "m:g:")) != -1) {
        switch (opt) {
            case 'm':
                M = atoi(optarg);
                break;
            case 'g':
                g = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-m total_atoms] [-g g]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (M % 6 != 0) {
        fprintf(stderr, "Error: The total number of atoms (M) must be a multiple of 6.\n");
        exit(EXIT_FAILURE);
    }

    sem_init(&C, 0, 0);
    sem_init(&N, 0, 0);
    sem_init(&S, 0, 0);
    sem_init(&Th, 0, 0);
    sem_init(&O, 0, 0);
    sem_init(&CO2, 0, 0);
    sem_init(&NO2, 0, 0);
    sem_init(&SO2, 0, 0);
    sem_init(&ThO2, 0, 0);
    sem_init(&synch_C, 0, 1);
    sem_init(&synch_N, 0, 1);
    sem_init(&synch_S, 0, 1);
    sem_init(&synch_Th, 0, 1);
    sem_init(&synch_O, 0, 1);
    sem_init(&synch_order, 0, 1);
    pthread_t producer_threads[5];
    pthread_t composer_threads[4];

    pthread_create(&producer_threads[0], NULL, Produce_C, NULL);
    pthread_create(&producer_threads[1], NULL, Produce_N, NULL);
    pthread_create(&producer_threads[2], NULL, Produce_S, NULL);
    pthread_create(&producer_threads[3], NULL, Produce_Th, NULL);
    pthread_create(&producer_threads[4], NULL, Produce_O, NULL);

    pthread_create(&composer_threads[0], NULL, Composer_CO2, NULL);
    pthread_create(&composer_threads[1], NULL, Composer_NO2, NULL);
    pthread_create(&composer_threads[2], NULL, Composer_SO2, NULL);
    pthread_create(&composer_threads[3], NULL, Composer_THO2, NULL);

    for (int i = 0; i < 5; i++) {
        pthread_join(producer_threads[i], NULL);
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(composer_threads[i], NULL);
    }

    sem_destroy(&C);
    sem_destroy(&N);
    sem_destroy(&S);
    sem_destroy(&Th);
    sem_destroy(&O);
    sem_destroy(&CO2);
    sem_destroy(&NO2);
    sem_destroy(&SO2);
    sem_destroy(&ThO2);
    sem_destroy(&synch_C);
    sem_destroy(&synch_N);
    sem_destroy(&synch_S);
    sem_destroy(&synch_Th);
    sem_destroy(&synch_O);
     sem_destroy(&synch_order);
    return 0;
}

void create_atom(Atom *atom, char type) {
    atom->atomTYPE = type;
    atom->atomID = ++atom_count;
    printf("%c with ID: %d is created.\n", atom->atomTYPE, atom->atomID);
}

void *Produce_C(void *arg) {
    int atoms_to_produce = M / 6;
    for (int i = 0; i < atoms_to_produce; i++) {
        sem_wait(&synch_C);
        Atom atom;
        create_atom(&atom, 'C');
        sem_post(&C);
        sem_post(&synch_C);
        usleep(rand_exponential(g) * 1000);
    }
    return NULL;
}

void *Produce_N(void *arg) {
    int atoms_to_produce = M / 6;
    for (int i = 0; i < atoms_to_produce; i++) {
        sem_wait(&synch_N);
        Atom atom;
        create_atom(&atom, 'N');
        sem_post(&N);
        sem_post(&synch_N);
        usleep(rand_exponential(g) * 1000);
    }
    return NULL;
}

void *Produce_S(void *arg) {
    int atoms_to_produce = M / 6;
    for (int i = 0; i < atoms_to_produce; i++) {
        sem_wait(&synch_S);
        Atom atom;
        create_atom(&atom, 'S');
        sem_post(&S);
        sem_post(&synch_S);
        usleep(rand_exponential(g) * 1000);
    }
    return NULL;
}

void *Produce_Th(void *arg) {
    int atoms_to_produce = M / 6;
    for (int i = 0; i < atoms_to_produce; i++) {
        sem_wait(&synch_Th);
        Atom atom;
        create_atom(&atom, 'T');
        sem_post(&Th);
        sem_post(&synch_Th);
        usleep(rand_exponential(g) * 1000);
    }
    return NULL;
}

void *Produce_O(void *arg) {
    int atoms_to_produce = M / 3;
    for (int i = 0; i < atoms_to_produce; i++) {
        sem_wait(&synch_O);
        Atom atom;
        create_atom(&atom, 'O');
        sem_post(&O);
        sem_post(&synch_O);
        usleep(rand_exponential(g) * 1000);
    }
    return NULL;
}

void *Composer_CO2(void *arg) {
    while (1) {
        sem_wait(&C);
        sem_wait(&O);
        sem_wait(&O);

        sem_wait(&synch_order);
        if (molecule_order % 5 == 0 || molecule_order % 5 == 2) {
            printf("CO2 is composed.\n");
            molecule_order++;
            sem_post(&synch_order);
        } else {
            sem_post(&synch_order);
            sem_post(&C);
            sem_post(&O);
            sem_post(&O);
        }
    }
    return NULL;
}

void *Composer_NO2(void *arg) {
    while (1) {
        sem_wait(&N);
        sem_wait(&O);
        sem_wait(&O);

        sem_wait(&synch_order);
        if (molecule_order % 5 == 1) {
            printf("NO2 is composed.\n");
            molecule_order++;
            sem_post(&synch_order);
        } else {
            sem_post(&synch_order);
            sem_post(&N);
            sem_post(&O);
            sem_post(&O);
        }
    }
    return NULL;
}

void *Composer_SO2(void *arg) {
    while (1) {
        sem_wait(&S);
        sem_wait(&O);
        sem_wait(&O);

        sem_wait(&synch_order);
        if (molecule_order % 5 == 3) {
            printf("SO2 is composed.\n");
            molecule_order++;
            sem_post(&synch_order);
        } else {
            sem_post(&synch_order);
            sem_post(&S);
            sem_post(&O);
            sem_post(&O);
        }
    }
    return NULL;
}

void *Composer_THO2(void *arg) {
    while (1) {
        sem_wait(&Th);
        sem_wait(&O);
        sem_wait(&O);

        sem_wait(&synch_order);
        if (molecule_order % 5 == 4) {
            printf("ThO2 is composed.\n");
            molecule_order++;
            sem_post(&synch_order);
        } else {
            sem_post(&synch_order);
            sem_post(&Th);
            sem_post(&O);
            sem_post(&O);
        }
    }
    return NULL;
}

double rand_exponential(double lambda) {
    double x = (double)rand() / RAND_MAX;
    return -1 / lambda * log(1 - x);
}

