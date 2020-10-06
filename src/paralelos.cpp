#include "paralelos.hpp"

const char path[] = "/dev/i2c-1";

struct identifier {
    /* Variable to hold device address */
    uint8_t dev_addr;

    /* Variable that contains file descriptor */
    int8_t fd;
};

bool programa_pode_continuar = true;

void signal_handler(int signum) {
    programa_pode_continuar = false;
    printf("Parando\n");
}

void ler_potenciometro(float *TR) {
    const char codigo = 0xA2;
    char padrao[] = {codigo, 0, 3, 9, 4};
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);      //Open in non blocking read/write mode
    if (uart0_filestream == -1) {
        printf("Erro - Não foi possível iniciar a UART.\n");
        programa_pode_continuar = false;
    }

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    while(programa_pode_continuar) {
        write(uart0_filestream, &padrao[0], 5);
        usleep(100000);
        read(uart0_filestream, TR, 4);
    }

    close(uart0_filestream);
}

void gerar_log_csv(float *TI, float *TE, float *TR) {
    FILE *file;
    file = fopen("arquivo.csv", "w+");
    fprintf(file, "Data/Hora, Temperatura Interna, Temperatura Externa, Temperatura de Referência\n");

    while(programa_pode_continuar) {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        fprintf(file, "%02d/%02d/%d %02d:%02d:%02d, %.1f, %.1f, %.1f\n",
            ltm->tm_mday,
            ltm->tm_mon+1,
            ltm->tm_year+1900,
            ltm->tm_hour+1,
            ltm->tm_min+1,
            ltm->tm_sec+1,
            *TI, *TE, *TR
        );
        usleep(2000000);
    }

    fclose(file);
}

struct bme280_dev criar_dev(uint8_t addr) {
    struct bme280_dev dev;
    struct identifier id;

    id.fd = open(path, O_RDWR);
    id.dev_addr = addr;
    dev.intf = BME280_I2C_INTF;
    dev.intf_ptr = &id;

    return dev;
}

void atualizar_temperaturas(float *TI, float *TE) {
    struct bme280_dev dev_interno = criar_dev(SENSOR_INTERNO), dev_externo = criar_dev(SENSOR_EXTERNO);

    bme280_init(&dev_interno);
    bme280_init(&dev_externo);

    struct bme280_data comp_data_int, comp_data_ext;

    dev_interno.settings.osr_h = dev_externo.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev_interno.settings.osr_h = dev_externo.settings.osr_h = BME280_OVERSAMPLING_16X;
    dev_interno.settings.osr_h = dev_externo.settings.osr_h = BME280_OVERSAMPLING_2X;
    dev_interno.settings.osr_h = dev_externo.settings.osr_h = BME280_FILTER_COEFF_16;

    uint8_t settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    bme280_set_sensor_settings(settings_sel, &dev_interno);
    bme280_set_sensor_settings(settings_sel, &dev_externo);

    while(programa_pode_continuar) {
        bme280_set_sensor_mode(BME280_FORCED_MODE, &dev_interno);
        bme280_set_sensor_mode(BME280_FORCED_MODE, &dev_externo);

        bme280_get_sensor_data(BME280_ALL, &comp_data_int, &dev_interno);
        bme280_get_sensor_data(BME280_ALL, &comp_data_ext, &dev_externo);

        *TI = comp_data_int.temperature;
        *TE = comp_data_ext.temperature;

        usleep(100000);
    }
}

void apresentar_temperaturas(float *TI, float *TE, float *TR) {
    printf("-------------------------------------------------------------------------\n");
    printf("| Temperatura Interna | Temperatura Externa | Temperatura de Referência |\n");

    while(programa_pode_continuar) {
        printf("-------------------------------------------------------------------------\n");
        printf("|         %.1lf        |         %.1lf        |           %.1lf            |\n", *TI, *TE, *TR);
        usleep(500000);
    }
}

void atualizar_lcd() {
    while(programa_pode_continuar) {
        //printf("Atualiza LCD a cada 1s\n");
        usleep(1000000);
    }
}
