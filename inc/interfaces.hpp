#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include <ncurses.h>
#include <mutex>
#include <unistd.h>
#include "constantes.hpp"

extern mutex mtx_interface;

void mostrar_opcoes(WINDOW *window);
void atualizar_menu(WINDOW *window, const int opcao_usuario, const int opcao_anterior, const float histerese);
void iniciar_saida(WINDOW *window, const int size_x);
void iniciar_logs(WINDOW *window);
void atualizar_logs(WINDOW *window, const int dispositivo, const int st);
void aviso_erro(WINDOW *window);
void pegar_temperatura(WINDOW *window, const float TE, float *TR);
void pegar_histerese(WINDOW *window, const int opcao_usuario, const int opcao_anterior, float *histerese);

#endif // INTERFACES_HPP
