#include "interfaces.hpp"

void mostrar_opcoes(WINDOW *window) {
    int line_size = getmaxx(window);

    mvwhline(window, 2, 0, 0, line_size);
    mvwprintw(window, 3, 1, "Como deseja definir a temperatura?");

    mvwprintw(window, 5, 1, "1. Usando o teclado");
    mvwprintw(window, 6, 1, "2. Usando o Potenciômetro");
    mvwprintw(window, 7, 1, "3. Definir Histerese");
    mvwprintw(window, 9, 1, "0. Sair");

    wrefresh(window);
}

void atualizar_menu(WINDOW *window, const int opcao_usuario, const int opcao_anterior, const int histerese) {
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
        mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: %d", histerese);

    wmove(window, last_line-2, 1);
    wclrtoeol(window);
    box(window, 0, 0);
    wrefresh(window);
}

void iniciar_saida(WINDOW *window, const int size_x) {
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
}

void iniciar_logs(WINDOW *window) {
    int line_size = getmaxx(window);
    int vertical_size = getmaxy(window);

    mvwprintw(window, 1, 1, "Informacoes do Sistema");
    mvwvline(window, 3, line_size/2, 0, vertical_size-18);
    mvwhline(window, 3, 0, 0, line_size);
    mvwprintw(window, 4, 1, "DISPOSITIVO"); mvwprintw(window, 4, (line_size/2)+2, "STATUS");
    mvwhline(window, 5, 0, 0, line_size);
    mvwprintw(window, 6, 1, "Arduino (TI / TR)"); mvwprintw(window, 6, (line_size/2)+2, "Iniciando");
    mvwhline(window, 7, 0, 0, line_size);
    mvwprintw(window, 8, 1, "Resistor"); mvwprintw(window, 8, (line_size/2)+2, "Iniciando");
    mvwhline(window, 9, 0, 0, line_size);
    mvwprintw(window, 10, 1, "Ventoinha"); mvwprintw(window, 10, (line_size/2)+2, "Iniciando");
    mvwhline(window, 11, 0, 0, line_size);
    mvwprintw(window, 12, 1, "Sensor Externo (TE)"); mvwprintw(window, 12, (line_size/2)+2, "Iniciando");
    mvwhline(window, 13, 0, 0, line_size);
    mvwprintw(window, 14, 1, "Display LCD"); mvwprintw(window, 14, (line_size/2)+2, "Iniciando");
    mvwhline(window, 15, 0, 0, line_size);
    mvwprintw(window, 16, 1, "Arquivo CSV"); mvwprintw(window, 16, (line_size/2)+2, "Iniciando");
    mvwhline(window, 17, 0, 0, line_size);

    box(window, 0, 0);
    wrefresh(window);
}

void atualizar_logs(WINDOW *window, string dispositivo, const int status) {
    int line_size = getmaxx(window);
    int linha;

    if(dispositivo == "UART") {
        linha = 6;
        dispositivo = "Arduino (TI / TR)";
    }
    else if(dispositivo == "CSV") {
        linha = 16;
        dispositivo = "Arquivo CSV";
    }

    string status_str;

    switch (status) {
    case -1:
        status_str = "Erro ao abrir";
        break;
    case 0:
        status_str = "Iniciando";
        break;
    case 1:
        status_str = "Funcionando";
        break;
    case 2:
        status_str = "Encerrado";
        break;
    default:
        break;
    }

    wmove(window, linha, (line_size/2)+2);
    wclrtoeol(window);
    mvwprintw(window, linha, (line_size/2)+2, status_str.c_str());

    box(window, 0, 0);
    wrefresh(window);
}

void pegar_temperatura(WINDOW *window, const float TE, float *TR) {
    int last_line = getmaxy(window);
    bool invalid = false;
    float temperatura;

    do {
        wmove(window, 11, 1);
        wclrtoeol(window);
        box(window, 0, 0);

        if(invalid) {
            mvwprintw(window, last_line-2, 1, "O valor mínimo possível da temperatura é %.1f", TE);
        }

        wrefresh(window);

        mvwprintw(window, 11, 1, "Digite o valor da temperatura: ");
        mvwscanw(window, 11, 32, " %f", &temperatura);
        invalid = temperatura < TE;
    } while (invalid);

    *TR = temperatura;
}

void pegar_histerese(WINDOW *window, const int opcao_usuario, const int opcao_anterior, int *histerese) {
    int last_line = getmaxy(window);
    bool invalid = false;
    int temp;

    do {
        wmove(window, 11, 1);
        wclrtoeol(window);
        box(window, 0, 0);

        if(invalid) {
            mvwprintw(window, last_line-2, 1, "Histerese deve ser maior do que 0");
        }

        wrefresh(window);

        mvwprintw(window, 11, 1, "Digite o valor da histerese: ");
        mvwscanw(window, 11, 30, " %d", &temp);
        invalid = temp < 1;
    } while (invalid);

    *histerese = temp;

    atualizar_menu(window, opcao_usuario, opcao_anterior, *histerese);
}
