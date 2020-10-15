#include "utilidades.hpp"

pair<string, string> transformar_temperaturas(const float TI, const float TE, const float TR) {
    const string linha1 = "TI:" + to_string(TI).substr(0, 4) + " TE:" + to_string(TE).substr(0, 4);
    const string linha2 = "TR:" + to_string(TR).substr(0, 4);

    return make_pair(linha1, linha2);
}

int8_t user_i2c_read(uint8_t reg_addr, uint8_t *data, uint32_t len, void *intf_ptr) {
    struct identifier id;

    id = *((struct identifier *)intf_ptr);

    write(id.fd, &reg_addr, 1);
    read(id.fd, data, len);

    return 0;
}

void user_delay_us(uint32_t period, void *intf_ptr) {
    usleep(period);
}

int8_t user_i2c_write(uint8_t reg_addr, const uint8_t *data, uint32_t len, void *intf_ptr) {
    return BME280_OK;
}
