/*
    * Projeto: Gerenciamento de Assentos de Cinema
    * Descrição: Simula a alocação de assentos em um cinema utilizando threads e semáforos.
    * Autores: Arthur Angelo, Kauan Campos, Matheus Lopes
    * Data: 02/07/2025

    * Compilação: gcc -o cinema multithreading.c
    * testar performance: perf stat ./cinema
    * Testar com Valgrind: valgrind --leak-check=full --track-origins=yes ./cinema
    * Execução: ./cinema
*/


#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/time.h>
#include <string.h>

#define FILAS 200
#define COLUNAS 200
#define NUM_THREADS 30
#define NUM_ASSENTOS (FILAS * COLUNAS)

int assentos_ocupados = 0;
int n_registros = 0; // Contador de registros
sem_t semid;

// Funções
void inicializar_assentos(char assentos[FILAS][COLUNAS]);
void mostrar_todos_assentos(char assentos[FILAS][COLUNAS]);
void alocar_lugar_sequencialmente(char assentos[FILAS][COLUNAS]);
void* alocar_lugar_paralelamente(void* arg);
void* alocar_lugar_paralelamente_sync(void* arg);
void checarMetricas(char assentos[FILAS][COLUNAS], const char* metodo, double tempo);
int contar_ocupados(char assentos[FILAS][COLUNAS]);

// Thread handler
pthread_t threads[NUM_THREADS];

// Utilizado para registrar assentos no arquivo
void registrar_ocupante_assento(int fila, int colunas, pthread_t tid){

    FILE *f = fopen("ocupantes.txt", "a");
    if (f == NULL) return;
    fprintf(f, "Ocupante do assento [%d][%d]: Thread %lu\n", fila, colunas, tid);
    fclose(f);

    // Incrementa o contador de registros
    n_registros++;
}

// Inicializa todos os assentos
void inicializar_assentos(char assentos[FILAS][COLUNAS]){
    for (int i = 0; i < FILAS; i++)
        for(int j = 0; j < COLUNAS; j++)
            assentos[i][j] = 'L';
    assentos_ocupados = 0;
}

// Mostra todos os assentos
void mostrar_todos_assentos(char assentos[FILAS][COLUNAS]){
    printf("\n--- MAPA DE ASSENTOS ---\n\n");
    printf("         ");
    for (int j = 0; j < COLUNAS; j++)
        printf("%-3d ", j+1);
    printf("\n");
    for (int i = 0; i < FILAS; i++){
        printf("Fila %-2d ", i+1);
        for (int j = 0; j < COLUNAS; j++)
            printf("[%c] ", assentos[i][j]);
        printf("\n");
    }
    printf("\nLegenda: [L] = Livre, [O] = Ocupado\n");
}

// Alocação sequencial
void alocar_lugar_sequencialmente(char assentos[FILAS][COLUNAS]){
    while(assentos_ocupados < NUM_ASSENTOS){
        int fila = rand() % FILAS;
        int coluna = rand() % COLUNAS;
        if(assentos[fila][coluna] == 'O') continue;
        assentos[fila][coluna] = 'O';
        assentos_ocupados++;
        registrar_ocupante_assento(fila, coluna, pthread_self());
    }
}

// Alocação paralela sem sincronização

void* alocar_lugar_paralelamente(void* arg){
    char (*assentos)[FILAS][COLUNAS] = arg;
    int tentativas = 0;
    const int MAX_TENTATIVAS = NUM_ASSENTOS * 10;

    while(assentos_ocupados < NUM_ASSENTOS && tentativas < MAX_TENTATIVAS) {
        int fila = rand() % FILAS;
        int coluna = rand() % COLUNAS;
        tentativas++;

        if((*assentos)[fila][coluna] == 'O') continue;
        
        // Adiciona delay para forçar condição de corrida
        usleep(100);
        
        (*assentos)[fila][coluna] = 'O';
        assentos_ocupados++;
        registrar_ocupante_assento(fila, coluna, pthread_self());
    }
    pthread_exit(0);
}

