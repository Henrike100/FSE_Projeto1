#include "paralelos.hpp"

int contador_alarm_print = 0;
int contador_alarm_csv = 0;
int contador_alarm_uart = 0;
int contador_alarm_gpio = 0;
int contador_alarm_LCD = 0;
int contador_alarm_SE = 0;

condition_variable cv;
mutex mtx_incrementa_variavel;
int qtd_dispositivos_funcionando = 0;
int qtd_dispositivos_verificados = 0;

mutex mtx_main;
mutex mtx_csv;
mutex mtx_uart;
mutex mtx_SE;
mutex mtx_gpio;
mutex mtx_print;
mutex mtx_LCD;

int status_programa = -1;

float TR = -1.0f;
float TI;
float TE;
float histerese = -1.0f;
int opcao_usuario = 0;

void signal_handler(int signum) {
    status_programa = ENCERRAMENTO_VIA_SIGNAL;
}

void alarm_handler(int signum) {
    contador_alarm_print++;
    contador_alarm_csv++;
    contador_alarm_uart++;
    contador_alarm_gpio++;
    contador_alarm_LCD++;
    contador_alarm_SE++;

    cv.notify_all();

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

void tratar_opcao_usuario(WINDOW *entrada) {
    do {
        pegar_opcao(entrada, &opcao_usuario);

        if(status_programa == ENCERRAMENTO_VIA_SIGNAL)
            break;

        atualizar_menu(entrada, opcao_usuario, histerese);

        switch (opcao_usuario) {
        case 0:
            status_programa = ENCERRAMENTO_VIA_USER;
            break;
        case 1:
            pegar_temperatura(entrada, &TE, &TR);
            break;
        case 3:
            pegar_histerese(entrada, opcao_usuario, &histerese);
            break;
        default:
            break;
        }

        if(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO) {
            pegar_histerese(entrada, opcao_usuario, &histerese);
            status_programa = EM_EXECUCAO;
            cv.notify_all();
        }
    } while(status_programa == EM_EXECUCAO);
}

void mostrar_temperaturas(WINDOW *saida) {
    mtx_interface.lock();
    const int line_size = getmaxx(saida);
    mtx_interface.unlock();
    
    const string spaces(((line_size/3)-5)/2, ' ');

    unique_lock<mutex> lck(mtx_print);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_funcionando != qtd_dispositivos_verificados) {
        return;
    }

    while(status_programa == EM_EXECUCAO or status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO) {
        while(contador_alarm_print < 5)
            cv.wait(lck);
        
        contador_alarm_print = 0;

        atualizar_saida(saida, TI, TE, TR);
    }
}

void gerar_log_csv(WINDOW *logs) {
    FILE *file;
    file = fopen("arquivo.csv", "w+");
    int status = INICIANDO;

    if(file == NULL) {
        atualizar_logs(logs, CSV, ERRO_AO_ABRIR, &status);
        incrementar_disp_funcionando(false);
        return;
    }
    
    atualizar_logs(logs, CSV, FUNCIONANDO, &status);
    incrementar_disp_funcionando(true);

    // Só continua depois de verificar todos os dispositivos
    unique_lock<mutex> lck(mtx_csv);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_funcionando != qtd_dispositivos_verificados) {
        fclose(file);
        atualizar_logs(logs, CSV, ENCERRADO, &status);
        return;
    }

    fprintf(file, "Data/Hora, Temperatura Interna, Temperatura Externa, Temperatura de Referência\n");

    while(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO)
        cv.wait(lck);

    while(status_programa == EM_EXECUCAO) {
        while(contador_alarm_csv < 20)
            cv.wait(lck);
        
        contador_alarm_csv = 0;

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
        mtx_TR.unlock();
        mtx_TE.unlock();
        mtx_TI.unlock();

        //if(num_escritos != 38) {
        if(num_escritos < 0) {
            atualizar_logs(logs, CSV, ERRO_AO_ESCREVER, &status);
        }
    }

    fclose(file);
    atualizar_logs(logs, CSV, ENCERRADO, &status);
}

