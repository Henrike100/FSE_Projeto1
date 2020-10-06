#ifndef PARALELOS_HPP
#define PARALELOS_HPP

#include <signal.h>
#include <stdio.h>
#include <ctime>
#include <string.h>
#include <sys/types.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <stdlib.h>
#include "bme280.hpp"

void signal_handler(int signum);
void ler_potenciometro(float *TR);
void gerar_log_csv(float *TI, float *TE, float *TR);
void atualizar_temperaturas(float *TI, float *TE);
void apresentar_temperaturas(float *TI, float *TE, float *TR);
void atualizar_lcd();

#endif // PARALELOS_HPP