// Alocação paralela com sincronização
void* alocar_lugar_paralelamente_sync(void* arg){
    char (*assentos)[FILAS][COLUNAS] = arg;
    while(1){
        int fila = rand() % FILAS;
        int coluna = rand() % COLUNAS;

        sem_wait(&semid);
        if (assentos_ocupados >= NUM_ASSENTOS){
            sem_post(&semid);
            break;
        }
        if((*assentos)[fila][coluna] == 'L'){
            (*assentos)[fila][coluna] = 'O';
            assentos_ocupados++;
            registrar_ocupante_assento(fila, coluna, pthread_self());
        }
        sem_post(&semid);
    }
    pthread_exit(0);
}

// Verifica a integridade e escreve métricas
void checarMetricas(char assentos[FILAS][COLUNAS], const char* metodo, double tempo){

    double throughput = NUM_ASSENTOS / tempo;
    int ocupados_verificados = contar_ocupados(assentos);

    printf("\nMétricas - %s:\n", metodo);
    printf("Tempo de execução: %.6f segundos\n", tempo);
    printf("Vazão (assentos/s): %.2f\n", throughput);
    printf("Assentos ocupados: %d\n", assentos_ocupados);
    if (n_registros != NUM_ASSENTOS || ocupados_verificados != NUM_ASSENTOS)
        printf("Inconsistência detectada nos dados!\n");
    else
        printf("Integridade verificada com sucesso\n");
}


int contar_ocupados(char assentos[FILAS][COLUNAS]){
    int ocupados = 0;
    for (int i = 0; i < FILAS; i++)
        for (int j = 0; j < COLUNAS; j++)
            if (assentos[i][j] == 'O')
                ocupados++;
    return ocupados;
}


// Main
int main(){
    int opcao;
    char assentos[FILAS][COLUNAS];

    inicializar_assentos(assentos);
    

    do {
        printf("\n--- CINEMA ---\n\n");
        printf("1. Mostrar lugares disponiveis\n");
        printf("2. Reservar lugar (sequencial)\n");
        printf("3. Reservar lugar (multithreading sem sync)\n");
        printf("4. Reservar lugar (multithreading com sync)\n");
        printf("5. Sair...\n");

        scanf("%d", &opcao);
        getchar(); // limpa o buffer
        remove("ocupantes.txt");

        struct timeval inicio, fim;
        double tempo_exec;

        switch(opcao){
            case 1:
                mostrar_todos_assentos(assentos);
                break;

            case 2:
                inicializar_assentos(assentos);
                gettimeofday(&inicio, NULL);
                alocar_lugar_sequencialmente(assentos);
                gettimeofday(&fim, NULL);
                tempo_exec = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1e6;
                checarMetricas(assentos, "Sequencial", tempo_exec);
                break;

            case 3:
                inicializar_assentos(assentos);
                gettimeofday(&inicio, NULL);
                for (int i = 0; i < NUM_THREADS; i++)
                    pthread_create(&threads[i], NULL, alocar_lugar_paralelamente, &assentos);
                for (int i = 0; i < NUM_THREADS; i++)
                    pthread_join(threads[i], NULL);
                gettimeofday(&fim, NULL);
                tempo_exec = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1e6;
                checarMetricas(assentos, "Multithread sem sync", tempo_exec);
                break;

            case 4:
                inicializar_assentos(assentos);
                sem_init(&semid, 0, 1);
                gettimeofday(&inicio, NULL);
                for (int i = 0; i < NUM_THREADS; i++)
                    pthread_create(&threads[i], NULL, alocar_lugar_paralelamente_sync, &assentos);
                for (int i = 0; i < NUM_THREADS; i++)
                    pthread_join(threads[i], NULL);
                gettimeofday(&fim, NULL);
                tempo_exec = (fim.tv_sec - inicio.tv_sec) + (fim.tv_usec - inicio.tv_usec) / 1e6;
                checarMetricas(assentos, "Multithread com sync", tempo_exec);
                break;
        }

        assentos_ocupados = 0; // Reseta contador de assentos ocupados
        n_registros = 0; // Reseta contador de registros
        sem_destroy(&semid); // Destrói o semáforo


    } while(opcao != 5);

    return 0;
}
