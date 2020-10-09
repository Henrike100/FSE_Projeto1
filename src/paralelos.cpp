#include "paralelos.hpp"

const char path[] = "/dev/i2c-1";

bool programa_pode_continuar = true;

void signal_handler(int signum) {
    programa_pode_continuar = false;
}

void pegar_opcao(WINDOW *window, int *opcao_usuario, int *opcao_anterior, int *histerese, float *TE, float *TR) {
    int last_line = getmaxy(window);

    do {
        if(*opcao_usuario != 3)
            *opcao_anterior = *opcao_usuario;

        bool invalid = false;

        do {
            wmove(window, 11, 1);
            wclrtoeol(window);
            box(window, 0, 0);
            if(invalid) {
                mvwprintw(window, last_line-2, 1, "Escolha deve estar entre 0 e 3");
            }

            wrefresh(window);

            mvwprintw(window, 11, 1, "Escolha a opcao: ");
            mvwscanw(window, 11, 18, " %d", opcao_usuario);

            invalid = *opcao_usuario < 0 || *opcao_usuario > 3;
        } while (invalid);

        if(!(*opcao_usuario)) break;

        atualizar_menu(window, *opcao_usuario, *opcao_anterior, *histerese);

        switch (*opcao_usuario) {
        case 1:
            pegar_temperatura(window, *TE, TR);
            break;
        case 2:
            
            break;
        case 3:
            pegar_histerese(window, *opcao_usuario, *opcao_anterior, histerese);
            break;
        default:
            break;
        }

        if(*histerese == -1) {
            pegar_histerese(window, *opcao_usuario, *opcao_anterior, histerese);
        }
    } while(programa_pode_continuar);

    programa_pode_continuar = false;
}

void mostrar_temperaturas(WINDOW *window, const int *opcao_usuario, const int *histerese,
                          const float *TI, const float *TE, const float *TR) {
    int linha = 3;
    int last_line = getmaxy(window) - 2;

    while(programa_pode_continuar) {
        if(*opcao_usuario == 0 or *histerese == -1) {
            usleep(100000);
            continue;
        }
        mvwprintw(window, min(last_line, linha), 1," %.1f ", *TI);
        wvline(window, 0, 1);
        mvwprintw(window, min(last_line, linha), 24," %.1f ", *TE);
        wvline(window, 0, 1);
        mvwprintw(window, min(last_line, linha), 47," %.1f", *TR);

        box(window, 0, 0);
        wrefresh(window);
        if(linha >= last_line+1) {
            scroll(window);
        }
        else
            linha++;
        usleep(100000);
    }
}

void gerar_log_csv(float *TI, float *TE, float *TR) {
    FILE *file;
    file = fopen("arquivo.csv", "w+");
    fprintf(file, "Data/Hora, Temperatura Interna, Temperatura Externa, Temperatura de Referência\n");

    while(programa_pode_continuar) {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        fprintf(file, "%02d/%02d/%d %02d:%02d:%02d, %.1f, %.1f, %.1f\n",
            ltm->tm_mday,
            ltm->tm_mon+1,
            ltm->tm_year+1900,
            ltm->tm_hour+1,
            ltm->tm_min+1,
            ltm->tm_sec+1,
            *TI, *TE, *TR
        );
        usleep(2000000);
    }

    fclose(file);
}

void atualizar_lcd(float *TI, float *TE, float *TR) {
    while(programa_pode_continuar) {
        //printf("Atualiza LCD a cada 1s\n");
        usleep(1000000);
    }
}
