#ifndef CONSTANTES_HPP
#define CONSTANTES_HPP

#include <string>

using namespace std;

#define NUM_DISPOSITIVOS            4

// status
#define INICIANDO                   0
#define FUNCIONANDO                 1
#define ENCERRADO                   2
#define ERRO_AO_ABRIR               3
#define ERRO_AO_ESCREVER            4
#define ERRO_AO_SOLICITAR_TEMP      5
#define ERRO_AO_LER_TEMP            6
#define SEM_DADO_DISPONIVEL         7

// dispositivos
#define SENSOR_INTERNO              0
#define SENSOR_EXTERNO              1
#define TEMPERATURA_REFERENCIA      2
#define RESISTOR                    3
#define VENTOINHA                   4
#define LCD                         5
#define CSV                         6

extern const string opcoes[];
extern const string dispositivos[];
extern const string status[];

#endif // CONSTANTES_HPP
