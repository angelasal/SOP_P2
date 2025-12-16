#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_L  3
#define MAX_E  2


int dato = 0;

void *lector(void *arg) {
    int id = (int )arg;
    while (1) {
        printf("Lector %d leyendo: %d\n", id, dato);
        usleep(rand() % 400000);
    }
}

void *escritor(void *arg) {
    int id = (int )arg;
    int aux;
    while (1) {
        aux=dato;
        usleep(rand() %8000);
        aux++;
        usleep(rand() %8000);
        dato=aux;
        printf("Escritor %d escribiendo: %d\n", id, dato);

        usleep(rand() % 200000);
    }
}

int main() { 
    pthread_t lectores[MAX_L], escritores[MAX_E]; 
    int i;
    pthread_attr_t atrib;

    srand(time(NULL)); 

    pthread_attr_init(&atrib);

   for(i=0; i<MAX_L; i++) {
        pthread_create(&lectores[i], &atrib, lector, (void *)i);
    }

    for(i=0; i<MAX_E; i++) {
        pthread_create(&escritores[i], &atrib, escritor, (void *)i);
    }

    for(i=0; i<MAX_L; i++) {
        pthread_join(lectores[i], NULL);
    } 

    for(i=0; i<MAX_E; i++) {
        pthread_join(escritores[i], NULL);
    }

    printf("Acaba el main\n");
    return 0;
}

/*Explicación:
Inicializar atributos con pthread_attr_init
Crear hilos lectores y escritores con pthread_create
Esperar su finalización con pthread_join */
