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
void tratar_opcao_usuario(WINDOW *entrada);
void mostrar_temperaturas(WINDOW *saida);
void gerar_log_csv(WINDOW *logs);
void comunicar_uart(WINDOW *logs);
void usar_gpio(WINDOW *logs);
void usar_LCD(WINDOW *logs);
void sensor_externo(WINDOW *logs);

#endif // PARALELOS_HPP
