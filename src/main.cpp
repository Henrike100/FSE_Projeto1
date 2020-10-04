#include <stdio.h>
#include <thread>
#include <signal.h>
#include <ctime>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART

using namespace std;

// TR, TI, TE são de leitura e escrita: controlar acesso

// Se opção = 1: Usuário escolhe temperatura 1 vez e coloca em TR
// Se opção = 2: Thread verifica valor do potenciômetro e atualiza TR
float TR;

// Temperatura Interna
float TI = 0.0;

// Temperatura Externa
float TE = 0.0;

bool programa_pode_continuar = true;

void signal_handler(int signum) {
    programa_pode_continuar = false;
    printf("Parando\n");
}

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

float pegar_temperatura() {
    float temperatura = -1.0f;

    printf("Defina a temperatura: ");
    scanf(" %f", &temperatura);

    while(temperatura < TE) {
        printf("A menor temperatura possível é %.1f. Escolha outro valor: ", TE);
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

void ler_potenciometro() {
    while(programa_pode_continuar) {
        printf("Lê Potenciometro a cada 100ms\n");
        usleep(100000);
    }
}

void gerar_log_csv() {
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
        TI, TE, TR);
        usleep(2000000);
    }

    fclose(file);
}

void atualiza_temperaturas() {
    while(programa_pode_continuar) {
        // escreve em TI, TE, TR
        usleep(500000);
    }
}

void apresenta_temperaturas() {
    while(programa_pode_continuar) {
        printf("-------------------------------------\n");
        printf("Temperatura Interna: %.1f\n", TI);
        printf("Temperatura Externa: %.1f\n", TE);
        printf("Temperatura de Referência: %.1f\n", TR);
        usleep(500000);
    }
}

void atualiza_lcd() {
    while(programa_pode_continuar) {
        printf("Atualiza LCD a cada 1s\n");
        usleep(1000000);
    }
}

int main(int argc, const char *argv[]) {
    signal(SIGINT, signal_handler);
    thread thread_temperaturas(atualiza_temperaturas);
    thread thread_potenciometro;
    
    int opt = menu_inicializacao();

    switch (opt) {
    case 1:
        TR = pegar_temperatura();
        break;
    case 2:
        thread_potenciometro = thread(ler_potenciometro);
        break;
    default:
        return 0;
    }

    float histerese = pegar_histerese();

    thread thread_apresenta(apresenta_temperaturas);
    thread thread_lcd(atualiza_lcd);
    thread thread_csv(gerar_log_csv);
    
    thread_temperaturas.join();
    if(opt == 2) thread_potenciometro.join();
    thread_apresenta.join();
    thread_lcd.join();
    thread_csv.join();

   return 0;
}
