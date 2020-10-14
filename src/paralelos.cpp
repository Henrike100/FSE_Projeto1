#include "paralelos.hpp"

mutex mtx_alarm_print; int contador_alarm_print = 0;
mutex mtx_alarm_csv;   int contador_alarm_csv = 0;
mutex mtx_alarm_uart;  int contador_alarm_uart = 0;
mutex mtx_alarm_gpio;  int contador_alarm_gpio = 0;
mutex mtx_alarm_i2c;

condition_variable cv;
mutex mtx_incrementa_variavel;
int qtd_dispositivos_funcionando = 0;
int qtd_dispositivos_verificados = 0;

mutex mtx_main;
mutex mtx_csv;
mutex mtx_uart;
mutex mtx_i2c;
mutex mtx_gpio;
mutex mtx_print;

int status_programa = -1;

float TR;
float TI;
float TE;
float histerese = -1.0f;
int opcao_usuario = 0;

struct identifier {
    uint8_t dev_addr;
    int8_t fd;
};

void signal_handler(int signum) {
    status_programa = ENCERRAMENTO_VIA_SIGNAL;
}

void alarm_handler(int signum) {
    contador_alarm_print++;
    contador_alarm_csv++;
    contador_alarm_uart++;
    contador_alarm_gpio++;

    // printa a cada 500ms
    if(contador_alarm_print == 5) {
        contador_alarm_print = 0;
        mtx_alarm_print.unlock();
    }

    // escreve no CSV a cada 2000ms
    if(contador_alarm_csv == 20) {
        contador_alarm_csv = 0;
        mtx_alarm_csv.unlock();
    }

    // lê UART a cada 500ms
    if(contador_alarm_uart == 5) {
        contador_alarm_uart = 0;
        mtx_alarm_uart.unlock();
    }

    // verifica GPIO a cada 500ms
    if(contador_alarm_gpio == 5) {
        contador_alarm_gpio = 0;
        mtx_alarm_gpio.unlock();
    }

    // usa i2c a cada 100ms
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
            status_programa = ENCERRAMENTO_COM_ERRO_INICIO;
        }
        else {
            status_programa = ESPERANDO_PRIM_ENTRADA_USUARIO;
        }

        cv.notify_all();
    }
    
    mtx_incrementa_variavel.unlock();
}

void pegar_opcao(WINDOW *window) {
    mtx_interface.lock();
    int last_line = getmaxy(window);
    mtx_interface.unlock();

    int opcao;

    do {
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
            mvwscanw(window, 11, 18, " %d", &opcao);

            invalid = opcao < 0 || opcao > 3;
        } while (invalid);

        if(status_programa == ENCERRAMENTO_VIA_SIGNAL)
            break;

        mtx_opcao.lock();
        opcao_usuario = opcao;
        mtx_opcao.unlock();

        atualizar_menu(window, opcao_usuario, histerese);

        switch (opcao_usuario) {
        case 0:
            status_programa = ENCERRAMENTO_VIA_USER;
        case 1:
            pegar_temperatura(window, TE, &TR);
            break;
        case 3:
            pegar_histerese(window, opcao_usuario, &histerese);
            break;
        default:
            break;
        }

        if(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO) {
            pegar_histerese(window, opcao_usuario, &histerese);
            status_programa = EM_EXECUCAO;
            cv.notify_all();
        }
    } while(status_programa == EM_EXECUCAO);
}

void mostrar_temperaturas(WINDOW *window) {
    mtx_interface.lock();
    const int line_size = getmaxx(window);
    mtx_interface.unlock();
    
    const string spaces(((line_size/3)-5)/2, ' ');

    unique_lock<mutex> lck(mtx_print);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_funcionando != qtd_dispositivos_verificados) {
        return;
    }

    while(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO)
        cv.wait(lck);

    while(status_programa == EM_EXECUCAO) {
        mtx_alarm_print.lock();

        mtx_interface.lock();

        mtx_TI.lock();
        mvwprintw(window, 3, 1, "%s%.1f%s", spaces.c_str(), TI, spaces.c_str());
        mtx_TI.unlock();

        mvwvline(window, 3, line_size/3, 0, 1);

        mtx_TE.lock();
        mvwprintw(window, 3, line_size/3+1, "%s%.1f%s", spaces.c_str(), TE, spaces.c_str());
        mtx_TE.unlock();

        mvwvline(window, 3, 2*line_size/3, 0, 1);

        mtx_TR.lock();
        mvwprintw(window, 3, 2*line_size/3+1, "%s%.1f%s", spaces.c_str(), TR, spaces.c_str());
        mtx_TR.unlock();

        box(window, 0, 0);
        wrefresh(window);
        mtx_interface.unlock();
    }
}

