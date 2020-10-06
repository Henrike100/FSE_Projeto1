#include <thread>
#include "interfaces.hpp"
#include "paralelos.hpp"

using namespace std;

// Modificar bme280_defs.hpp
// linha 118

// TR, TI, TE são de leitura e escrita: controlar acesso

// Se opção = 1: Usuário escolhe temperatura 1 vez e coloca em TR
// Se opção = 2: Thread verifica valor do potenciômetro e atualiza TR
float TR;

// Temperatura Interna
float TI = 30.0;

// Temperatura Externa
float TE = 30.0;

int main(int argc, const char *argv[]) {
    signal(SIGINT, signal_handler);
    thread thread_temperaturas(atualizar_temperaturas, &TI, &TE);
    thread thread_potenciometro;
    
    int opt = menu_inicializacao();

    switch (opt) {
    case 1:
        TR = pegar_temperatura(&TE);
        break;
    case 2:
        thread_potenciometro = thread(ler_potenciometro, &TR);
        break;
    default:
        return 0;
    }

    float histerese = pegar_histerese();

    thread thread_apresenta(apresentar_temperaturas, &TI, &TE, &TR);
    thread thread_lcd(atualizar_lcd);
    thread thread_csv(gerar_log_csv, &TI, &TE, &TR);
    
    thread_temperaturas.join();
    if(opt == 2) thread_potenciometro.join();
    thread_apresenta.join();
    thread_lcd.join();
    thread_csv.join();

   return 0;
}
