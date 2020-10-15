#include "interfaces.hpp"

mutex mtx_interface;
mutex mtx_TR;
mutex mtx_TI;
mutex mtx_TE;
mutex mtx_opcao;
mutex mtx_histerese;

void iniciar_entrada(WINDOW *entrada) {
    mtx_interface.lock();
    int line_size = getmaxx(entrada);

    mvwprintw(entrada, 1, 1, " Entrada atual: Nenhuma");
    mvwvline(entrada, 1, line_size/2, 0, 1);
    mvwprintw(entrada, 1, (line_size/2)+1, " Valor da Histerese: Não definido");

    mvwhline(entrada, 2, 0, 0, line_size);
    mvwprintw(entrada, 3, 1, "Como deseja definir a temperatura?");

    mvwprintw(entrada, 5, 1, "1. Usando o teclado");
    mvwprintw(entrada, 6, 1, "2. Usando o Potenciômetro");
    mvwprintw(entrada, 7, 1, "3. Definir Histerese");
    mvwprintw(entrada, 9, 1, "0. Sair");
    box(entrada, 0, 0);
    wrefresh(entrada);
    mtx_interface.unlock();
}

void iniciar_saida(WINDOW *saida, const int size_x) {
    mtx_interface.lock();
    int line_size = getmaxx(saida);
    const string spaces1(((line_size/3)-20)/2, ' ');
    const string spaces3(((line_size/3)-26)/2, ' ');

    mvwprintw(saida, 1, 1, "%sTEMPERATURA INTERNA%s", spaces1.c_str(), spaces1.c_str());
    mvwvline(saida, 1, line_size/3, 0, 1);
    mvwprintw(saida, 1, line_size/3 + 1, "%sTEMPERATURA EXTERNA%s", spaces1.c_str(), spaces1.c_str());
    mvwvline(saida, 1, 2*line_size/3, 0, 1);
    mvwprintw(saida, 1, 2*line_size/3 + 1, "%sTEMPERATURA DE REFERENCIA%s", spaces3.c_str(), spaces3.c_str());
    mvwhline(saida, 2, 0, 0, size_x);
    box(saida, 0, 0);
    wrefresh(saida);
    mtx_interface.unlock();
}

void iniciar_logs(WINDOW *logs) {
    mtx_interface.lock();
    int line_size = getmaxx(logs);

    mvwprintw(logs, 1, 1, "Informacoes do Sistema");
    mvwvline(logs, 3, line_size/2, 0, 17);
    mvwhline(logs, 3, 0, 0, line_size);
    mvwprintw(logs, 4, 1, "DISPOSITIVO"); mvwprintw(logs, 4, (line_size/2)+2, "STATUS");
    mvwhline(logs, 5, 0, 0, line_size);
    mvwprintw(logs, 6, 1, dispositivos[0].c_str()); mvwprintw(logs, 6, (line_size/2)+2, "Iniciando");
    mvwhline(logs, 7, 0, 0, line_size);
    mvwprintw(logs, 8, 1, dispositivos[1].c_str()); mvwprintw(logs, 8, (line_size/2)+2, "Iniciando");
    mvwhline(logs, 9, 0, 0, line_size);
    mvwprintw(logs, 10, 1, dispositivos[2].c_str()); mvwprintw(logs, 10, (line_size/2)+2, "Iniciando");
    mvwhline(logs, 11, 0, 0, line_size);
    mvwprintw(logs, 12, 1, dispositivos[3].c_str()); mvwprintw(logs, 12, (line_size/2)+2, "Iniciando");
    mvwhline(logs, 13, 0, 0, line_size);
    mvwprintw(logs, 14, 1, dispositivos[4].c_str()); mvwprintw(logs, 14, (line_size/2)+2, "Iniciando");
    mvwhline(logs, 15, 0, 0, line_size);
    mvwprintw(logs, 16, 1, dispositivos[5].c_str()); mvwprintw(logs, 16, (line_size/2)+2, "Iniciando");
    mvwhline(logs, 17, 0, 0, line_size);
    mvwprintw(logs, 18, 1, dispositivos[6].c_str()); mvwprintw(logs, 18, (line_size/2)+2, "Iniciando");
    mvwhline(logs, 19, 0, 0, line_size);

    box(logs, 0, 0);
    wrefresh(logs);
    mtx_interface.unlock();
}

void atualizar_menu(WINDOW *entrada, const int opcao_usuario, const float histerese) {
    mtx_interface.lock();
    int line_size = getmaxx(entrada);
    int last_line = getmaxy(entrada);

    if(opcao_usuario == 1 or opcao_usuario == 2) {
        wmove(entrada, 1, 1);
        wclrtoeol(entrada);
        mvwprintw(entrada, 1, 1, " Entrada atual: %s", opcoes[opcao_usuario].c_str());
    }
    else {
        wmove(entrada, 1, line_size/2);
        wclrtoeol(entrada);
    }

    mvwvline(entrada, 1, line_size/2, 0, 1);

    if(histerese > 0)
        mvwprintw(entrada, 1, (line_size/2)+1, " Valor da Histerese: %.1f", histerese);
    else
        mvwprintw(entrada, 1, (line_size/2)+1, " Valor da Histerese: Não definido");

    wmove(entrada, last_line-2, 1);
    wclrtoeol(entrada);
    box(entrada, 0, 0);
    wrefresh(entrada);
    mtx_interface.unlock();
}

