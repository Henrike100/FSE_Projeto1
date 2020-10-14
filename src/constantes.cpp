#include "constantes.hpp"

const string opcoes[] = {
    "Nenhuma",
    "Teclado",
    "Potenciômetro"
};

const string dispositivos[] = {
    "Sensor Interno (UART)",
    "Sensor Externo (I2C)",
    "Temperatura de Referencia (UART)",
    "Resistor (GPIO)",
    "Ventoinha (GPIO)",
    "Display LCD (I2C)",
    "Arquivo CSV"
};

const string status[] = {
    "Iniciando",
    "Funcionando",
    "Encerrado",
    "Erro ao Iniciar",
    "Erro ao Escrever",
    "Erro ao Solicitar Temperatura",
    "Erro ao Ler Temperatura",
    "Sem Dado Disponível",
    "Funcionando (ligado)",
    "Funcionando (desligado)",
    "Erro no Sensor Mode",
    "Erro ao abrir I2C bus",
    "Erro ao adquirir acesso",
    "Erro ao incializar dispositivo",
};
