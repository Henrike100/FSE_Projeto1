#include <thread>

#include "paralelos.hpp"

using namespace std;

float TR;
float TI;
float TE;

float histerese = -1.0f;

int opcao_usuario = 0;

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

    iniciar_entrada(entrada);
    iniciar_saida(saida, size_x);
    iniciar_logs(logs);

    thread thread_csv(gerar_log_csv, logs);
    thread thread_uart(comunicar_uart, logs);
    thread thread_i2c(usar_i2c, logs);
    thread thread_gpio(usar_gpio, logs);

    // Só continua depois de verificar todos os dispositivos
    unique_lock<mutex> lck(mtx_main);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);

    // Se nem todos os dispositivos forem inicializados corretamente
    if(qtd_dispositivos_funcionando != qtd_dispositivos_verificados) {
        thread_csv.join();
        thread_uart.join();
        thread_i2c.join();
        thread_gpio.join();

        aviso_encerramento(logs, ENCERRAMENTO_COM_ERRO_INICIO);

        delwin(entrada);
        delwin(saida);
        delwin(logs);

        endwin();

        return 0;
    }

    ualarm(100000, 0);

    thread thread_entrada(pegar_opcao, entrada);
    thread thread_saida(mostrar_temperaturas, saida);
    
    thread_csv.join();
    thread_uart.join();
    thread_i2c.join();
    thread_gpio.join();
    thread_entrada.join();
    thread_saida.join();

    aviso_encerramento(logs, status_programa);

    delwin(entrada);
    delwin(saida);
    delwin(logs);

    endwin();

    return 0;
}
