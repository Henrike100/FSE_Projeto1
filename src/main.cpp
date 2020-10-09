#include <thread>

#include "interfaces.hpp"
#include "paralelos.hpp"

using namespace std;

float TR = 30.0;
float TI = 30.0;
float TE = 30.0;

int histerese = -1;

int opcao_usuario = 0, opcao_anterior = 0;

int main(int argc, const char *argv[]) {
    signal(SIGINT, signal_handler);
    int size_x, size_y;
    initscr();

    getmaxyx(stdscr, size_y, size_x);

    WINDOW *entrada = newwin(size_y/2,size_x/2, 0, 0),
           *saida   = newwin(size_y,size_x/2, 0, size_x/2), 
           *logs    = newwin(size_y/2,size_x/2, size_y/2, 0);
        
    scrollok(saida, true);
    wsetscrreg(saida, 3, size_y-2);

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

    thread thread_lcd(atualizar_lcd, logs, &TI, &TE, &TR);
    thread thread_csv(gerar_log_csv, logs, &TI, &TE, &TR);
    thread thread_uart(ler_UART, logs, &TI, &TR);

    thread thread_entrada(pegar_opcao, entrada, &opcao_usuario, &opcao_anterior, &histerese, &TE, &TR);
    thread thread_saida(mostrar_temperaturas, saida, &opcao_usuario, &histerese, &TI, &TE, &TR);
    
    thread_lcd.join();
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
