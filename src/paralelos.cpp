#include "paralelos.hpp"

bool programa_pode_continuar = true;

void signal_handler(int signum) {
    programa_pode_continuar = false;
    printf("Parando\n");
}

void ler_potenciometro(float *TR) {
    const char codigo = 0xA2;
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);      //Open in non blocking read/write mode
    if (uart0_filestream == -1) {
        printf("Erro - Não foi possível iniciar a UART.\n");
        programa_pode_continuar = false;
    }

    while(programa_pode_continuar) {
        struct termios options;
        tcgetattr(uart0_filestream, &options);
        options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     //<Set baud rate
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcflush(uart0_filestream, TCIFLUSH);
        tcsetattr(uart0_filestream, TCSANOW, &options);

        char padrao[] = {codigo, 0, 3, 9, 4};

        write(uart0_filestream, &padrao[0], 5);

        int count = read(uart0_filestream, TR, 4);

        usleep(100000);
    }

    close(uart0_filestream);
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
            *TI, *TE, *TR
        );
        usleep(2000000);
    }

    fclose(file);
}

void atualizar_temperaturas() {
    while(programa_pode_continuar) {
        // escreve em TI, TE
        usleep(100000);
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
