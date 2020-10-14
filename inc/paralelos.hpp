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
#include "gpio.hpp"

extern mutex mtx_main;
extern condition_variable cv;
extern int qtd_dispositivos_funcionando;
extern int qtd_dispositivos_verificados;
extern int status_programa;

void signal_handler(int signum);
void alarm_handler(int signum);
void pegar_opcao(WINDOW *window);
void mostrar_temperaturas(WINDOW *window);
void gerar_log_csv(WINDOW *window);
void comunicar_uart(WINDOW *window);
void usar_gpio(WINDOW *window);
void usar_i2c(WINDOW *window);

#endif // PARALELOS_HPP
