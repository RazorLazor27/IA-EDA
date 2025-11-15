#include <opencv2/opencv.hpp>
#include <algorithm>
#include <vector>

/**
 * Genera y visualiza un mapa de calor con delimitación opcional de zonas
 * * Esta función crea una visualización de mapa de calor a partir de una matriz de datos numéricos,
 * con la capacidad de mostrar zonas delimitadas por rectángulos negros. La función utiliza 
 * interpolación cúbica para generar transiciones suaves entre los valores y permite diferentes
 * niveles de resolución.
 * * @param M Matriz de datos de entrada (std::vector<std::vector<float>>)
 * Contiene los valores numéricos que se representarán como colores en el mapa.
 * Todos los vectores internos deben tener la misma longitud.
 * * @param factor Factor de escala para la resolución de la imagen resultante (int)
 * Valores recomendados: 10-30
 * - Valores bajos (10-15): menor detalle, procesamiento más rápido
 * - Valores altos (25-30): mayor detalle, procesamiento más lento
 * * @param Z Matriz opcional de etiquetas de zonas (std::vector<std::vector<int>>, por defecto vacía)
 * Si se proporciona, debe tener las mismas dimensiones que M.
 * Cada valor entero representa una zona diferente.
 * Las zonas se delimitan automáticamente con rectángulos negros.
 * Si se omite o está vacía, se muestra solo el mapa de calor sin delimitaciones.
 * * @note Requisitos:
 * - OpenCV debe estar instalado
 * - La matriz M no puede estar vacía
 * - Si se proporciona Z, debe tener las mismas dimensiones que M
 * * @example Uso básico:
 * * std::vector<std::vector<float>> datos = {{1.0, 2.0}, {3.0, 4.0}};
 * plotHeatmap(datos, 20);  // Solo mapa de calor
 * * @example Uso con zonas:
 * * std::vector<std::vector<float>> datos = {{1.0, 2.0}, {3.0, 4.0}};
 * std::vector<std::vector<int>> zonas = {{1, 2}, {1, 2}};
 * plotHeatmap(datos, 25, zonas);  // Mapa con delimitación de zonas
 */
void plotHeatmap(const std::vector<std::vector<float>>& M, int factor, const std::vector<std::vector<int>>& Z = {}) {
    int rows = M.size();
    int cols = M[0].size();
    cv::Mat matM(rows, cols, CV_32F);
    
    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < cols; ++j) {
            matM.at<float>(i,j) = M[i][j];
        }
    }
    
    cv::Mat matZ;
    if (!Z.empty()) {
        matZ = cv::Mat(rows, cols, CV_32S);
        for(int i = 0; i < rows; ++i) {
            for(int j = 0; j < cols; ++j) {
                matZ.at<int>(i,j) = Z[i][j];
            }
        }
    }
    
    cv::Mat M_big;
    cv::resize(matM, M_big, cv::Size(), factor, factor, cv::INTER_CUBIC);

    cv::Mat Mnorm, heatmap;
    cv::normalize(M_big, Mnorm, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::applyColorMap(Mnorm, heatmap, cv::COLORMAP_VIRIDIS);

    if (!matZ.empty()) {
        cv::Mat heatmapRGBA;
        cv::cvtColor(heatmap, heatmapRGBA, cv::COLOR_BGR2BGRA);
        
        cv::Mat Z_big;
        cv::resize(matZ, Z_big, cv::Size(), factor, factor, cv::INTER_NEAREST);

        // --- LÓGICA DE DIBUJO DE BORDES MEJORADA ---
        // En lugar de dibujar rectángulos por zona (que se superponen
        // si las zonas no son rectangulares), vamos a dibujar un pixel
        // negro en el heatmap donde dos celdas adyacentes de Z_big
        // tengan un ID de zona diferente.
        
        // Iteramos sobre la matriz de zonas escalada 'Z_big'
        // Sus dimensiones son (rows * factor) x (cols * factor)
        // Y dibujamos directamente en el 'heatmapRGBA'
        
        for (int i = 0; i < Z_big.rows; ++i) {
            for (int j = 0; j < Z_big.cols; ++j) {
                int currentZone = Z_big.at<int>(i, j);
                bool isBorder = false;

                // Comprobar vecino de la derecha
                if (j + 1 < Z_big.cols) {
                    if (currentZone != Z_big.at<int>(i, j + 1)) {
                        isBorder = true;
                    }
                }
                
                // Comprobar vecino de abajo
                if (i + 1 < Z_big.rows) {
                    if (currentZone != Z_big.at<int>(i + 1, j)) {
                        isBorder = true;
                    }
                }

                if (isBorder) {
                    // Si esta celda (i, j) está en el borde con otra zona
                    // la pintamos de negro.
                    heatmapRGBA.at<cv::Vec4b>(i, j) = cv::Vec4b(0, 0, 0, 255);
                }
            }
        }
        
        // Ahora agregamos el borde blanco EXTERIOR que tenía el código original.
        int borderExtension = 2;
        cv::Mat expandedHeatmap;
        cv::copyMakeBorder(heatmapRGBA, expandedHeatmap, borderExtension, borderExtension, 
                          borderExtension, borderExtension, cv::BORDER_CONSTANT, cv::Scalar(255,255,255,255));

        // El antiguo código de 'uniqueZones' y 'cv::rectangle' ya no es necesario.
        
        cv::cvtColor(expandedHeatmap, heatmap, cv::COLOR_BGRA2BGR);
    }

    std::string windowTitle = !matZ.empty() ? "Mapa de Calor con Zonas" : "Mapa de Calor";
    cv::imshow(windowTitle, heatmap);
    cv::waitKey(0);
}