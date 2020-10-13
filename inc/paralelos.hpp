#ifndef PARALELOS_HPP
#define PARALELOS_HPP

#include <stdio.h>
#include <ctime>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/types.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <condition_variable>

#include "interfaces.hpp"
#include "constantes.hpp"
#include "bme280.hpp"
#include "utilidades.hpp"
#include "lcd.hpp"

extern mutex mtx_main;

extern condition_variable cv;
extern int qtd_dispositivos_funcionando;
extern int qtd_dispositivos_verificados;

void signal_handler(int signum);
void alarm_handler(int signum);
void pegar_opcao(WINDOW *window, int *opcao_usuario, int *opcao_anterior, float *histerese, float *TE, float *TR);
void mostrar_temperaturas(WINDOW *window, const float *histerese, const float *TI, const float *TE, const float *TR);
void gerar_log_csv(WINDOW *window, const float *TI, const float *TE, const float *TR);
void comunicar_uart(WINDOW *window, float *TI, float *TR, const int *opcao_usuario);
void usar_gpio(WINDOW *window, const float *TI, const float *TR, const float *histerese);
void usar_i2c(WINDOW *window, const float *TI, float *TE, const float *TR);

#endif // PARALELOS_HPP