void atualizar_saida(WINDOW *saida, const float TI, const float TE, const float TR) {
    mtx_TI.lock();
    mtx_TE.lock();
    mtx_TR.lock();
    mtx_interface.lock();

    const int line_size = getmaxx(saida);

    const string spaces(((line_size/3)-5)/2, ' ');

    mvwprintw(saida, 3, 1, "%s%.1f%s", spaces.c_str(), TI, spaces.c_str());
    mvwvline(saida, 3, line_size/3, 0, 1);
    mvwprintw(saida, 3, line_size/3+1, "%s%.1f%s", spaces.c_str(), TE, spaces.c_str());
    mvwvline(saida, 3, 2*line_size/3, 0, 1);

    if(TR > 0) {
        mvwprintw(saida, 3, 2*line_size/3+1, "%s%.1f%s", spaces.c_str(), TR, spaces.c_str());
    }

    box(saida, 0, 0);
    wrefresh(saida);

    mtx_interface.unlock();
    mtx_TR.unlock();
    mtx_TE.unlock();
    mtx_TI.unlock();
}

void atualizar_logs(WINDOW *logs, const int dispositivo, const int novo_status, int *status_atual) {
    if(novo_status == *status_atual)
        return;

    mtx_interface.lock();
    const int line_size = getmaxx(logs);
    const int linha = (2*dispositivo)+6;

    wmove(logs, linha, (line_size/2)+2);
    wclrtoeol(logs);
    mvwprintw(logs, linha, (line_size/2)+2, status[novo_status].c_str());

    box(logs, 0, 0);
    wrefresh(logs);
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

void pegar_opcao(WINDOW *entrada, int *opcao_usuario) {
    mtx_interface.lock();
    int last_line = getmaxy(entrada);
    mtx_interface.unlock();
    int opcao;
    bool invalid = false;

    do {
        mtx_interface.lock();
        wmove(entrada, 11, 1);
        wclrtoeol(entrada);
        box(entrada, 0, 0);

        if(invalid) {
            mvwprintw(entrada, last_line-2, 1, "Escolha deve estar entre 0 e 3");
        }

        wrefresh(entrada);

        mvwprintw(entrada, 11, 1, "Escolha a opcao: ");
        mtx_interface.unlock();
        mvwscanw(entrada, 11, 18, " %d", &opcao);
        invalid = opcao < 0 || opcao > 3;
    } while (invalid);

    mtx_opcao.lock();
    *opcao_usuario = opcao;
    mtx_opcao.unlock();
}

void pegar_temperatura(WINDOW *entrada, const float *TE, float *TR) {
    mtx_interface.lock();
    int last_line = getmaxy(entrada);
    mtx_interface.unlock();
    bool invalid = false;
    float temperatura;

    do {
        mtx_interface.lock();
        wmove(entrada, 11, 1);
        wclrtoeol(entrada);
        box(entrada, 0, 0);

        if(invalid) {
            mvwprintw(entrada, last_line-2, 1, "O valor mínimo possível da temperatura é %.1f", *TE);
        }

        wrefresh(entrada);

        mvwprintw(entrada, 11, 1, "Digite o valor da temperatura: ");
        mtx_interface.unlock();
        mvwscanw(entrada, 11, 32, " %f", &temperatura);
        invalid = temperatura < *TE;
    } while (invalid);

    mtx_TR.lock();
    *TR = temperatura;
    mtx_TR.unlock();
}

void pegar_histerese(WINDOW *entrada, const int opcao_usuario, float *histerese) {
    mtx_interface.lock();
    int last_line = getmaxy(entrada);
    mtx_interface.unlock();
    bool invalid = false;
    float temp;

    do {
        mtx_interface.lock();
        wmove(entrada, 11, 1);
        wclrtoeol(entrada);
        box(entrada, 0, 0);

        if(invalid) {
            mvwprintw(entrada, last_line-2, 1, "Histerese deve ser maior do que 0");
        }

        wrefresh(entrada);

        mvwprintw(entrada, 11, 1, "Digite o valor da histerese: ");
        mtx_interface.unlock();
        mvwscanw(entrada, 11, 30, " %f", &temp);
        invalid = !(temp > 0.0f);
    } while (invalid);

    mtx_histerese.lock();
    *histerese = temp;
    mtx_histerese.unlock();

    atualizar_menu(entrada, opcao_usuario, *histerese);
}
