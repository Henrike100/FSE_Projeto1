#include "paralelos.hpp"

const char path[] = "/dev/i2c-1";

mutex mtx_alarm_print; int contador_alarm_print = 0;
mutex mtx_alarm_csv;   int contador_alarm_csv = 0;
mutex mtx_alarm_uart;  int contador_alarm_uart = 0;
mutex mtx_alarm_i2c;

condition_variable cv;
mutex mtx_incrementa_variavel;
int qtd_dispositivos_funcionando = 0;
int qtd_dispositivos_verificados = 0;

mutex mtx_main;
mutex mtx_csv;
mutex mtx_uart;

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

void alarm_handler(int signum) {
    contador_alarm_print++;
    contador_alarm_csv++;
    contador_alarm_uart++;

    if(contador_alarm_print == 5) {
        contador_alarm_print = 0;
        mtx_alarm_print.unlock();
    }

    if(contador_alarm_csv == 20) {
        contador_alarm_csv = 0;
        mtx_alarm_csv.unlock();
    }

    if(contador_alarm_uart == 5) {
        contador_alarm_uart = 0;
        mtx_alarm_uart.unlock();
    }

    mtx_alarm_i2c.unlock();

    ualarm(100000, 0);
}

void incrementar_disp_funcionando(bool funcionando) {
    mtx_incrementa_variavel.lock();

    qtd_dispositivos_verificados++;

    if(funcionando)
        qtd_dispositivos_funcionando++;

    if(qtd_dispositivos_verificados == NUM_DISPOSITIVOS) {
        if(qtd_dispositivos_funcionando != qtd_dispositivos_verificados) {
            programa_pode_continuar = false;
        }

        cv.notify_all();
    }
    
    mtx_incrementa_variavel.unlock();
}

