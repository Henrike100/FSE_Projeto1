#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include <ncurses.h>
#include <mutex>
#include <unistd.h>
#include "constantes.hpp"

extern mutex mtx_interface;

extern mutex mtx_TR;
extern mutex mtx_TI;
extern mutex mtx_TE;
extern mutex mtx_opcao;
extern mutex mtx_histerese;

void iniciar_entrada(WINDOW *window);
void atualizar_menu(WINDOW *window, const int opcao_usuario, const float histerese);
void iniciar_saida(WINDOW *window, const int size_x);
void iniciar_logs(WINDOW *window);
void atualizar_logs(WINDOW *window, const int dispositivo, const int st);
void aviso_encerramento(WINDOW *logs, const int status);
void pegar_temperatura(WINDOW *window, const float TE, float *TR);
void pegar_histerese(WINDOW *window, const int opcao_usuario, float *histerese);

#endif // INTERFACES_HPP
