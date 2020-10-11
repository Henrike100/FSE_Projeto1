#include <thread>

#include "paralelos.hpp"

using namespace std;

float TR = 30.0;
float TI = 30.0;
float TE = 30.0;

float histerese = -1.0f;

int opcao_usuario = 0, opcao_anterior = 0;

int main(int argc, const char *argv[]) {
    signal(SIGHUP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    signal(SIGALRM, alarm_handler);

    int size_x, size_y;
    initscr();

    getmaxyx(stdscr, size_y, size_x);

    if(size_y < 25 or size_x < 144) {
        endwin();
        printf("Para uma melhor experiência, ajuste o tamanho do terminal para, no mínimo:\n");
        printf("144 colunas e 25 linhas (144x25)\n");
        printf("Atual: %dx%d\n", size_x, size_y);
        return 0;
    }

    WINDOW *saida   = newwin(5, size_x, 0, 0),
           *entrada = newwin(size_y-5, size_x/2-1, 5, 0),
           *logs    = newwin(size_y-5, size_x/2, 5, size_x/2);

    box(entrada, 0, 0);
    box(saida, 0, 0);
    box(logs, 0, 0);

    refresh();
    wrefresh(entrada);
    wrefresh(saida);
    wrefresh(logs);

    mostrar_opcoes(entrada);
    iniciar_saida(saida, size_x);
    iniciar_logs(logs);

    atualizar_menu(entrada, opcao_usuario, opcao_anterior, histerese);

    thread thread_csv(gerar_log_csv, logs, &TI, &TE, &TR);
    thread thread_uart(comunicar_uart, logs, &TI, &TR, &opcao_usuario);
    thread thread_i2c(usar_i2c, logs, &TI, &TE, &TR);

    ualarm(100000, 0);

    thread thread_entrada(pegar_opcao, entrada, &opcao_usuario, &opcao_anterior, &histerese, &TE, &TR);
    thread thread_saida(mostrar_temperaturas, saida, &histerese, &TI, &TE, &TR);
    
    thread_csv.join();
    thread_uart.join();
    thread_i2c.join();
    thread_entrada.join();
    thread_saida.join();

    delwin(entrada);
    delwin(saida);
    delwin(logs);

    endwin();

    return 0;
}
