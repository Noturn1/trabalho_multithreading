#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define FILAS 5
#define COLUNAS 5

int assentos_ocupados = 0; // Variável global para contar assentos ocupados

void inicializar_assentos(char assentos[FILAS][COLUNAS]);
void mostrar_todos_assentos(char assentos[FILAS][COLUNAS]);
void alocar_lugar_sequencialmente(char assentos[FILAS][COLUNAS]);
void* alocar_lugar_paralelamente(void* arg);
void registrar_ocupante_assento(int fila, int coluna, int tid);

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

    fila = rand() % FILAS;
    coluna = rand() % COLUNAS;

    while( assentos[ fila ][ coluna ] == 'O' ){
        int fila = rand() % FILAS;
        int coluna = rand() % COLUNAS;
    }
    assentos[fila][coluna] = 'O'; // Ocupado
    assentos_ocupados++; // Incrementa o contador de assentos ocupados
    registrar_ocupante_assento(fila, coluna, pthread_self());
}


void* alocar_lugar_paralelamente(void* arg){

    if(assentos_ocupados >= FILAS * COLUNAS){
        printf("Todos os assentos estão ocupados!\n");
        pthread_exit(NULL);
    }                

    char (*assentos)[FILAS][COLUNAS] = arg;
    int fila = rand() % FILAS;
    int coluna = rand() % COLUNAS;
    while( (*assentos)[ fila ][ coluna ] == 'O' ){
        int fila = rand() % FILAS;
        int coluna = rand() % COLUNAS;
    }

    (*assentos)[ fila ][ coluna ] = 'O';
    assentos_ocupados++; // Incrementa o contador de assentos ocupados
    registrar_ocupante_assento(fila, coluna, pthread_self());
    pthread_exit(NULL);
}

int main(){
    int i,opcao;
    char assentos[FILAS][COLUNAS];
    pthread_t t1, t2, t3;

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
                if(pthread_create(&t1, NULL, alocar_lugar_paralelamente, (void*)&assentos)){
                    perror("Falha ao criar a thread 1");
                    return 1;
                }
                if(pthread_create(&t2, NULL, alocar_lugar_paralelamente, (void*)&assentos)){
                    perror("Falha ao criar a thread 2");
                    return 1;
                }
                if(pthread_create(&t3, NULL, alocar_lugar_paralelamente, (void*)&assentos)){
                    perror("Falha ao criar a thread 3");
                    return 1;
                }

                mostrar_todos_assentos(assentos);

                /*printf("\nPressione Enter para continuar...");
                while(getchar() != '\n');*/
                break;

        }
    } while(opcao != 5);


    
    return 0;
}