void pegar_opcao(WINDOW *window, int *opcao_usuario, int *opcao_anterior, float *histerese, float *TE, float *TR) {
    mtx_interface.lock();
    int last_line = getmaxy(window);
    mtx_interface.unlock();

    do {
        if(*opcao_usuario != 3)
            *opcao_anterior = *opcao_usuario;

        bool invalid = false;

        do {
            mtx_interface.lock();
            wmove(window, 11, 1);
            wclrtoeol(window);
            box(window, 0, 0);
            if(invalid) {
                mvwprintw(window, last_line-2, 1, "Escolha deve estar entre 0 e 3");
            }

            wrefresh(window);

            mvwprintw(window, 11, 1, "Escolha a opcao: ");
            mtx_interface.unlock();
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

void mostrar_temperaturas(WINDOW *window, const float *histerese, const float *TI, const float *TE, const float *TR) {
    mtx_interface.lock();
    const int line_size = getmaxx(window);
    mtx_interface.unlock();
    
    const string spaces(((line_size/3)-5)/2, ' ');

    while(programa_pode_continuar) {
        mtx_alarm_print.lock();

        if(*histerese < 0)
            continue;

        mtx_interface.lock();
        mvwprintw(window, 3, 1, "%s%.1f%s", spaces.c_str(), *TI, spaces.c_str());
        mvwvline(window, 3, line_size/3, 0, 1);
        mvwprintw(window, 3, line_size/3+1, "%s%.1f%s", spaces.c_str(), *TE, spaces.c_str());
        mvwvline(window, 3, 2*line_size/3, 0, 1);
        mvwprintw(window, 3, 2*line_size/3+1, "%s%.1f%s", spaces.c_str(), *TR, spaces.c_str());

        box(window, 0, 0);
        wrefresh(window);
        mtx_interface.unlock();
    }
}

void gerar_log_csv(WINDOW *window, float *TI, float *TE, float *TR) {
    FILE *file;
    file = fopen("arquivo.csv", "w+");

    if(file == NULL) {
        atualizar_logs(window, CSV, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        return;
    }
    
    atualizar_logs(window, CSV, FUNCIONANDO);
    incrementar_disp_funcionando(true);

    // Só continua depois de verificar todos os dispositivos
    unique_lock<mutex> lck(mtx_csv);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_funcionando != qtd_dispositivos_verificados) {
        fclose(file);
        atualizar_logs(window, CSV, ENCERRADO);
        return;
    }

    fprintf(file, "Data/Hora, Temperatura Interna, Temperatura Externa, Temperatura de Referência\n");

    while(programa_pode_continuar) {
        mtx_alarm_csv.lock();
        time_t now = time(0);
        tm *ltm = localtime(&now);
        int num_escritos = fprintf(file, "%02d/%02d/%d %02d:%02d:%02d, %.1f, %.1f, %.1f\n",
            ltm->tm_mday,
            ltm->tm_mon+1,
            ltm->tm_year+1900,
            ltm->tm_hour+1,
            ltm->tm_min+1,
            ltm->tm_sec+1,
            *TI, *TE, *TR
        );

        if(num_escritos != 38)
            atualizar_logs(window, CSV, ERRO_AO_ESCREVER);
    }

    fclose(file);
    atualizar_logs(window, CSV, ENCERRADO);
}

void comunicar_uart(WINDOW *window, float *TI, float *TR, const int *opcao_usuario) {
    int uart0_filestream = -1;
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    
    if (uart0_filestream == -1) {
        atualizar_logs(window, SENSOR_INTERNO, ERRO_AO_ABRIR);
        atualizar_logs(window, TEMPERATURA_REFERENCIA, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        return;
    }

    atualizar_logs(window, SENSOR_INTERNO, FUNCIONANDO);
    atualizar_logs(window, TEMPERATURA_REFERENCIA, FUNCIONANDO);
    incrementar_disp_funcionando(true);

    // Só continua depois de verificar todos os dispositivos
    unique_lock<mutex> lck(mtx_uart);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_verificados != qtd_dispositivos_funcionando) {
        close(uart0_filestream);
        atualizar_logs(window, SENSOR_INTERNO, ENCERRADO);
        atualizar_logs(window, TEMPERATURA_REFERENCIA, ENCERRADO);
        return;
    }

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
        mtx_alarm_uart.lock();
        
        // Pede temperatura interna
        count = write(uart0_filestream, &pede_TI[0], 5);

        if(count < 0) {
            //atualizar_logs(window, "UART", ERRO_DE_SOLICITACAO);
        }
        else {
            atualizar_logs(window, SENSOR_INTERNO, FUNCIONANDO);
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
            atualizar_logs(window, SENSOR_INTERNO, FUNCIONANDO);
        }

        if(*opcao_usuario == 2) {
            // Pede temperatura de referencia
            count = write(uart0_filestream, &pede_TR[0], 5);

            if(count < 0) {
                //atualizar_logs(window, "UART", ERRO_DE_SOLICITACAO);
            }
            else {
                atualizar_logs(window, TEMPERATURA_REFERENCIA, FUNCIONANDO);
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
                atualizar_logs(window, TEMPERATURA_REFERENCIA, FUNCIONANDO);
            }
        }
    }

    close(uart0_filestream);
    atualizar_logs(window, SENSOR_INTERNO, ENCERRADO);
    atualizar_logs(window, TEMPERATURA_REFERENCIA, ENCERRADO);
}

void usar_gpio(WINDOW *window, const float *TI, const float *TR, const float *histerese) {

}

void usar_i2c(WINDOW *window, const float *TI, float *TE, const float *TR) {
    struct bme280_dev dev;
    struct identifier id;
    id.fd = open(path, O_RDWR);

    if(id.fd < 0) {
        // Failed to open the i2c bus
        atualizar_logs(window, SENSOR_EXTERNO, ERRO_AO_ABRIR);
        atualizar_logs(window, LCD, ERRO_AO_ABRIR);
        return;
    }

    id.dev_addr = ENDERECO_SENSOR_EXTERNO;
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
        mtx_alarm_i2c.lock();

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
    }
}
