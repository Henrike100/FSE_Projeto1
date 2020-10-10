#include <thread>

#include "interfaces.hpp"
#include "paralelos.hpp"

using namespace std;

float TR = 30.0;
float TI = 30.0;
float TE = 30.0;

float histerese = -1.0f;

int opcao_usuario = 0, opcao_anterior = 0;

int main(int argc, const char *argv[]) {
    signal(SIGINT, signal_handler);
    int size_x, size_y;
    initscr();

    getmaxyx(stdscr, size_y, size_x);

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
    thread thread_uart(comunicar_uart, logs, &TI, &TR);

    thread thread_entrada(pegar_opcao, entrada, &opcao_usuario, &opcao_anterior, &histerese, &TE, &TR);
    thread thread_saida(mostrar_temperaturas, saida, &opcao_usuario, &histerese, &TI, &TE, &TR);
    
    thread_csv.join();
    thread_uart.join();
    thread_entrada.join();
    thread_saida.join();

    delwin(entrada);
    delwin(saida);
    delwin(logs);

    endwin();

    return 0;
}