void comunicar_uart(WINDOW *logs) {
    int uart0_filestream = -1;
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);
    int status_interno = INICIANDO, status_referencia = INICIANDO;
    
    if (uart0_filestream == -1) {
        atualizar_logs(logs, SENSOR_INTERNO, ERRO_AO_ABRIR, &status_interno);
        atualizar_logs(logs, TEMPERATURA_REFERENCIA, ERRO_AO_ABRIR, &status_referencia);
        incrementar_disp_funcionando(false);
        return;
    }

    atualizar_logs(logs, SENSOR_INTERNO, FUNCIONANDO, &status_interno);
    atualizar_logs(logs, TEMPERATURA_REFERENCIA, FUNCIONANDO, &status_referencia);
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
        atualizar_logs(logs, SENSOR_INTERNO, ENCERRADO, &status_interno);
        atualizar_logs(logs, TEMPERATURA_REFERENCIA, ENCERRADO, &status_referencia);
        return;
    }

    while(status_programa == EM_EXECUCAO or status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO) {
        while(contador_alarm_uart < 5)
            cv.wait(lck);
        
        contador_alarm_uart = 0;
        
        // Pede temperatura interna
        count = write(uart0_filestream, &pede_TI[0], 5);

        if(count < 0) {
            atualizar_logs(logs, SENSOR_INTERNO, ERRO_AO_SOLICITAR_TEMP, &status_interno);
        }
        else {
            atualizar_logs(logs, SENSOR_INTERNO, FUNCIONANDO, &status_interno);

            // Recebe temepratura interna
            count = read(uart0_filestream, &temperatura_coletada, 4);

            if(count < 0) {
                atualizar_logs(logs, SENSOR_INTERNO, ERRO_AO_LER_TEMP, &status_interno);
            }
            else if(count == 0) {
                atualizar_logs(logs, SENSOR_INTERNO, SEM_DADO_DISPONIVEL, &status_interno);
            }
            else {
                mtx_TI.lock();
                TI = temperatura_coletada;
                mtx_TI.unlock();
                atualizar_logs(logs, SENSOR_INTERNO, FUNCIONANDO, &status_interno);
            }
        }

        // opcao 2 -> Pegar temperatura pelo potenciometro
        mtx_opcao.lock();
        if(opcao_usuario == 2) {
            // Pede temperatura de referencia
            count = write(uart0_filestream, &pede_TR[0], 5);

            if(count < 0) {
                atualizar_logs(logs, TEMPERATURA_REFERENCIA, ERRO_AO_SOLICITAR_TEMP, &status_referencia);
            }
            else {
                atualizar_logs(logs, TEMPERATURA_REFERENCIA, FUNCIONANDO, &status_referencia);

                // Recebe temepratura de referencia
                count = read(uart0_filestream, &temperatura_coletada, 4);

                if(count < 0) {
                    atualizar_logs(logs, TEMPERATURA_REFERENCIA, ERRO_AO_LER_TEMP, &status_referencia);
                }
                else if(count == 0) {
                    atualizar_logs(logs, TEMPERATURA_REFERENCIA, SEM_DADO_DISPONIVEL, &status_referencia);
                }
                else {
                    mtx_TR.lock();
                    TR = temperatura_coletada;
                    mtx_TR.unlock();
                    atualizar_logs(logs, TEMPERATURA_REFERENCIA, FUNCIONANDO, &status_referencia);
                }
            }
        }
        mtx_opcao.unlock();
    }

    close(uart0_filestream);
    atualizar_logs(logs, SENSOR_INTERNO, ENCERRADO, &status_interno);
    atualizar_logs(logs, TEMPERATURA_REFERENCIA, ENCERRADO, &status_referencia);
}

void usar_gpio(WINDOW *logs) {
    int status_resistor = INICIANDO, status_ventoinha = INICIANDO;

    if (!bcm2835_init()) {
        atualizar_logs(logs, RESISTOR, ERRO_AO_ABRIR, &status_resistor);
        atualizar_logs(logs, VENTOINHA, ERRO_AO_ABRIR, &status_ventoinha);
        incrementar_disp_funcionando(false);
        return;
    }

    bcm2835_gpio_fsel(PINO_RESISTOR, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(PINO_VENTOINHA, BCM2835_GPIO_FSEL_OUTP);

    atualizar_logs(logs, RESISTOR, FUNCIONANDO, &status_resistor);
    atualizar_logs(logs, VENTOINHA, FUNCIONANDO, &status_ventoinha);
    incrementar_disp_funcionando(true);

    // Só continua depois de verificar todos os dispositivos
    unique_lock<mutex> lck(mtx_gpio);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_verificados != qtd_dispositivos_funcionando) {
        atualizar_logs(logs, RESISTOR, ENCERRADO, &status_resistor);
        atualizar_logs(logs, VENTOINHA, ENCERRADO, &status_ventoinha);
        return;
    }

    bcm2835_gpio_write(PINO_RESISTOR, 0);
    atualizar_logs(logs, RESISTOR, FUNCIONANDO_LIGADO, &status_resistor);

    bcm2835_gpio_write(PINO_VENTOINHA, 1);
    atualizar_logs(logs, VENTOINHA, FUNCIONANDO_DESLIGADO, &status_ventoinha);

    while(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO)
        cv.wait(lck);

    while(status_programa == EM_EXECUCAO) {
        while(contador_alarm_gpio < 5)
            cv.wait(lck);
        
        contador_alarm_gpio = 0;

        mtx_TR.lock(); mtx_histerese.lock();
        double minimo = TR - (histerese/2), maximo = TR + (histerese/2);
        mtx_histerese.unlock(); mtx_TR.unlock();

        mtx_TI.lock();
        if(TI < minimo) {
            if(status_resistor == FUNCIONANDO_DESLIGADO) {
                bcm2835_gpio_write(PINO_RESISTOR, 0);
                atualizar_logs(logs, RESISTOR, FUNCIONANDO_LIGADO, &status_resistor);

                bcm2835_gpio_write(PINO_VENTOINHA, 1);
                atualizar_logs(logs, VENTOINHA, FUNCIONANDO_DESLIGADO, &status_ventoinha);
            }
        }
        else if(TI > maximo) {
            if(status_ventoinha == FUNCIONANDO_DESLIGADO) {
                bcm2835_gpio_write(PINO_VENTOINHA, 0);
                atualizar_logs(logs, VENTOINHA, FUNCIONANDO_LIGADO, &status_ventoinha);

                bcm2835_gpio_write(PINO_RESISTOR, 1);
                atualizar_logs(logs, RESISTOR, FUNCIONANDO_DESLIGADO, &status_resistor);
            }
        }
        mtx_TI.unlock();
    }

    bcm2835_gpio_write(PINO_RESISTOR, 1);
    bcm2835_gpio_write(PINO_VENTOINHA, 1);
    bcm2835_close();

    atualizar_logs(logs, RESISTOR, ENCERRADO, &status_resistor);
    atualizar_logs(logs, VENTOINHA, ENCERRADO, &status_ventoinha);
}

