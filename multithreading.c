#include <stdio.h>
#include <stdlib.h>

#define FILAS 10
#define COLUNAS 10

void inicializar_assentos(char assentos[FILAS][COLUNAS]);
void mostrar_todos_assentos(char assentos[FILAS][COLUNAS]);

int main(){
    int i,opcao;
    char assentos[FILAS][COLUNAS];


    do{

        printf("--- CINEMA ---\n\n");
        printf("1. Mostrar lugares disponiveis\n");
        printf("2. Reservar lugar (sequencial)\n");
        printf("3. Reservar lugar (multithreading s/ sincronizacao)\n");
        printf("4. Reservar lugar (multithreading c/ sincronizacao)\n");
        printf("5. Sair...\n");

        scanf("%d",opcao);

        switch(opcao){
            case 1:
                mostrar_todos_assentos(assentos);
                break;
            case 2:
                alocar_lugar_sequencialmente(); // FAZER
                break;
            case 3:
                alocar_lugar_multithreading_sem_sync(); // FAZER
                break;
            case 4:
                alocar_lugar_multithreading_com_sync(); // FAZER
                break;
        }
    } while(opcao != 5);

    return 0;
}

void inicializar_assentos(){
    for (int i = 0; i < FILAS; i++){
        for(int j = 0; j < COLUNAS; j++){
            assentos[i][j] = 'L'; // Inicializar todos assentos como livres
        }
    }
}

void mostrar_todos_assentos(char assentos[FILAS][COLUNAS]){

    printf("--- MAPA DE ASSENTOS ---\n\n");

    // Imprimir o numero das colunas
    for (int j = 0; j < COLUNAS){
        printf("%-3d", j+1); // Espaco de 3 caracteres e alinha a esquerda (-)
    }

    for (int i = 0; i < FILAS; i++){
        printf("Fila %-2d ", i+1)
        for (j = 0; j < COLUNAS; j++){
            printf("[%c] ", assentos[i][j]);
        }
        printf("\n");
    }
    printf("\nLegenda: [L] = Livre, [O] = Ocupado\n");
}

