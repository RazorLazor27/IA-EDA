#include <iostream>
#include <vector>

void plotHeatmap(const std::vector<std::vector<float>>& M, int factor, const std::vector<std::vector<int>>& Z = {});

int main() {
    // Datos
    std::vector<std::vector<float>> datos = {
        {10.1, 12.3, 15.8, 18.2},
        {11.5, 13.7, 16.9, 19.4},
        {13.2, 15.1, 18.3, 20.7},
        {15.8, 17.6, 20.1, 22.9}
    };
    
    // Definir zonas (cada numero es una zona diferente)
    std::vector<std::vector<int>> zonas = {
        {1, 1, 2, 2},
        {1, 1, 2, 2},
        {3, 3, 4, 4},
        {3, 3, 4, 4}
    };
    
    // Mostrar mapa con delimitacion de zonas
    plotHeatmap(datos, 25, zonas);
    
    return 0;
}