void usar_LCD(WINDOW *logs) {
    int status = INICIANDO;

    if (wiringPiSetup() == -1) {
        atualizar_logs(logs, LCD, ERRO_AO_ABRIR, &status);
        incrementar_disp_funcionando(false);
        close(id.fd);
        return;
    }

    int fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init(fd);

    atualizar_logs(logs, LCD, FUNCIONANDO, &status);
    incrementar_disp_funcionando(true);

    // Só continua depois de verificar todos os dispositivos
    unique_lock<mutex> lck(mtx_gpio);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_verificados != qtd_dispositivos_funcionando) {
        atualizar_logs(logs, LCD, ENCERRADO);
        return;
    }

    while(status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO)
        cv.wait(lck);

    while(status_programa == EM_EXECUCAO) {
        while(contador_alarm_LCD < 5)
            cv.wait(lck);
        
        contador_alarm_LCD = 0;

        pair<string, string> linha1_linha2 = transformar_temperaturas(TI, TE, TR);

        ClrLcd(fd);
        lcdLoc(fd, LINE1);
        typeln(fd, linha1_linha2.first.c_str());
        lcdLoc(fd, LINE2);
        typeln(fd, linha1_linha2.second.c_str());

        atualizar_logs(logs, LCD, FUNCIONANDO, &status);
    }
}

void sensor_externo(WINDOW *logs) {
    struct bme280_dev dev;
    struct identifier id;
    const char path[] = "/dev/i2c-1";
    int status = INICIANDO;

    id.fd = open(path, O_RDWR);

    if(id.fd < 0) {
        // Failed to open the i2c bus
        atualizar_logs(logs, SENSOR_EXTERNO, ERRO_I2C_BUS, &status);
        incrementar_disp_funcionando(false);
        return;
    }

    id.dev_addr = ENDERECO_SENSOR_EXTERNO;
    if (ioctl(id.fd, I2C_SLAVE, id.dev_addr) < 0) {
        // Failed to acquire bus access and/or talk to slave
        atualizar_logs(logs, SENSOR_EXTERNO, ERRO_I2C_BUS_ACESSO, &status);
        incrementar_disp_funcionando(false);
        close(id.fd);
        return;
    }

    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_us = user_delay_us;
    dev.intf_ptr = &id;

    int rslt = bme280_init(&dev);
    if(rslt != BME280_OK) {
        // Failed to initialize the device
        atualizar_logs(logs, SENSOR_EXTERNO, ERRO_I2C_INICIAR_DISP, &status);
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
        atualizar_logs(logs, SENSOR_EXTERNO, ERRO_AO_ABRIR, &status);
        incrementar_disp_funcionando(false);
        return;
    }

    atualizar_logs(logs, SENSOR_EXTERNO, FUNCIONANDO, &status);
    incrementar_disp_funcionando(true);

    unique_lock<mutex> lck(mtx_SE);
    while(qtd_dispositivos_verificados != NUM_DISPOSITIVOS)
        cv.wait(lck);
    
    if(qtd_dispositivos_verificados != qtd_dispositivos_funcionando) {
        close(id.fd);
        atualizar_logs(logs, SENSOR_EXTERNO, ENCERRADO, &status);
        return;
    }

    while(status_programa == EM_EXECUCAO or status_programa == ESPERANDO_PRIM_ENTRADA_USUARIO) {
        while(contador_alarm_SE < 1)
            cv.wait(lck);
        
        contador_alarm_SE = 0;

        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
        if (rslt != BME280_OK) {
            // Failed to set sensor mode
            atualizar_logs(logs, SENSOR_EXTERNO, ERRO_SENSOR_MODE, &status);
        }
        else {
            rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
            if (rslt != BME280_OK) {
                // Failed to get sensor data
                atualizar_logs(logs, SENSOR_EXTERNO, ERRO_AO_LER_TEMP, &status);
            }
            else {
                mtx_TE.lock();
                TE = comp_data.temperature;
                mtx_TE.unlock();
                atualizar_logs(logs, SENSOR_EXTERNO, FUNCIONANDO, &status);
            }
        }
    }

    close(id.fd);
    atualizar_logs(logs, SENSOR_EXTERNO, ENCERRADO);
}
