#include "paralelos.hpp"

bool programa_pode_continuar = true;

void signal_handler(int signum) {
    programa_pode_continuar = false;
    printf("Parando\n");
}

void ler_potenciometro() {
    while(programa_pode_continuar) {
        //printf("Lê Potenciometro a cada 100ms\n");
        usleep(100000);
    }
}

void gerar_log_csv(float *TI, float *TE, float *TR) {
    FILE *file;
    file = fopen("arquivo.csv", "w+");
    fprintf(file, "Data/Hora, Temperatura Interna, Temperatura Externa, Temperatura de Referência\n");

    while(programa_pode_continuar) {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        fprintf(file, "%02d/%02d/%d %02d:%02d:%02d, %.1f, %.1f, %.1f\n",
        ltm->tm_mday,
        ltm->tm_mon+1,
        ltm->tm_year+1900,
        ltm->tm_hour+1,
        ltm->tm_min+1,
        ltm->tm_sec+1,
        *TI, *TE, *TR);
        usleep(2000000);
    }

    fclose(file);
}

void atualizar_temperaturas() {
    while(programa_pode_continuar) {
        // escreve em TI, TE, TR
        usleep(500000);
    }
}

void apresentar_temperaturas(float *TI, float *TE, float *TR) {
    bool first = true;
    while(programa_pode_continuar) {
        printf("-------------------------------------------------------------------------\n");
        if(first) {
            printf("| Temperatura Interna | Temperatura Externa | Temperatura de Referência |\n");
            printf("-------------------------------------------------------------------------\n");
            first = false;
        }
        printf("|         %.1lf        |         %.1lf        |           %.1lf            |\n", *TI, *TE, *TR);
        usleep(500000);
    }
}

void atualizar_lcd() {
    while(programa_pode_continuar) {
        //printf("Atualiza LCD a cada 1s\n");
        usleep(1000000);
    }
}
