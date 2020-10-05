#ifndef PARALELOS_HPP
#define PARALELOS_HPP

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <ctime>

void signal_handler(int signum);
void ler_potenciometro();
void gerar_log_csv(float *TI, float *TE, float *TR);
void atualizar_temperaturas();
void apresentar_temperaturas(float *TI, float *TE, float *TR);
void atualizar_lcd();

#endif // PARALELOS_HPP