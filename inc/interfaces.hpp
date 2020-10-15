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

void iniciar_entrada(WINDOW *entrada);
void iniciar_saida(WINDOW *saida, const int size_x);
void iniciar_logs(WINDOW *logs);
void atualizar_menu(WINDOW *entrada, const int opcao_usuario, const float histerese);
void atualizar_saida(WINDOW *saida, const float TI, const float TE, const float TR);
void atualizar_logs(WINDOW *logs, const int dispositivo, const int novo_status, int *status_atual);
void aviso_encerramento(WINDOW *logs, const int status);
void pegar_opcao(WINDOW *entrada, int *opcao_usuario);
void pegar_temperatura(WINDOW *entrada, const float *TE, float *TR);
void pegar_histerese(WINDOW *entrada, const int opcao_usuario, float *histerese);

#endif // INTERFACES_HPP
