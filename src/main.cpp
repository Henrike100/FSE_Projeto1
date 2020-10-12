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
    const int min_colunas = 144, min_linhas = 28;

    if(size_y < min_linhas or size_x < min_colunas) {
        endwin();
        printf("Para uma melhor experiência, ajuste o tamanho do terminal para, no mínimo:\n");
        printf("%d colunas e %d linhas (%dx%d)\n", min_colunas, min_linhas, min_colunas, min_linhas);
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

    // Só continua depois de verificar todos os dispositivos
    unique_lock<mutex> lck(mtx_main);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);

    // Se nem todos os dispositivos forem inicializados corretamente
    if(qtd_dispositivos_funcionando != qtd_dispositivos_verificados) {
        thread_csv.join();
        thread_uart.join();
        thread_i2c.join();

        aviso_erro(logs);

        delwin(entrada);
        delwin(saida);
        delwin(logs);

        endwin();

        return 0;
    }

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
