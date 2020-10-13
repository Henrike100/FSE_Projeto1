#include "utilidades.hpp"

pair<string, string> transformar_temperaturas(const float *TI, const float *TE, const float *TR) {
    const string linha1 = "TI:" + to_string(*TI).substr(0, 4) + " TE:" + to_string(*TE).substr(0, 4);
    const string linha2 = "TR:" + to_string(*TR).substr(0, 4);

    return make_pair(linha1, linha2);
}
