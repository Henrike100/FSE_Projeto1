#include "interfaces.hpp"

mutex mtx_interface;
mutex mtx_TR;
mutex mtx_TI;
mutex mtx_TE;
mutex mtx_opcao;
mutex mtx_histerese;

void iniciar_entrada(WINDOW *window) {
    mtx_interface.lock();
    int line_size = getmaxx(window);

    mvwprintw(window, 1, 1, " Entrada atual: Nenhuma");
    mvwvline(window, 1, line_size/2, 0, 1);
    mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: Não definido");

    mvwhline(window, 2, 0, 0, line_size);
    mvwprintw(window, 3, 1, "Como deseja definir a temperatura?");

    mvwprintw(window, 5, 1, "1. Usando o teclado");
    mvwprintw(window, 6, 1, "2. Usando o Potenciômetro");
    mvwprintw(window, 7, 1, "3. Definir Histerese");
    mvwprintw(window, 9, 1, "0. Sair");

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

void atualizar_menu(WINDOW *window, const int opcao_usuario, const float histerese) {
    mtx_interface.lock();
    int line_size = getmaxx(window);
    int last_line = getmaxy(window);

    if(opcao_usuario == 1 or opcao_usuario == 2) {
        wmove(window, 1, 1);
        wclrtoeol(window);
        mvwprintw(window, 1, 1, " Entrada atual: %s", opcoes[opcao_usuario].c_str());
    }
    else {
        wmove(window, 1, line_size/2);
        wclrtoeol(window);
    }

    mvwvline(window, 1, line_size/2, 0, 1);

    if(histerese > 0)
        mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: %.1f", histerese);
    else
        mvwprintw(window, 1, (line_size/2)+1, " Valor da Histerese: Não definido");

    wmove(window, last_line-2, 1);
    wclrtoeol(window);
    box(window, 0, 0);
    wrefresh(window);
    mtx_interface.unlock();
}

void atualizar_logs(WINDOW *window, const int dispositivo, const int novo_status, int *status_atual) {
    if(novo_status == *status_atual)
        return;

    mtx_interface.lock();
    const int line_size = getmaxx(window);
    const int linha = (2*dispositivo)+6;

    wmove(window, linha, (line_size/2)+2);
    wclrtoeol(window);
    mvwprintw(window, linha, (line_size/2)+2, status[novo_status].c_str());

    box(window, 0, 0);
    wrefresh(window);
    mtx_interface.unlock();

    *status_atual = novo_status;
}

void aviso_encerramento(WINDOW *logs, const int status) {
    mtx_interface.lock();

    if(status == ENCERRAMENTO_VIA_USER) {
        wmove(logs, 21, 1);
        wclrtoeol(logs);
        mvwprintw(logs, 21, 1, "Voce escolheu encerrar o programa. Encerrando dispositivos...");
        box(logs, 0, 0);
        wrefresh(logs);

        mtx_interface.unlock();
        return;
    }
    else if(ENCERRAMENTO_VIA_SIGNAL) {
        wmove(logs, 21, 1);
        wclrtoeol(logs);
        mvwprintw(logs, 21, 1, "O programa recebeu um sinal para encerrar a execucao.");
        mvwprintw(logs, 22, 1, "Assim que voce enviar algum dado, o programa sera encerrado.");
        box(logs, 0, 0);
        wrefresh(logs);

        mtx_interface.unlock();
        return;
    }

    int segundos = 5;

    while(segundos) {
        wmove(logs, 21, 1);
        wclrtoeol(logs);

        switch (status) {
        case ENCERRAMENTO_COM_ERRO_INICIO:
            mvwprintw(logs, 21, 1, "Nem todos os dispositivos conseguiram ser inicializados. Encerrando em %d...", segundos);
            break;
        case ENCERRAMENTO_COM_ERRO_EXEC:
            mvwprintw(logs, 21, 1, "Um dos dispositivos apresentou erro durante a execucao. Encerrando em %d...", segundos);
            break;
        default:
            mvwprintw(logs, 21, 1, "O programa foi encerrado por uma razao desconhecida. Encerrando em %d...", segundos);
            break;
        }

        box(logs, 0, 0);
        wrefresh(logs);
        segundos--;
        sleep(1);
    }

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

    mtx_TR.lock();
    *TR = temperatura;
    mtx_TR.unlock();
}

void pegar_histerese(WINDOW *window, const int opcao_usuario, float *histerese) {
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

    mtx_histerese.lock();
    *histerese = temp;
    mtx_histerese.unlock();

    atualizar_menu(window, opcao_usuario, *histerese);
}
