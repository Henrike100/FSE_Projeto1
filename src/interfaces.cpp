#include "interfaces.hpp"

mutex mtx_interface;

void mostrar_opcoes(WINDOW *window) {
    mtx_interface.lock();
    int line_size = getmaxx(window);

    mvwhline(window, 2, 0, 0, line_size);
    mvwprintw(window, 3, 1, "Como deseja definir a temperatura?");

    mvwprintw(window, 5, 1, "1. Usando o teclado");
    mvwprintw(window, 6, 1, "2. Usando o Potenciômetro");
    mvwprintw(window, 7, 1, "3. Definir Histerese");
    mvwprintw(window, 9, 1, "0. Sair");

    wrefresh(window);
    mtx_interface.unlock();
}

void atualizar_menu(WINDOW *window, const int opcao_usuario, const int opcao_anterior, const float histerese) {
    mtx_interface.lock();
    int line_size = getmaxx(window);
    int last_line = getmaxy(window);

    wmove(window, 1, 1);
    wclrtoeol(window);

    // Histerese não é uma opcao de entrada, portanto...
    if(opcao_usuario != 3) // se a opção escolhida não for 'Definir histerese', atualiza 'Entrada atual'
        mvwprintw(window, 1, 1, " Entrada atual: %s", opcoes[opcao_usuario].c_str());
    else // cao contrário, a opção que vale é a anterior
        mvwprintw(window, 1, 1, " Entrada atual: %s", opcoes[opcao_anterior].c_str());

    mvwvline(window, 1, line_size/2, 0, 1);

    if(histerese == -1)
        mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: Não definido");
    else
        mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: %.1f", histerese);

    wmove(window, last_line-2, 1);
    wclrtoeol(window);
    box(window, 0, 0);
    wrefresh(window);
    mtx_interface.unlock();
}

void iniciar_saida(WINDOW *window, const int size_x) {
    mtx_interface.lock();
    int line_size = getmaxx(window);
    const string spaces1(((line_size/3)-20)/2, ' ');
    const string spaces3(((line_size/3)-26)/2, ' ');

    mvwprintw(window, 1, 1, "%sTEMPERATURA INTERNA%s", spaces1.c_str(), spaces1.c_str());
    mvwvline(window, 1, line_size/3, 0, 1);
    mvwprintw(window, 1, line_size/3 + 1, "%sTEMPERATURA EXTERNA%s", spaces1.c_str(), spaces1.c_str());
    mvwvline(window, 1, 2*line_size/3, 0, 1);
    mvwprintw(window, 1, 2*line_size/3 + 1, "%sTEMPERATURA DE REFERENCIA%s", spaces3.c_str(), spaces3.c_str());
    mvwhline(window, 2, 0, 0, size_x);
    box(window, 0, 0);
    wrefresh(window);
    mtx_interface.unlock();
}

void iniciar_logs(WINDOW *window) {
    mtx_interface.lock();
    int line_size = getmaxx(window);

    mvwprintw(window, 1, 1, "Informacoes do Sistema");
    mvwvline(window, 3, line_size/2, 0, 17);
    mvwhline(window, 3, 0, 0, line_size);
    mvwprintw(window, 4, 1, "DISPOSITIVO"); mvwprintw(window, 4, (line_size/2)+2, "STATUS");
    mvwhline(window, 5, 0, 0, line_size);
    mvwprintw(window, 6, 1, dispositivos[0].c_str()); mvwprintw(window, 6, (line_size/2)+2, "Iniciando");
    mvwhline(window, 7, 0, 0, line_size);
    mvwprintw(window, 8, 1, dispositivos[1].c_str()); mvwprintw(window, 8, (line_size/2)+2, "Iniciando");
    mvwhline(window, 9, 0, 0, line_size);
    mvwprintw(window, 10, 1, dispositivos[2].c_str()); mvwprintw(window, 10, (line_size/2)+2, "Iniciando");
    mvwhline(window, 11, 0, 0, line_size);
    mvwprintw(window, 12, 1, dispositivos[3].c_str()); mvwprintw(window, 12, (line_size/2)+2, "Iniciando");
    mvwhline(window, 13, 0, 0, line_size);
    mvwprintw(window, 14, 1, dispositivos[4].c_str()); mvwprintw(window, 14, (line_size/2)+2, "Iniciando");
    mvwhline(window, 15, 0, 0, line_size);
    mvwprintw(window, 16, 1, dispositivos[5].c_str()); mvwprintw(window, 16, (line_size/2)+2, "Iniciando");
    mvwhline(window, 17, 0, 0, line_size);
    mvwprintw(window, 18, 1, dispositivos[6].c_str()); mvwprintw(window, 18, (line_size/2)+2, "Iniciando");
    mvwhline(window, 19, 0, 0, line_size);

    box(window, 0, 0);
    wrefresh(window);
    mtx_interface.unlock();
}

void atualizar_logs(WINDOW *window, const int dispositivo, const int st) {
    mtx_interface.lock();
    const int line_size = getmaxx(window);
    const int linha = (2*dispositivo)+6;

    wmove(window, linha, (line_size/2)+2);
    wclrtoeol(window);
    mvwprintw(window, linha, (line_size/2)+2, status[st].c_str());

    box(window, 0, 0);
    wrefresh(window);
    mtx_interface.unlock();
}

void pegar_temperatura(WINDOW *window, const float TE, float *TR) {
    mtx_interface.lock();
    int last_line = getmaxy(window);
    mtx_interface.unlock();
    bool invalid = false;
    float temperatura;

    do {
        mtx_interface.lock();
        wmove(window, 11, 1);
        wclrtoeol(window);
        box(window, 0, 0);

        if(invalid) {
            mvwprintw(window, last_line-2, 1, "O valor mínimo possível da temperatura é %.1f", TE);
        }

        wrefresh(window);

        mvwprintw(window, 11, 1, "Digite o valor da temperatura: ");
        mtx_interface.unlock();
        mvwscanw(window, 11, 32, " %f", &temperatura);
        invalid = temperatura < TE;
    } while (invalid);

    *TR = temperatura;
}

void pegar_histerese(WINDOW *window, const int opcao_usuario, const int opcao_anterior, float *histerese) {
    mtx_interface.lock();
    int last_line = getmaxy(window);
    mtx_interface.unlock();
    bool invalid = false;
    float temp;

    do {
        mtx_interface.lock();
        wmove(window, 11, 1);
        wclrtoeol(window);
        box(window, 0, 0);

        if(invalid) {
            mvwprintw(window, last_line-2, 1, "Histerese deve ser maior do que 0");
        }

        wrefresh(window);

        mvwprintw(window, 11, 1, "Digite o valor da histerese: ");
        mtx_interface.unlock();
        mvwscanw(window, 11, 30, " %f", &temp);
        invalid = !(temp > 0.0f);
    } while (invalid);

    *histerese = temp;

    atualizar_menu(window, opcao_usuario, opcao_anterior, *histerese);
}
