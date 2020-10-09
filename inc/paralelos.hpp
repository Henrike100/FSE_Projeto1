#ifndef PARALELOS_HPP
#define PARALELOS_HPP

#include <stdio.h>
#include <ctime>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include "interfaces.hpp"
#include "constantes.hpp"

void signal_handler(int signum);
void pegar_opcao(WINDOW *window, int *opcao_usuario, int *opcao_anterior, int *histerese, float *TE, float *TR);
void mostrar_temperaturas(WINDOW *window, const int *opcao_usuario, const int *histerese,
                          const float *TI, const float *TE, const float *TR);
void gerar_log_csv(WINDOW *window, float *TI, float *TE, float *TR);
void atualizar_lcd(WINDOW *window, float *TI, float *TE, float *TR);
void ler_UART(WINDOW *window, float *TI, float *TR);

#endif // PARALELOS_HPP