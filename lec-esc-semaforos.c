 #include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_L  3
#define MAX_E  2

// Variable compartida (zona crítica)
int dato = 0;

// ------------------
// SEMÁFOROS
// ------------------

sem_t mutex;         // Protege nlectores (sección crítica de gestión de lectores)
sem_t m_escritores;  // Evita que entre un escritor si hay lectores o escritores dentro

// Contador del número de lectores que están leyendo simultáneamente
int nlectores = 0;


// ===============================================================
// =======================   HILO LECTOR   ========================
// ===============================================================

void *lector(void *arg) {
    int id = (int)arg;   // Identificador del lector

    while (1) {

        // -------------------------------------------------------
        // ENTRADA DE LECTOR (V)
        // -------------------------------------------------------

        //bloquea el acceso a nlectores para comprobar si 1 o 0
        sem_wait(&mutex);
        nlectores++;
        // deja en espera a los escritores porque hay un lector leyendo
        if (nlectores == 1) {
            sem_wait(&m_escritores);
        }
        //deja que los lectores continuen leyendo y devolviendo (SC)
        sem_post(&mutex);


        // -------------------------------------------------------
        // ZONA CRÍTICA (SC)
        // -------------------------------------------------------
        printf("Lector %d leyendo: %d\n", id, dato);
        usleep(rand() % 200000);


        // -------------------------------------------------------
        // SALIDA DE LECTOR (P)
        // -------------------------------------------------------

        // bloquea el acceso a nlectores para comprobar si 1 o 0
        sem_wait(&mutex);
        nlectores--;
        // desbloquea a los escritores, como no hay nadie leyendo deja escribir
        if (nlectores == 0) {
            sem_post(&m_escritores);
        }
        //deja que los lectores continuen leyendo
        sem_post(&mutex);

        usleep(rand() % 300000);
    }
}


// ===============================================================
// ======================   HILO ESCRITOR   ======================
// ===============================================================

void *escritor(void *arg) {
    int id = (int)arg;
    int aux;

    while (1) {

        // -------------------------------------------------------
        // ENTRADA DE ESCRITOR (V)
        // -------------------------------------------------------

        sem_wait(&m_escritores);


        // -------------------------------------------------------
        // ZONA CRÍTICA (SC)
        // -------------------------------------------------------

        aux = dato;
        usleep(rand() % 8000);
        aux++;
        usleep(rand() % 8000);
        dato = aux;

        printf("Escritor %d escribiendo: %d\n", id, dato);


        // -------------------------------------------------------
        // SALIDA DE ESCRITOR (P)
        // -------------------------------------------------------

        sem_post(&m_escritores);

        usleep(rand() % 200000);
    }
}


// ===============================================================
// =========================   MAIN   ============================
// ===============================================================

int main() {

    pthread_t lectores[MAX_L], escritores[MAX_E];  // Arrays de hilos
    pthread_attr_t atrib;
    int i;

    // Inicializamos el generador de números aleatorios
    srand(time(NULL));

    // Inicializamos los atributos por defecto
    pthread_attr_init(&atrib);

    // Inicializar los semáforos mutex = 1 → permite acceso exclusivo a nlectores
    sem_init(&mutex, 0, 1);

    // m_escritores = 1 → permite que entre un escritor y bloquea lectores si un escritor está dentro
    sem_init(&m_escritores, 0, 1);


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

    printf("Acaba el main\n");

    return 0;
}
