#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include <ncurses.h>
#include "constantes.hpp"

void mostrar_opcoes(WINDOW *window);
void atualizar_menu(WINDOW *window, const int opcao_usuario, const int opcao_anterior, const int histerese);
void iniciar_saida(WINDOW *window, const int size_x);
void iniciar_logs(WINDOW *window);
void atualizar_logs(WINDOW *window, string dispositivo, const int status);
void pegar_temperatura(WINDOW *window, const float TE, float *TR);
void pegar_histerese(WINDOW *window, const int opcao_usuario, const int opcao_anterior, int *histerese);

#endif // INTERFACES_HPP
