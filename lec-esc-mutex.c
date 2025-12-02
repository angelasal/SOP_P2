
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_L 3
#define MAX_E 2

int dato = 0;

// Variables de sincronización
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_lectores = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_escritores = PTHREAD_COND_INITIALIZER;

// Variables de estado
int nlectores = 0;              // Número de lectores leyendo
int escribiendo = 0;            // Si hay un escritor escribiendo
int escritores_esperando = 0;   // Escritores esperando (prioridad)

void *lector(void *arg) {
    int id = (int)arg;

    while (1) {
        // ENTRADA SECCIÓN CRÍTICA
        pthread_mutex_lock(&mutex);

        // Si hay escritor escribiendo o esperando, el lector espera
        while (escribiendo || escritores_esperando > 0) {
            pthread_cond_wait(&cond_lectores, &mutex);
        }

        nlectores++; // Entra a leer
        pthread_mutex_unlock(&mutex);

        // LECTURA
        printf("Lector %d leyendo: %d\n", id, dato);
        usleep(rand() % 1000000);

        // SALIDA SECCIÓN CRÍTICA
        pthread_mutex_lock(&mutex);
        nlectores--;
        if (nlectores == 0) {
            // Si no quedan lectores, avisamos a escritores
            pthread_cond_signal(&cond_escritores);
        }
        pthread_mutex_unlock(&mutex);
    }
}

void *escritor(void *arg) {
    int id = (int)arg;

    while (1) {
        // ENTRADA SECCIÓN CRÍTICA
        pthread_mutex_lock(&mutex);
        escritores_esperando++;

        // Espera mientras haya lectores o un escritor escribiendo
        while (nlectores > 0 || escribiendo) {
            pthread_cond_wait(&cond_escritores, &mutex);
        }

        escritores_esperando--;
        escribiendo = 1; // Marca que está escribiendo
        pthread_mutex_unlock(&mutex);

        // ESCRITURA
        dato++;
        printf("Escritor %d escribiendo: %d\n", id, dato);
        usleep(rand() % 2000000);

        // SALIDA SECCIÓN CRÍTICA
        pthread_mutex_lock(&mutex);
        escribiendo = 0;

        // Avisamos a otros escritores y a todos los lectores
        pthread_cond_signal(&cond_escritores);
        pthread_cond_broadcast(&cond_lectores);

        pthread_mutex_unlock(&mutex);
    }
}

int main() {
    pthread_t lectores[MAX_L], escritores[MAX_E];
    pthread_attr_t atrib;
    int i;

    srand(time(NULL));
    pthread_attr_init(&atrib);

    // Crear lectores
    for (i = 0; i < MAX_L; i++) {
        pthread_create(&lectores[i], &atrib, lector, (void *)i);
    }

    // Crear escritores
    for (i = 0; i < MAX_E; i++) {
        pthread_create(&escritores[i], &atrib, escritor, (void *)i);
    }

    // Esperar a que terminen (en este caso nunca terminan)
    for (i = 0; i < MAX_L; i++) {
        pthread_join(lectores[i], NULL);
    }
    for (i = 0; i < MAX_E; i++) {
        pthread_join(escritores[i], NULL);
    }

    printf("Acaba el main\n");
    return 0;
}
