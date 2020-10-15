#ifndef CONSTANTES_HPP
#define CONSTANTES_HPP

#include <string>

using namespace std;

// Número total de dispositivos/conexões
#define NUM_DISPOSITIVOS            5

/*** Status do Programa ***/

// Nem todos os dispositivos ou conexões foram inicializados, deve-se encerrar
#define ENCERRAMENTO_COM_ERRO_INICIO        0
// Durante a execução, algum dispositivo ou conexão apresentou problema, deve-se encerrar
#define ENCERRAMENTO_COM_ERRO_EXEC          1
// O programa recebeu algum sinal, deve-se encerrar
#define ENCERRAMENTO_VIA_SIGNAL             2
// O usuário desejou encerrar a execução do programa por vontade própria
#define ENCERRAMENTO_VIA_USER               3
// O programa só pode realmente comecar quando o usuario digitar algo pela primeira vez
#define ESPERANDO_PRIM_ENTRADA_USUARIO      4
// Execucao normal, apos o usuario envia a entrada pelo menos uma vez
#define EM_EXECUCAO                         5

// status
#define INICIANDO                   0
#define FUNCIONANDO                 1
#define ENCERRADO                   2
#define ERRO_AO_ABRIR               3
#define ERRO_AO_ESCREVER            4
#define ERRO_AO_SOLICITAR_TEMP      5
#define ERRO_AO_LER_TEMP            6
#define SEM_DADO_DISPONIVEL         7
#define FUNCIONANDO_LIGADO          8
#define FUNCIONANDO_DESLIGADO       9
#define ERRO_SENSOR_MODE           10
#define ERRO_I2C_BUS               11
#define ERRO_I2C_BUS_ACESSO        12
#define ERRO_I2C_INICIAR_DISP      13
#define ERRO_AO_CONFIG_SENSOR      14

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
