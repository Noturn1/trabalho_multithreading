#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

#define FILAS 50
#define COLUNAS 50
#define NUM_THREADS 30 // Número de threads para alocação paralela
#define NUM_ASSENTOS (FILAS * COLUNAS)

int assentos_ocupados = 0; // Variável global para contar assentos ocupados
// sem_t semaforo;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void inicializar_assentos(char assentos[FILAS][COLUNAS]);
void mostrar_todos_assentos(char assentos[FILAS][COLUNAS]);
void alocar_lugar_sequencialmente(char assentos[FILAS][COLUNAS]);
void* alocar_lugar_paralelamente(void* arg);
void registrar_ocupante_assento(int fila, int coluna, int tid);
void* alocar_lugar_paralelamente_sync(void* arg);

void inicializar_assentos(char assentos[FILAS][COLUNAS]){
    for (int i = 0; i < FILAS; i++){
        for(int j = 0; j < COLUNAS; j++){
            assentos[i][j] = 'L'; // Inicializar todos assentos como livres
        }
    }
}

void mostrar_todos_assentos(char assentos[FILAS][COLUNAS]){

    printf("--- MAPA DE ASSENTOS ---\n\n");
    printf("         ");

    // Imprimir o numero das colunas
    for (int j = 0; j < COLUNAS; j++){
        printf("%-3d ", j+1); // Espaco de 3 caracteres e alinha a esquerda (-)
    }
    printf("\n");

    for (int i = 0; i < FILAS; i++){
        printf("Fila %-2d ", i+1);
        for (int j = 0; j < COLUNAS; j++){
            printf("[%c] ", assentos[i][j]);
        }
        printf("\n");
    }
    printf("\nLegenda: [L] = Livre, [O] = Ocupado\n");
}

void registrar_ocupante_assento(int fila, int colunas, int tid){
    FILE *f; 
    f = fopen("ocupantes.txt", "a"); // Abre o arquivo em modo append
    if (f == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }
    fprintf(f, "Ocupante do assento [%d][%d]: Thread %d\n", fila, colunas, tid);
    fclose(f);


}

void alocar_lugar_sequencialmente(char assentos[FILAS][COLUNAS]){
    int fila, coluna;

    while(assentos_ocupados < NUM_ASSENTOS){
        fila = rand() % FILAS;
        coluna = rand() % COLUNAS;

        if(assentos[fila][coluna] == 'O') // Verifica se o assento está ocupado
            continue; // vai para a próxima iteração

        assentos[fila][coluna] = 'O'; // Ocupado
        assentos_ocupados++;
        registrar_ocupante_assento(fila, coluna, pthread_self());
    }
}


void* alocar_lugar_paralelamente(void* arg){

    char ( *assentos )[ FILAS ][ COLUNAS ] = arg;

    while(assentos_ocupados < NUM_ASSENTOS){
        int fila = rand() % FILAS;
        int coluna = rand() % COLUNAS;

        if( ( *assentos )[ fila ][ coluna ] == 'O' )
            continue; // Se o assento já está ocupado, tenta outro

        // Delay proposital para aumentar chance de corrida
        usleep(1000); // 1 milissegundo
        //Não testei sem, alguém testa pfv
        
        ( *assentos )[ fila ][ coluna ] = 'O';
        assentos_ocupados++;
        registrar_ocupante_assento(fila, coluna, pthread_self());
    }

    pthread_exit( 0 );
}

void* alocar_lugar_paralelamente_sync(void* arg) {
    char (*assentos)[FILAS][COLUNAS] = arg;

    int id = *(int*)arg;

    while (1) {
        //sem_wait(&semaforo); // Espera pelo semáforo

        pthread_mutex_lock(&mutex); // Entra na região crítica

        if (assentos_ocupados >= NUM_ASSENTOS) {
            pthread_mutex_unlock(&mutex);
            //sem_post(&semaforo);
            break;
        }

        int fila = rand() % FILAS;
        int coluna = rand() % COLUNAS;

        if ((*assentos)[fila][coluna] == 'O') {
            pthread_mutex_unlock(&mutex);
            //sem_post(&semaforo);
            continue;
        }

        (*assentos)[fila][coluna] = 'O';
        assentos_ocupados++;
        registrar_ocupante_assento(fila, coluna, pthread_self());

        pthread_mutex_unlock(&mutex);
        //sem_post(&semaforo);
    }

    pthread_exit(0);
}

int main(){
    int i,opcao;
    char assentos[ FILAS ][ COLUNAS ];
    pthread_t threads[ NUM_THREADS ];

    inicializar_assentos(assentos);
    do{

        printf("--- CINEMA ---\n\n");
        printf("1. Mostrar lugares disponiveis\n");
        printf("2. Reservar lugar (sequencial)\n");
        printf("3. Reservar lugar (multithreading s/ sincronizacao)\n");
        printf("4. Reservar lugar (multithreading c/ sincronizacao)\n");
        printf("5. Sair...\n");

        scanf("%d",&opcao);

        switch(opcao){
            case 1:
                mostrar_todos_assentos(assentos);

                /*printf("\nPressione Enter para continuar...");
                while(getchar() != '\n');
*/
                break;
            case 2:
                mostrar_todos_assentos(assentos);
                alocar_lugar_sequencialmente(assentos); 

                printf("\nPressione Enter para continuar...");
                while(getchar() != '\n');
                break;
            case 3:
                /*Criando as threads*/

                for(int i = 0; i < NUM_THREADS; i++){
                    if(pthread_create(&threads[i], NULL, alocar_lugar_paralelamente, (void*)&assentos)){
                        perror("Falha ao criar a thread");
                        return 1;
                    }
                }

                for(int i = 0; i < NUM_THREADS; i++){
                    if(pthread_join(threads[i], NULL) != 0){
                        perror("pthread_join falhou");
                        return 1;
                    }
                }
        

                mostrar_todos_assentos(assentos);

                /*printf("\nPressione Enter para continuar...");
                while(getchar() != '\n');*/
                break;
            case 4:
                // Inicializa o semáforo com valor 1 (binário)
                //sem_init(&semaforo, 0, 1);
                assentos_ocupados = 0;
                inicializar_assentos(assentos);

                for (int i = 0; i < NUM_THREADS; i++) {
                    if (pthread_create(&threads[i], NULL, alocar_lugar_paralelamente_sync, (void*)&assentos)) {
                        perror("Falha ao criar a thread");
                        return 1;
                    }
                }
                for (int i = 0; i < NUM_THREADS; i++) {
                    if (pthread_join(threads[i], NULL) != 0) {
                        perror("pthread_join falhou");
                        return 1;
                    }
                }
                //sem_destroy(&semaforo);
                mostrar_todos_assentos(assentos);
                break;

        }
    } while(opcao != 5);


    
    return 0;
}