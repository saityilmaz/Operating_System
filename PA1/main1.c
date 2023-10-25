#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>

typedef struct {
    int atomID;
    char atomTYPE;
} Atom;

int C_count = 0, N_count = 0, S_count = 0, Th_count = 0, O_count = 0;
int molecule_order = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condCO2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t condNO2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t condSO2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t condTHO2 = PTHREAD_COND_INITIALIZER;

Atom *atoms;


void atom_gen(char atomTYPE, int atomID);

void *Composer_CO2(void *arg);
void *Composer_NO2(void *arg);
void *Composer_SO2(void *arg);
void *Composer_THO2(void *arg);

int main(int argc, char *argv[]) {
    int MC = 20, MN = 20, MS = 20, MTH = 20, MO = 20;
    double generation_rate = 100;
     int opt;

    while ((opt = getopt(argc, argv, "c:n:s:t:o:g:")) != -1) {
        switch (opt) {
            case 'c':
                MC = atoi(optarg);
                break;
            case 'n':
                MN = atoi(optarg);
                break;
            case 's':
                MS = atoi(optarg);
                break;
            case 't':
                MTH = atoi(optarg);
                break;
            case 'o':
                MO = atoi(optarg);
                break;
            case 'g':
                generation_rate = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-c C_atoms] [-n N_atoms] [-s S_atoms] [-t Th_atoms] [-o O_atoms] [-g generation_rate]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    atoms = calloc(MC + MN + MS + MTH + MO, sizeof(Atom));


    pthread_t composer_threads[4];
    pthread_create(&composer_threads[0], NULL, Composer_CO2, NULL);
    pthread_create(&composer_threads[1], NULL, Composer_NO2, NULL);
    pthread_create(&composer_threads[2], NULL, Composer_SO2, NULL);
    pthread_create(&composer_threads[3], NULL, Composer_THO2, NULL);

    int atomID = 0;
    for (int i = 0; i < MC + MN + MS + MTH + MO; ++i) {
        double x = (double)rand() / (double)RAND_MAX;
        double sleep_time = -1 / generation_rate * log(1 - x);
        usleep(sleep_time * 1000000);

        char atomTYPE;
        int rand_atom = rand() % (MC + MN + MS + MTH + MO);
        if (rand_atom < MC) {
            atomTYPE = 'C';
        } else if (rand_atom < MC + MN) {
            atomTYPE = 'N';
        } else if (rand_atom < MC + MN + MS) {
            atomTYPE = 'S';
        } else if (rand_atom < MC + MN + MS + MTH) {
            atomTYPE = 'T';
        } else {
            atomTYPE = 'O';
        }

        printf("%c with ID: %d is created.\n", atomTYPE, atomID);
        atom_gen(atomTYPE, atomID);
        if(C_count>2){
            printf("%c with ID: %d is wasted.\n",atomTYPE, atomID);
            C_count--;
        }
        if(N_count>1){
            printf("%c with ID: %d is wasted.\n", atomTYPE, atomID);
            N_count--;
        }
        if(S_count>1){
            printf("%c with ID: %d is wasted.\n", atomTYPE, atomID);
            S_count--;
        }
        if(Th_count>1){
            printf("%c with ID: %d is wasted.\n", atomTYPE, atomID);
            Th_count--;
        }
        if(O_count>10){
            printf("%c with ID: %d is wasted.\n", atomTYPE, atomID);
            O_count--;
        }
        atomID++;
    }



    free(atoms);

    return 0;
}

void atom_gen(char atomTYPE, int atomID) {
    pthread_mutex_lock(&mutex);
    Atom atom;
    atom.atomTYPE = atomTYPE;
    atom.atomID = atomID;
    atoms[atomID] = atom;


    switch (atomTYPE) {
        case 'C':
            C_count++;
            pthread_cond_signal(&condCO2);
            break;
        case 'N':
            N_count++;
            pthread_cond_signal(&condNO2);
            break;
        case 'S':
            S_count++;
            pthread_cond_signal(&condSO2);
            break;
        case 'T':
            Th_count++;
            pthread_cond_signal(&condTHO2);
            break;
        case 'O':
            O_count++;
            pthread_cond_signal(&condCO2);
            pthread_cond_signal(&condNO2);
            pthread_cond_signal(&condSO2);
            pthread_cond_signal(&condTHO2);
            break;
    }

    pthread_mutex_unlock(&mutex);
}

void *Composer_CO2(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (C_count < 1 || O_count < 2 || ((molecule_order % 5 != 0)&(molecule_order % 5 != 2))) {
            pthread_cond_wait(&condCO2, &mutex);
        }

        C_count--;
        O_count -= 2;
        printf("Composed molecule: CO2\n");

        // Update the molecule_order counter
        if(molecule_order % 5 != 0) pthread_cond_signal(&condNO2);
        else if(molecule_order % 5 != 2) pthread_cond_signal(&condSO2);
        molecule_order++;
        pthread_mutex_unlock(&mutex);
    }
}

void *Composer_NO2(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (N_count < 1 || O_count < 2 || molecule_order % 5 != 1) {
            pthread_cond_wait(&condNO2, &mutex);
        }

        N_count--;
        O_count -= 2;
        printf("Composed molecule: NO2\n");

        // Update the molecule_order counter
        molecule_order++;
        pthread_cond_signal(&condSO2);
        pthread_mutex_unlock(&mutex);
    }
}

void *Composer_SO2(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (S_count < 1 || O_count < 2 || molecule_order % 5 != 3) {
            pthread_cond_wait(&condSO2, &mutex);
        }

        S_count--;
        O_count -= 2;
        printf("Composed molecule: SO2\n");

        // Update the molecule_order counter
        molecule_order++;
        pthread_cond_signal(&condTHO2);
        pthread_mutex_unlock(&mutex);
    }
}

void *Composer_THO2(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (Th_count < 1 || O_count < 2 || molecule_order % 5 != 4) {
            pthread_cond_wait(&condTHO2, &mutex);
        }

        Th_count--;
        O_count -= 2;
        printf("Composed molecule: ThO2\n");

        // Update the molecule_order counter
        molecule_order++;
        pthread_cond_signal(&condCO2);
        pthread_mutex_unlock(&mutex);
    }
}

