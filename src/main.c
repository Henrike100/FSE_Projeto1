#include <stdio.h>

int main(int argc, const char * argv[]) {
    int opt = -1;

    printf("MENU\n\n");

    printf("1. Solicitar Inteiro\n");
    printf("2. Solicitar Float\n");
    printf("3. Solicitar String\n");
    printf("4. Enviar Inteiro\n");
    printf("5. Enviar Float\n");
    printf("6. Enviar String\n\n");

    printf("0. Sair\n\n");

    printf("Escolha: ");

    scanf(" %d", &opt);

    while(opt < 0 || opt > 6) {
        printf("Escolha deve estar entre 0 e 6: ");
        scanf(" %d", &opt);
    }

    switch (opt) {
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    default:
        break;
    }

   return 0;
}