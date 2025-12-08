#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_L 3
#define MAX_E 2

//estructura del monitor
typedef struct {
    int nlectores;
    int escribiendo;
    int dato;
    pthread_mutex_t mutex;
    pthread_cond_t cond_lectores;
    pthread_cond_t cond_escritores;
} monitor_t;

// inicialización del monitor
void monitor_init(monitor_t *m) {
    m->nlectores = 0;
    m->escribiendo = 0;
    m->dato = 0;
    pthread_mutex_init(&m->mutex, NULL);
    pthread_cond_init(&m->cond_lectores, NULL);
    pthread_cond_init(&m->cond_escritores, NULL);
}

// procedimientos del monitor
void entrada_lector(monitor_t *m) {
    pthread_mutex_lock(&m->mutex);

    while (m->escribiendo) {
        pthread_cond_wait(&m->cond_lectores, &m->mutex);
    }

    m->nlectores++;
    pthread_mutex_unlock(&m->mutex);
}

void salida_lector(monitor_t *m) {
    pthread_mutex_lock(&m->mutex);
    m->nlectores--;

    if (m->nlectores == 0) {
        pthread_cond_signal(&m->cond_escritores);
    }
    pthread_mutex_unlock(&m->mutex);
}

void entrada_escritor(monitor_t *m) {
    pthread_mutex_lock(&m->mutex);

    while (m->nlectores > 0 || m->escribiendo) {
        pthread_cond_wait(&m->cond_escritores, &m->mutex);
    }

    m->escribiendo = 1;
    pthread_mutex_unlock(&m->mutex);
}

void salida_escritor(monitor_t *m) {
    pthread_mutex_lock(&m->mutex);
    m->escribiendo = 0;

    pthread_cond_signal(&m->cond_escritores);
    pthread_cond_broadcast(&m->cond_lectores);

    pthread_mutex_unlock(&m->mutex);
}


monitor_t monitor;

void *lector(void *arg) {
    int id = (int )arg;

    while (1) {
        entrada_lector(&monitor);

        // LECTURA
        printf("Lector %d leyendo: %d\n", id, monitor.dato);
        usleep(rand() % 300000);

        salida_lector(&monitor);
        usleep(rand() % 300000);
    }
}

void *escritor(void *arg) {
    int id = (int )arg;

    while (1) {
        entrada_escritor(&monitor);

        // ESCRITURA
        monitor.dato++;
        printf("Escritor %d escribiendo: %d\n", id, monitor.dato);
        usleep(rand() % 300000);

        salida_escritor(&monitor);
        usleep(rand() % 300000);
    }
}

int main() {
    pthread_t lectores[MAX_L], escritores[MAX_E];
    int i;
    pthread_attr_t atrib;

    srand(time(NULL));
    pthread_attr_init(&atrib);
    monitor_init(&monitor);

    for (i = 0; i < MAX_L; i++) {
        pthread_create(&lectores[i], &atrib, lector, (void *)i);
    }

    for (i = 0; i < MAX_E; i++) {
        pthread_create(&escritores[i], &atrib, escritor, (void *)i);
    }

    for (i = 0; i < MAX_L; i++) {
        pthread_join(lectores[i], NULL);
    }

    for (i = 0; i < MAX_E; i++) {
        pthread_join(escritores[i], NULL);
    }

    return 0;
}
