#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_L 3
#define MAX_E 2

//estructura del monitor
typedef struct {
    int nlectores;
    int nesc_activos;
    int nesc_esperando;
    int dato;
    pthread_mutex_t mutex;
    pthread_cond_t cond_lectores;
    pthread_cond_t cond_escritores;
} monitor_t;

monitor_t monitor;

void monitor_init(monitor_t *m) {
    m->nlectores = 0;
    m->nesc_activos = 0;
    m->nesc_esperando = 0;
    m->dato = 0;
    pthread_mutex_init(&m->mutex, NULL);
    pthread_cond_init(&m->cond_lectores, NULL);
    pthread_cond_init(&m->cond_escritores, NULL);
}

// procedimientos del monitor
void comenzar_lectura(monitor_t *m) {
    pthread_mutex_lock(&m->mutex);

    while (m->nesc_activos || m->nesc_esperando>0) {
        pthread_cond_wait(&m->cond_lectores, &m->mutex);
    }

    m->nlectores++;
    pthread_mutex_unlock(&m->mutex);
}

void terminar_lectura(monitor_t *m) {
    pthread_mutex_lock(&m->mutex);
    m->nlectores--;

    if (m->nlectores == 0) {
        pthread_cond_signal(&m->cond_escritores);
    }
    pthread_mutex_unlock(&m->mutex);
}

void comenzar_escritura(monitor_t *m) {
    pthread_mutex_lock(&m->mutex);

    m->nesc_esperando++;
    while (m->nlectores > 0 || m->nesc_activos) {
        pthread_cond_wait(&m->cond_escritores, &m->mutex);
    }
    m->nesc_esperando--;

    m->nesc_activos = 1;
    pthread_mutex_unlock(&m->mutex);
}

void terminar_escritura(monitor_t *m) {
    pthread_mutex_lock(&m->mutex);
    m->nesc_activos = 0;

    pthread_cond_signal(&m->cond_escritores);
    pthread_cond_broadcast(&m->cond_lectores);

    pthread_mutex_unlock(&m->mutex);
}

void *lector(void *arg) {
    int id = (int )arg;

    for(int i=0; i<5; i++) {
        comenzar_lectura(&monitor);

        // LECTURA
        printf("Lector %d leyendo: %d\t", id, monitor.dato);
        printf("Escritores esperando: %d\n", monitor.nesc_esperando);
        usleep(rand() % 300000);

        terminar_lectura(&monitor);
        usleep(rand() % 300000);
    }
    pthread_exit(NULL);
}

void *escritor(void *arg) {
    int id = (int )arg;

    for(int i=0; i<5; i++) {
        comenzar_escritura(&monitor);

        // ESCRITURA
        monitor.dato++;
        printf("Escritor %d escribiendo: %d\t", id, monitor.dato);
        printf("Escritores esperando: %d\n", monitor.nesc_esperando);
        usleep(rand() % 300000);

        terminar_escritura(&monitor);
        usleep(rand() % 300000);
    }
    pthread_exit(NULL);
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
