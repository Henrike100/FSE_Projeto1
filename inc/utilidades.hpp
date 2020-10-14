#ifndef UTILIDADES_HPP
#define UTILIDADES_HPP

#include <utility>
#include <string>
#include "bme280.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

pair<string, string> transformar_temperaturas(const float TI, const float TE, const float TR);
int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr);
void user_delay_us(uint32_t period, void *intf_ptr);
int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr);

#endif // UTILIDADES_HPP
