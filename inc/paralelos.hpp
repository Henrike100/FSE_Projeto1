#ifndef PARALELOS_HPP
#define PARALELOS_HPP

#include <stdio.h>
#include <ctime>
#include <signal.h>
#include <unistd.h>
#include "interfaces.hpp"

void signal_handler(int signum);
void pegar_opcao(WINDOW *window, int *opcao_usuario, int *opcao_anterior, int *histerese, float *TE, float *TR);
void mostrar_temperaturas(WINDOW *window, const int *opcao_usuario, const int *histerese,
                          const float *TI, const float *TE, const float *TR);
void gerar_log_csv(float *TI, float *TE, float *TR);
void atualizar_lcd(float *TI, float *TE, float *TR);

#endif // PARALELOS_HPP