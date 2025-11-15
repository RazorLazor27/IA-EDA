===============================================================================
                    CÓDIGO OPCIONAL PARA VISUALIZACION DE MAPAS DE CALOR
===============================================================================

Una herramienta simple para visualizar datos matriciales como mapas de calor 
interactivos, con soporte opcional para delimitacion de zonas.

===============================================================================
                           INSTALACION DE OPENCV
===============================================================================

LINUX (Ubuntu/Debian)
----------------------
    # Actualizar paquetes
    sudo apt update
    
    # Instalar OpenCV
    sudo apt install libopencv-dev
    
    # Verificar instalacion
    pkg-config --modversion opencv4

===============================================================================
                            ESTRUCTURA DEL PROYECTO
===============================================================================

mi_proyecto/
├── heatmap.cpp          # Archivo con la funcion plotHeatmap
├── main.cpp             # Tu codigo principal
└── README.txt           # Este archivo

===============================================================================
                                COMPILACION
===============================================================================

Linux:
    g++ main.cpp heatmap.cpp -I/usr/include/opencv4 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -o programa

EXPLICACION DEL COMANDO:
    -I/usr/include/opencv4    # Ruta de headers para OpenCV 4.x
    -lopencv_core             # Libreria principal de OpenCV
    -lopencv_imgproc          # Procesamiento de imagenes (resize, colormap)
    -lopencv_highgui          # Interfaz grafica (imshow, waitKey)
    -lopencv_imgcodecs        # Codificacion de imagenes

===============================================================================
                             USO DE LA FUNCION
===============================================================================

DECLARACION:
    void plotHeatmap(const std::vector<std::vector<float>>& M, 
                    int factor, 
                    const std::vector<std::vector<int>>& Z = {});

PARAMETROS:
    M       - Matriz de datos a visualizar (std::vector<std::vector<float>>)
    factor  - Factor de escala, valores recomendados: 10-30
    Z       - Matriz de zonas, opcional (std::vector<std::vector<int>>)

VALORES RECOMENDADOS PARA FACTOR:
    10-15: Matrices grandes (>20x20), visualizacion rapida
    20-25: Matrices medianas (5x5 a 20x20), balance calidad/velocidad
    25-30: Matrices pequenas (<10x10), maxima calidad

===============================================================================
                              EJEMPLOS DE USO
===============================================================================

EJEMPLO 1: Mapa basico
----------------------

#include <iostream>
#include <vector>

// Declarar la funcion
void plotHeatmap(const std::vector<std::vector<float>>& M, int factor, const std::vector<std::vector<int>>& Z = {});

int main() {
    // Datos de temperatura
    std::vector<std::vector<float>> temperaturas = {
        {18.5, 19.2, 22.1, 24.3},
        {19.1, 20.0, 23.5, 25.1},
        {21.3, 22.8, 26.2, 28.7},
        {20.8, 22.1, 25.8, 29.2}
    };
    
    // Mostrar solo el mapa de calor
    plotHeatmap(temperaturas, 25);
    
    return 0;
}


EJEMPLO 2: Mapa con zonas
-------------------------

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

===============================================================================
                               PERSONALIZACION
===============================================================================

CAMBIAR ESCALA DE COLORES:
    Si quieres cambiar los colores del mapa, modifica la linea en heatmap.cpp:

    // Linea actual (azul-verde-amarillo)
    cv::applyColorMap(Mnorm, heatmap, cv::COLORMAP_VIRIDIS);

    // Otras opciones:
    cv::applyColorMap(Mnorm, heatmap, cv::COLORMAP_HOT);      // Negro-Rojo-Amarillo-Blanco
    cv::applyColorMap(Mnorm, heatmap, cv::COLORMAP_JET);      // Azul-Cyan-Verde-Amarillo-Rojo
    cv::applyColorMap(Mnorm, heatmap, cv::COLORMAP_RAINBOW);  // Arcoiris completo

===============================================================================