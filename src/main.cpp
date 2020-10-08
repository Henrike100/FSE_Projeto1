#include <ncurses.h>
#include <string>
#include <unistd.h>
#include <thread>
#include <mutex>

using namespace std;

mutex mtx;

// TR, TI, TE são de leitura e escrita: controlar acesso

// Se opção = 1: Usuário escolhe temperatura 1 vez e coloca em TR
// Se opção = 2: Thread verifica valor do potenciômetro e atualiza TR
float TR = 30.0;

// Temperatura Interna
float TI = 30.0;

// Temperatura Externa
float TE = 30.0;

int histerese = -1;

bool pode_continuar = true;

const string opcoes[] = {"Nenhuma", "Teclado", "Potenciômetro"};
int opt = 0;

void mostrar_menu(WINDOW *window) {
    int line_size = getmaxx(window);

    mvwprintw(window, 1, 1, " Opção atual: %s", opcoes[opt].c_str());
    mvwvline(window, 1, line_size/2, 0, 1);
    mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: Não definido");
    mvwhline(window, 2, 0, 0, line_size);
    mvwprintw(window, 3, 1, "Como deseja definir a temperatura?");

    mvwprintw(window, 5, 1, "1. Usando o teclado");
    mvwprintw(window, 6, 1, "2. Usando o Potenciômetro");
    mvwprintw(window, 7, 1, "3. Definir Histerese");
    mvwprintw(window, 9, 1, "0. Sair");

    wrefresh(window);
}

void pegar_temperatura() {

}

void pegar_histerese() {
    
}

void pegar_opcao(WINDOW *window) {
    do {
        bool invalid = false;
        int last_line = getmaxy(window);
        int line_size = getmaxx(window);
        wmove(window, last_line-2, 1);
        wclrtoeol(window);
        box(window, 0, 0);
        wrefresh(window);

        do {
            wmove(window, 11, 1);
            wclrtoeol(window);
            box(window, 0, 0);
            if(invalid) {
                mvwprintw(window, last_line-2, 1, "Escolha deve estar entre 0 e 3");
                wrefresh(window);
            }

            mvwprintw(window, 11, 1, "Escolha a opção: ");
            mvwscanw(window, 11, 18, " %d", &opt);

            invalid = opt < 0 || opt > 3;

            wrefresh(window);
        } while (invalid);

        wmove(window, 1, 1);
        wclrtoeol(window);
        box(window, 0, 0);
        wrefresh(window);
        mvwprintw(window, 1, 1, " Opção atual: %s", opcoes[opt].c_str());
        mvwvline(window, 1, line_size/2, 0, 1);
        mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: Não definido");
        wrefresh(window);

        if(histerese == -1) {
            bool invalidHisterese = false;
            do {
                wmove(window, 11, 1);
                wclrtoeol(window);
                box(window, 0, 0);

                if(invalidHisterese) {
                    mvwprintw(window, last_line-2, 1, "Histerese deve ser maior do que 0");
                    wrefresh(window);
                }

                mvwprintw(window, 11, 1, "Digite o valor da histerese: ");
                mvwscanw(window, 11, 30, " %d", &histerese);

                invalidHisterese = histerese < 1;
            } while (invalidHisterese);
            mtx.unlock();
        }

        wmove(window, 1, 1);
        wclrtoeol(window);
        box(window, 0, 0);
        wrefresh(window);
        mvwprintw(window, 1, 1, " Opção atual: %s", opcoes[opt].c_str());
        mvwvline(window, 1, line_size/2, 0, 1);
        mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: %d", histerese);
        wrefresh(window);
    } while(opt != 0);

    pode_continuar = false;
}

void mostrar_temperaturas(WINDOW *saida) {
    mtx.lock();
    mtx.unlock();
    int linha = 3;

    while(pode_continuar) {
        mvwprintw(saida, min(34, linha), 1," %.1f ", TI);
        wvline(saida, 0, 1);
        mvwprintw(saida, min(34, linha), 24," %.1f ", TE);
        wvline(saida, 0, 1);
        mvwprintw(saida, min(34, linha), 47," %.1f ", TR);

        box(saida, 0, 0);
        wrefresh(saida);
        if(linha >= 35) {
            scroll(saida);
        }
        linha++;
        usleep(100000);
    }
}

int main(int argc, const char *argv[]) {
    int size_x, size_y;
    initscr();

    getmaxyx(stdscr, size_y, size_x);

    WINDOW *entrada = newwin(size_y/2,size_x/2, 0, 0),
           *saida   = newwin(size_y,size_x/2, 0, size_x/2), 
           *logs    = newwin(size_y/2,size_x/2, size_y/2, 0);
        
    scrollok(saida, true);
    wsetscrreg(saida, 3, 34);

    box(entrada, 0, 0);
    box(saida, 0, 0);
    box(logs, 0, 0);

    refresh();
    wrefresh(entrada);
    wrefresh(saida);
    wrefresh(logs);

    mvwprintw(saida, 1, 1, " Temperatura Interna ");
    wvline(saida, 0, 1);
    mvwprintw(saida, 1, 24, " Temperatura Externa ");
    wvline(saida, 0, 1);
    mvwprintw(saida, 1, 47, " Temperatura de Referência ");
    mvwhline(saida, 2, 0, 0, size_x/2);
    box(saida, 0, 0);
    wrefresh(saida);

    mostrar_menu(entrada);

    mtx.lock();

    thread thread_entrada(pegar_opcao, entrada);
    thread thread_saida(mostrar_temperaturas, saida);
    
    thread_entrada.join();
    thread_saida.join();

    delwin(entrada);
    delwin(saida);
    delwin(logs);

    endwin();

    return 0;
}

