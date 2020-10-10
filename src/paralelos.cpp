#include "paralelos.hpp"

const char path[] = "/dev/i2c-1";

int uart = INICIANDO;
int csv  = INICIANDO;

struct identifier {
    /* Variable to hold device address */
    uint8_t dev_addr;

    /* Variable that contains file descriptor */
    int8_t fd;
};

bool programa_pode_continuar = true;

void signal_handler(int signum) {
    programa_pode_continuar = false;
}

void pegar_opcao(WINDOW *window, int *opcao_usuario, int *opcao_anterior, float *histerese, float *TE, float *TR) {
    int last_line = getmaxy(window);

    do {
        if(*opcao_usuario != 3)
            *opcao_anterior = *opcao_usuario;

        bool invalid = false;

        do {
            wmove(window, 11, 1);
            wclrtoeol(window);
            box(window, 0, 0);
            if(invalid) {
                mvwprintw(window, last_line-2, 1, "Escolha deve estar entre 0 e 3");
            }

            wrefresh(window);

            mvwprintw(window, 11, 1, "Escolha a opcao: ");
            mvwscanw(window, 11, 18, " %d", opcao_usuario);

            invalid = *opcao_usuario < 0 || *opcao_usuario > 3;
        } while (invalid);

        if(!(*opcao_usuario)) break;

        atualizar_menu(window, *opcao_usuario, *opcao_anterior, *histerese);

        switch (*opcao_usuario) {
        case 1:
            pegar_temperatura(window, *TE, TR);
            break;
        case 2:
            
            break;
        case 3:
            pegar_histerese(window, *opcao_usuario, *opcao_anterior, histerese);
            break;
        default:
            break;
        }

        if(*histerese < 0.0f) {
            pegar_histerese(window, *opcao_usuario, *opcao_anterior, histerese);
        }
    } while(programa_pode_continuar);

    programa_pode_continuar = false;
}

void mostrar_temperaturas(WINDOW *window, const int *opcao_usuario, const float *histerese,
                          const float *TI, const float *TE, const float *TR) {
    const int line_size = getmaxx(window);
    
    const string spaces(((line_size/3)-5)/2, ' ');

    while(programa_pode_continuar) {
        if(*opcao_usuario == 0 or *histerese == -1) {
            usleep(100000);
            continue;
        }
        mvwprintw(window, 3, 1, "%s%.1f%s", spaces.c_str(), *TI, spaces.c_str());
        mvwvline(window, 3, line_size/3, 0, 1);
        mvwprintw(window, 3, line_size/3+1, "%s%.1f%s", spaces.c_str(), *TE, spaces.c_str());
        mvwvline(window, 3, 2*line_size/3, 0, 1);
        mvwprintw(window, 3, 2*line_size/3+1, "%s%.1f%s", spaces.c_str(), *TR, spaces.c_str());

        box(window, 0, 0);
        wrefresh(window);
        usleep(500000);
    }
}

void gerar_log_csv(WINDOW *window, float *TI, float *TE, float *TR) {
    FILE *file;
    int tentativas = 0;

    do {
        file = fopen("arquivo.csv", "w+");
        if(file == NULL) {
            tentativas++;
            usleep(1000000);
        }
        else
            break;
    } while (tentativas < MAX_TENTATIVAS);
    
    if(file == NULL) {
        csv = ERRO_AO_ABRIR;
        atualizar_logs(window, "CSV", ERRO_AO_ABRIR);
        programa_pode_continuar = false;
        return;
    }
    
    csv = FUNCIONANDO;
    atualizar_logs(window, "CSV", FUNCIONANDO);

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
    atualizar_logs(window, "CSV", ENCERRADO);
}

void comunicar_uart(WINDOW *window, float *TI, float *TR) {
    int uart0_filestream = -1;
    int tentativas = 0;
    do {
        if (uart0_filestream == -1) {
            tentativas++;
            usleep(1000000);
        }
        else
            break;
    } while (tentativas < MAX_TENTATIVAS);
    
    if (uart0_filestream == -1) {
        uart = ERRO_AO_ABRIR;
        atualizar_logs(window, "UART", ERRO_AO_ABRIR);
        //programa_pode_continuar = false;
        return;
    }
    
    uart = FUNCIONANDO;
    atualizar_logs(window, "UART", FUNCIONANDO);

    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    char pede_TI[] = {(char)0xA1, 0, 3, 9, 4};
    char pede_TR[] = {(char)0xA2, 0, 3, 9, 4};

    int count;

    while(programa_pode_continuar) {
        // Pede temperatura interna
        count = write(uart0_filestream, &pede_TI[0], 5);

        if(count < 0) {
            //atualizar_logs(window, "UART", ERRO_DE_SOLICITACAO);
        }
        else {
            atualizar_logs(window, "UART", FUNCIONANDO);
        }

        // Recebe temepratura interna
        count = read(uart0_filestream, TI, 4);

        if(count < 0) {
            //atualizar_logs(window, "UART", ERRO_DE_SOLICITACAO);
        }
        else if(count == 0) {
            //atualizar_logs(window, "UART", ERRO_DE_SOLICITACAO);
        }
        else {
            atualizar_logs(window, "UART", FUNCIONANDO);
        }

        // Pede temperatura de referencia
        count = write(uart0_filestream, &pede_TR[0], 5);

        if(count < 0) {
            //atualizar_logs(window, "UART", ERRO_DE_SOLICITACAO);
        }
        else {
            atualizar_logs(window, "UART", FUNCIONANDO);
        }

        // Recebe temepratura referencia
        count = read(uart0_filestream, TR, 4);

        if(count < 0) {
            //atualizar_logs(window, "UART", ERRO_DE_SOLICITACAO);
        }
        else if(count == 0) {
            //atualizar_logs(window, "UART", ERRO_DE_SOLICITACAO);
        }
        else {
            atualizar_logs(window, "UART", FUNCIONANDO);
        }

        usleep(100000);
    }

    close(uart0_filestream);
    atualizar_logs(window, "UART", ENCERRADO);
}

void usar_gpio(WINDOW *window, const float *TI, const float *TR, const float *histerese) {

}

void usar_i2c(WINDOW *window, const float *TI, float *TE, const float *TR) {
    struct bme280_dev dev;
    struct identifier id;
    const char path[] = "/dev/i2c-1";

    if((id.fd = open(path, O_RDWR)) < 0) {
        // Failed to open the i2c bus
    }

    id.dev_addr = SENSOR_EXTERNO;
    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
        // Failed to acquire bus access and/or talk to slave
    }

    dev.intf = BME280_I2C_INTF;
    dev.intf_ptr = &id;

    int rslt = bme280_init(&dev);
    if(rslt != BME280_OK) {
        // Failed to initialize the device
    }

    uint8_t settings_sel = 0;
    struct bme280_data comp_data;

    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    while(programa_pode_continuar) {
        rslt = bme280_set_sensor_settings(settings_sel, &dev);
        if(rslt != BME280_OK) {
            // Failed to set sensor settings
        }

        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
        if (rslt != BME280_OK) {
            // Failed to set sensor mode
        }

        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
        if (rslt != BME280_OK) {
            // Failed to get sensor data
        }

        *TE = comp_data.temperature;

        // LCD

        usleep(100000);
    }
}