void gerar_log_csv(WINDOW *window) {
    FILE *file;
    file = fopen("arquivo.csv", "w+");
    int status = INICIANDO;

    if(file == NULL) {
        atualizar_logs(window, CSV, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        return;
    }
    
    atualizar_logs(window, CSV, FUNCIONANDO);
    status = FUNCIONANDO;
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

    while(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO)
        cv.wait(lck);

    while(status_programa == EM_EXECUCAO) {
        mtx_alarm_csv.lock();
        time_t now = time(0);
        tm *ltm = localtime(&now);

        mtx_TI.lock();
        mtx_TE.lock();
        mtx_TR.lock();
        int num_escritos = fprintf(file, "%02d/%02d/%d %02d:%02d:%02d, %.1f, %.1f, %.1f\n",
            ltm->tm_mday,
            ltm->tm_mon+1,
            ltm->tm_year+1900,
            ltm->tm_hour+1,
            ltm->tm_min+1,
            ltm->tm_sec+1,
            TI, TE, TR
        );
        mtx_TI.unlock();
        mtx_TE.unlock();
        mtx_TR.unlock();

        if(num_escritos != 38 && status != ERRO_AO_ESCREVER) {
            atualizar_logs(window, CSV, ERRO_AO_ESCREVER);
            status = ERRO_AO_ESCREVER;
        }
    }

    fclose(file);
    atualizar_logs(window, CSV, ENCERRADO);
}

void comunicar_uart(WINDOW *window) {
    int uart0_filestream = -1;
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    int status_interno = INICIANDO, status_referencia = INICIANDO;
    
    if (uart0_filestream == -1) {
        atualizar_logs(window, SENSOR_INTERNO, ERRO_AO_ABRIR);
        atualizar_logs(window, TEMPERATURA_REFERENCIA, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        return;
    }

    atualizar_logs(window, SENSOR_INTERNO, FUNCIONANDO);
    atualizar_logs(window, TEMPERATURA_REFERENCIA, FUNCIONANDO);
    status_interno = FUNCIONANDO;
    status_referencia = FUNCIONANDO;
    incrementar_disp_funcionando(true);

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
    float temperatura_coletada;

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

    while(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO)
        cv.wait(lck);

    while(status_programa == EM_EXECUCAO) {
        mtx_alarm_uart.lock();
        
        // Pede temperatura interna
        count = write(uart0_filestream, &pede_TI[0], 5);

        if(count < 0) {
            if(status_interno != ERRO_AO_SOLICITAR_TEMP) {
                atualizar_logs(window, SENSOR_INTERNO, ERRO_AO_SOLICITAR_TEMP);
                status_interno = ERRO_AO_SOLICITAR_TEMP;
            }
        }
        else {
            if(status_interno != FUNCIONANDO) {
                atualizar_logs(window, SENSOR_INTERNO, FUNCIONANDO);
                status_interno = FUNCIONANDO;
            }

            // Recebe temepratura interna
            count = read(uart0_filestream, &temperatura_coletada, 4);

            if(count < 0) {
                if(status_interno != ERRO_AO_LER_TEMP) {
                    atualizar_logs(window, SENSOR_INTERNO, ERRO_AO_LER_TEMP);
                    status_interno = ERRO_AO_LER_TEMP;
                }
            }
            else if(count == 0) {
                if(status_interno != SEM_DADO_DISPONIVEL) {
                    atualizar_logs(window, SENSOR_INTERNO, SEM_DADO_DISPONIVEL);
                    status_interno = SEM_DADO_DISPONIVEL;
                }
            }
            else {
                mtx_TI.lock();
                TI = temperatura_coletada;
                mtx_TI.unlock();
                if(status_interno != FUNCIONANDO) {
                    atualizar_logs(window, SENSOR_INTERNO, FUNCIONANDO);
                    status_interno = FUNCIONANDO;
                }
            }
        }

        // opcao 2 -> Pegar temperatura pelo potenciometro
        mtx_opcao.lock();
        if(opcao_usuario == 2) {
            // Pede temperatura de referencia
            count = write(uart0_filestream, &pede_TR[0], 5);

            if(count < 0) {
                if(status_referencia != ERRO_AO_SOLICITAR_TEMP) {
                    atualizar_logs(window, TEMPERATURA_REFERENCIA, ERRO_AO_SOLICITAR_TEMP);
                    status_referencia = ERRO_AO_SOLICITAR_TEMP;
                }
            }
            else {
                if(status_referencia != FUNCIONANDO) {
                    atualizar_logs(window, TEMPERATURA_REFERENCIA, FUNCIONANDO);
                    status_referencia = FUNCIONANDO;
                }

                // Recebe temepratura de referencia
                count = read(uart0_filestream, &temperatura_coletada, 4);

                if(count < 0) {
                    if(status_referencia != ERRO_AO_LER_TEMP) {
                        atualizar_logs(window, TEMPERATURA_REFERENCIA, ERRO_AO_LER_TEMP);
                        status_referencia = ERRO_AO_LER_TEMP;
                    }
                }
                else if(count == 0) {
                    if(status_referencia != SEM_DADO_DISPONIVEL) {
                        atualizar_logs(window, TEMPERATURA_REFERENCIA, SEM_DADO_DISPONIVEL);
                        status_referencia = SEM_DADO_DISPONIVEL;
                    }
                }
                else {
                    if(status_referencia != FUNCIONANDO) {
                        atualizar_logs(window, TEMPERATURA_REFERENCIA, FUNCIONANDO);
                        status_referencia = FUNCIONANDO;
                    }

                    mtx_TR.lock();
                    TR = temperatura_coletada;
                    mtx_TR.unlock();
                }
            }
        }
        mtx_opcao.unlock();
    }

    close(uart0_filestream);
    atualizar_logs(window, SENSOR_INTERNO, ENCERRADO);
    atualizar_logs(window, TEMPERATURA_REFERENCIA, ENCERRADO);
}

void usar_gpio(WINDOW *window) {
    if (!bcm2835_init()) {
        atualizar_logs(window, RESISTOR, ERRO_AO_ABRIR);
        atualizar_logs(window, VENTOINHA, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        return;
    }

    atualizar_logs(window, RESISTOR, FUNCIONANDO);
    atualizar_logs(window, VENTOINHA, FUNCIONANDO);
    incrementar_disp_funcionando(true);

    bcm2835_gpio_fsel(PINO_RESISTOR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(PINO_VENTOINHA, BCM2835_GPIO_FSEL_OUTP);

    // Só continua depois de verificar todos os dispositivos
    unique_lock<mutex> lck(mtx_gpio);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_verificados != qtd_dispositivos_funcionando) {
        atualizar_logs(window, RESISTOR, ENCERRADO);
        atualizar_logs(window, VENTOINHA, ENCERRADO);
        return;
    }

    bcm2835_gpio_write(PINO_RESISTOR, 0);
    int status_resistor = FUNCIONANDO_LIGADO;

    bcm2835_gpio_write(PINO_VENTOINHA, 1);
    int status_ventoinha = FUNCIONANDO_DESLIGADO;

    while(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO)
        cv.wait(lck);

    while(status_programa == EM_EXECUCAO) {
        mtx_alarm_gpio.lock();

        mtx_TR.lock(); mtx_histerese.lock();
        double minimo = TR - (histerese/2), maximo = TR + (histerese/2);
        mtx_histerese.unlock(); mtx_TR.unlock();

        mtx_TI.lock();
        if(TI < minimo) {
            if(status_resistor == FUNCIONANDO_DESLIGADO) {
                status_resistor = FUNCIONANDO_LIGADO;
                bcm2835_gpio_write(PINO_RESISTOR, 0);
                atualizar_logs(window, RESISTOR, FUNCIONANDO_LIGADO);

                status_ventoinha = FUNCIONANDO_DESLIGADO;
                bcm2835_gpio_write(PINO_VENTOINHA, 1);
                atualizar_logs(window, VENTOINHA, FUNCIONANDO_DESLIGADO);
            }
        }
        else if(TI > maximo) {
            if(status_ventoinha == FUNCIONANDO_DESLIGADO) {
                status_ventoinha = FUNCIONANDO_LIGADO;
                bcm2835_gpio_write(PINO_VENTOINHA, 0);
                atualizar_logs(window, VENTOINHA, FUNCIONANDO_LIGADO);

                status_resistor = FUNCIONANDO_DESLIGADO;
                bcm2835_gpio_write(PINO_RESISTOR, 1);
                atualizar_logs(window, RESISTOR, FUNCIONANDO_DESLIGADO);
            }
        }
        mtx_TI.unlock();
    }

    bcm2835_gpio_write(PINO_RESISTOR, 1);
    bcm2835_gpio_write(PINO_VENTOINHA, 1);

    bcm2835_close();
    atualizar_logs(window, RESISTOR, ENCERRADO);
    atualizar_logs(window, VENTOINHA, ENCERRADO);
}

void usar_i2c(WINDOW *window) {
    struct bme280_dev dev;
    struct identifier id;
    const char path[] = "/dev/i2c-1";
    int status_sensor = INICIANDO, status_LCD = INICIANDO;

    id.fd = open(path, O_RDWR);

    if(id.fd < 0) {
        // Failed to open the i2c bus
        atualizar_logs(window, SENSOR_EXTERNO, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        return;
    }

    id.dev_addr = ENDERECO_SENSOR_EXTERNO;
    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
        // Failed to acquire bus access and/or talk to slave
        atualizar_logs(window, SENSOR_EXTERNO, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        close(id.fd);
        return;
    }

    dev.intf = BME280_I2C_INTF;
    dev.intf_ptr = &id;

    int rslt = bme280_init(&dev);
    if(rslt != BME280_OK) {
        // Failed to initialize the device
        atualizar_logs(window, SENSOR_EXTERNO, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        return;
    }

    uint8_t settings_sel = 0;
    struct bme280_data comp_data;

    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    rslt = bme280_set_sensor_settings(settings_sel, &dev);
    if(rslt != BME280_OK) {
        // Failed to set sensor settings
        atualizar_logs(window, SENSOR_EXTERNO, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        return;
    }

    atualizar_logs(window, SENSOR_EXTERNO, FUNCIONANDO);
    status_sensor = FUNCIONANDO;

    if (wiringPiSetup() == -1) {
        atualizar_logs(window, LCD, ERRO_AO_ABRIR);
        incrementar_disp_funcionando(false);
        close(id.fd);
        return;
    }

    int fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init(fd);

    atualizar_logs(window, LCD, FUNCIONANDO);
    incrementar_disp_funcionando(true);
    status_LCD = FUNCIONANDO;

    unique_lock<mutex> lck(mtx_i2c);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_verificados != qtd_dispositivos_funcionando) {
        close(id.fd);
        atualizar_logs(window, SENSOR_EXTERNO, ENCERRADO);
        atualizar_logs(window, LCD, ENCERRADO);
        return;
    }

    while(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO)
        cv.wait(lck);

    while(status_programa == EM_EXECUCAO) {
        mtx_alarm_i2c.lock();

        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
        if (rslt != BME280_OK) {
            // Failed to set sensor mode
            if(status_sensor != ERRO_SENSOR_MODE) {
                atualizar_logs(window, SENSOR_EXTERNO, ERRO_SENSOR_MODE);
                status_sensor = ERRO_SENSOR_MODE;
            }
        }
        else {
            rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
            if (rslt != BME280_OK) {
                // Failed to get sensor data
                if(status_sensor != ERRO_AO_LER_TEMP) {
                    atualizar_logs(window, SENSOR_EXTERNO, ERRO_AO_LER_TEMP);
                    status_sensor = ERRO_AO_LER_TEMP;
                }
            }
            else {
                mtx_TE.lock();
                TE = comp_data.temperature;
                mtx_TE.unlock();
                if(status_sensor != FUNCIONANDO) {
                    atualizar_logs(window, SENSOR_EXTERNO, FUNCIONANDO);
                    status_sensor = FUNCIONANDO;
                }
            }
        }

        pair<string, string> linha1_linha2 = transformar_temperaturas(TI, TE, TR);

        ClrLcd(fd);
        lcdLoc(fd, LINE1);
        typeln(fd, linha1_linha2.first.c_str());
        lcdLoc(fd, LINE2);
        typeln(fd, linha1_linha2.second.c_str());

        if(status_LCD != FUNCIONANDO) {
            status_LCD = FUNCIONANDO;
            atualizar_logs(window, LCD, FUNCIONANDO);
        }
    }

    close(id.fd);
    atualizar_logs(window, SENSOR_EXTERNO, ENCERRADO);
    atualizar_logs(window, LCD, ENCERRADO);
}
