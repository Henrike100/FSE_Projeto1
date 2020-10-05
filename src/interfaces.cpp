#include "interfaces.hpp"

int menu_inicializacao() {
    int opt = -1;

    printf("Como deseja definir a temperatura?\n\n");

    printf("1. Usando o teclado\n");
    printf("2. Potenciômetro\n");

    printf("0. Sair\n\n");

    printf("Escolha: ");

    scanf(" %d", &opt);

    while(opt < 0 || opt > 2) {
        printf("Escolha deve estar entre 0 e 2: ");
        scanf(" %d", &opt);
    }

    return opt;
}

float pegar_temperatura(float *TE) {
    float temperatura = -1.0f;

    printf("Defina a temperatura: ");
    scanf(" %f", &temperatura);

    while(temperatura < *TE) {
        printf("A menor temperatura possível é %.1f. Escolha outro valor: ", *TE);
        scanf(" %f", &temperatura);
    }

    return temperatura;
}

float pegar_histerese() {
    float histerese = -1.0f;

    printf("Defina a Histerese: ");
    scanf(" %f", &histerese);

    while(histerese <= 0) {
        printf("A histerese deve ser maior que 0. Escolha outro valor: ");
        scanf(" %f", &histerese);
    }

    return histerese;
}
