#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define FILAS 10
#define COLUNAS 10

void inicializar_assentos(char assentos[FILAS][COLUNAS]);
void mostrar_todos_assentos(char assentos[FILAS][COLUNAS]);
void alocar_lugar_sequencialmente(char assentos[FILAS][COLUNAS]);
void* alocar_lugar_paralelamente(void* arg);

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

void alocar_lugar_sequencialmente(char assentos[FILAS][COLUNAS]){
    int fila, coluna;

    fila = rand() % FILAS;
    coluna = rand() % COLUNAS;
    assentos[fila-1][coluna-1] = 'O'; // Ocupado
}

/*void alocar_lugar_paralelamente(char assentos[FILAS][COLUNAS]){
    int fila, coluna;

    fila = rand() % FILAS;
    coluna = rand() % COLUNAS;
    assentos[fila-1][coluna-1] = 'O'; // Ocupado
}*/

void* alocar_lugar_paralelamente(void* arg){
    char (*assentos)[FILAS][COLUNAS] = arg;
    int fila = rand() % FILAS;
    int coluna = rand() % COLUNAS;
    (*assentos)[fila][coluna] = 'O'; // Ocupa a posição
    pthread_exit(NULL);
}



int main(){
    int i,opcao;
    char assentos[FILAS][COLUNAS];
    pthread_t t1, t2, t3;
    int tid1 = 1, tid2 = 2, tid3 = 3;


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

                printf("\nPressione Enter para continuar...");
                while(getchar() != '\n');

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

                /*Esperar todas as thread terminarem*/
                if(pthread_join(t1, NULL)){
                    perror("Falha ao esperar a thread 1");
                    return 1;
                }
                if(pthread_join(t2, NULL)){
                    perror("Falha ao esperar a thread 1");
                    return 1;
                }
                if(pthread_join(t3, NULL)){
                    perror("Falha ao esperar a thread 1");
                    return 1;
                }

                printf("\nPressione Enter para continuar...");
                while(getchar() != '\n');
                break;

        }
    } while(opcao != 5);


    
    return 0;
}
