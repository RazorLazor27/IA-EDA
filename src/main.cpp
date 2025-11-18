#include <iostream>
#include <vector>
#include <cmath>      // Para sqrt y pow
#include <numeric>    // Para std::accumulate
#include <random>     // Para generación de números aleatorios (proceso estocástico)
#include <limits>     // Para std::numeric_limits
#include <map>        // Para agrupar valores por zona
#include <fstream>    // Para lectura de archivos
#include <string>

// Muestra el mapa de calor
void plotHeatmap(const std::vector<std::vector<float>>& M, int factor, const std::vector<std::vector<int>>& Z = {}, bool showLabels = false);

// --------------------------------------------------------------------------
// ESTRUCTURAS DE DATOS PRINCIPALES
// --------------------------------------------------------------------------

/**
 * @struct Instancia cualquiera, para el comienzo del algoritmo se puede definir la instancia inicial y luego esta se va modificando mediante avance el algoritmo.
 * @brief Almacena los datos de entrada del problema.
 *
 * @var datos_terreno Matriz (N x M) con los valores del índice (ej. NDVI, humedad).
 * @var num_zonas (p) El número de sensores/zonas a definir.
 */
struct Instancia {
    std::vector<std::vector<float>> datos_terreno; 
    int num_zonas;
    int N_filas;
    int M_columnas;

    Instancia(const std::vector<std::vector<float>>& datos, int p)
        : datos_terreno(datos), num_zonas(p) {
        N_filas = datos.size();
        M_columnas = (N_filas > 0) ? datos[0].size() : 0;
    }
};

/**
 * @struct Solucion
 * @brief Representa una solución al problema.
 *
 * @var zonas_asignadas Matriz (N x M) donde cada celda (i, j) tiene un ID
 * de zona (0 a p-1).
 * @var costo El valor de la función objetivo (costo) para esta solución.
 * Usamos 'double' para precisión.
 */
struct Solucion {
    std::vector<std::vector<int>> zonas_asignadas;
    double costo;

    Solucion(int N, int M) : costo(std::numeric_limits<double>::infinity()) {
        zonas_asignadas.resize(N, std::vector<int>(M, 0));
    }
};

// --------------------------------------------------------------------------
// MOTOR DE NÚMEROS ALEATORIOS (Para 'Restart')
// --------------------------------------------------------------------------
std::mt19937 gen(std::random_device{}()); // Generador Mersenne Twister

/**
 * @brief Genera un entero aleatorio en el rango [min, max]
 */
int randint(int min, int max) {
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

// --------------------------------------------------------------------------
// IMPLEMENTACIÓN MÍNIMA REQUERIDA
// --------------------------------------------------------------------------

// 1. Generación de Solución Inicial (Greedy / Aleatoria)

/**
 * @brief Genera una solución inicial aleatoria.
 *
 * Asigna cada celda (i, j) del terreno a una zona aleatoria (0 a p-1).
 * Esto sirve como punto de partida para el Hill Climbing y es esencial
 * para la estrategia de "Restart", ya que cada "Restart" comienza
 * desde un punto aleatorio diferente.
 *
 * @param instancia Los datos del problema.
 * @return Una 'Solucion' inicializada aleatoriamente.
 */
Solucion generar_solucion_inicial_aleatoria(const Instancia& instancia) {
    Solucion sol(instancia.N_filas, instancia.M_columnas);

    for (int i = 0; i < instancia.N_filas; ++i) {
        for (int j = 0; j < instancia.M_columnas; ++j) {
            // Asigna un ID de zona aleatorio entre 0 y p-1
            sol.zonas_asignadas[i][j] = randint(0, instancia.num_zonas - 1);
        }
    }
    // El costo se calculará por separado
    return sol;
}

// 2. Cálculo de Función de Evaluación

/**
 * @brief Función auxiliar para calcular la varianza de un conjunto de datos.
 *
 * Varianza = (1/N) * Sum( (x_i - media)^2 )
 *
 * @param valores Vector de valores de una zona.
 * @return La varianza (double) de los valores.
 */
double calcular_varianza(const std::vector<float>& valores) {
    if (valores.empty() || valores.size() == 1) {
        return 0.0; // Varianza de 0 o 1 elemento es 0.
    }

    double n = valores.size();
    
    // Calcular la media
    double suma = std::accumulate(valores.begin(), valores.end(), 0.0);
    double media = suma / n;

    // Calcular la suma de los cuadrados de las diferencias
    double suma_cuadrados_dif = 0.0;
    for (float val : valores) {
        suma_cuadrados_dif += std::pow(val - media, 2);
    }

    double varianza = suma_cuadrados_dif / n;
    return varianza;
}

/**
 * @brief Calcula la varianza total de todos los datos en la instancia.
 */
double calcular_varianza_total(const Instancia& instancia){
    std::vector<float> full_data;
    for (int i = 0; i < instancia.N_filas; ++i) {
        for (int j = 0; j < instancia.M_columnas; ++j) {
            full_data.push_back(instancia.datos_terreno[i][j]);
        }
    }
    return calcular_varianza(full_data);
}

/**
 * @brief Calcula la función de evaluación (costo) de una solución.
 *
 * Según lo investigado en el estado del arte, el objetivo del problema es
 * minimizar la pérdida de representatividad de las zonas definidas.
 * Lo que efectivamente se traduce a minimizar la suma de las varianzas internas
 * de cada zona.
 *
 * @param instancia Los datos del terreno (S).
 * @param solucion La partición de zonas (Z).
 * @return El costo total (Suma de Varianzas Internas) como un 'double'.
 */
double evaluar_solucion(const Instancia& instancia, const Solucion& solucion, double umbral_varianza) {
    // Usamos un map para agrupar todos los valores que pertenecen a cada zona
    std::map<int, std::vector<float>> valores_por_zona;

    //Agrupar valores por zona
    for (int i = 0; i < instancia.N_filas; ++i) {
        for (int j = 0; j < instancia.M_columnas; ++j) {

            int zona_id = solucion.zonas_asignadas[i][j];
            float valor_dato = instancia.datos_terreno[i][j];
            valores_por_zona[zona_id].push_back(valor_dato);
        }
    }

    // Calcular la varianza de cada zona y sumarlas
    double costo_total = 0.0;
    double penalizacion = 0.0;
    const double deterrant = 1e9; // Factor de penalización muuuuy grande

    for (int k = 0; k < instancia.num_zonas; ++k) {
        // Busca la zona k en el map. Si no existe (zona vacía), el vector estará vacío.
        double varianza_zona = calcular_varianza(valores_por_zona[k]);
        costo_total += varianza_zona;
        if (varianza_zona > umbral_varianza) {
            // Si no cumple el umbral, aplicamos una penalización grande
            penalizacion += (varianza_zona - umbral_varianza) * deterrant;
        }
    }
    // Como estamos minimizando los valores, la penalización se suma al costo total
    return costo_total + penalizacion;
}

// Algoritmo Hill Climbing 

/**
 * @brief Implementa la búsqueda local Hill Climbing con estrategia "First Improvement".
 *
 * Explora el "vecindario" de la solución actual. El vecindario se define
 * cambiando la asignación de ZONA de UNA celda (i, j) a la vez.
 *
 * "First Improvement": Tan pronto como encuentra un movimiento (un "vecino")
 * que MEJORA (reduce) el costo, acepta ese movimiento y reinicia la
 * búsqueda desde la nueva solución.
 *
 * Continúa hasta que no se puede encontrar ninguna mejora en una pasada
 * completa (alcanza un óptimo local).
 *
 * @param instancia Los datos del problema.
 * @param sol_inicial La solución desde la cual comenzar la búsqueda.
 * @return La `Solucion` optimizada (óptimo local).
 */
Solucion hill_climbing_first_improvement(const Instancia& instancia, Solucion sol_actual, double umbral_varianza) {
    
    sol_actual.costo = evaluar_solucion(instancia, sol_actual, umbral_varianza);

    bool mejora_encontrada;
    do {
        mejora_encontrada = false;
        bool restart = false;

        // Iteramos por cada celda del terreno
        for (int i = 0; i < instancia.N_filas && !restart; ++i) {
            for (int j = 0; j < instancia.M_columnas && !restart; ++j) {
                
                int zona_original = sol_actual.zonas_asignadas[i][j];

                // Probamos mover esta celda (i, j) a cada OTRA zona posible
                for (int nueva_zona = 0; nueva_zona < instancia.num_zonas; ++nueva_zona) {
                    
                    if (nueva_zona == zona_original) continue;

                    sol_actual.zonas_asignadas[i][j] = nueva_zona;
                    
                    double nuevo_costo = evaluar_solucion(instancia, sol_actual, umbral_varianza);

                    // Este es el First 'Improvement' -> si mejora, aceptamos y salimos
                    if (nuevo_costo < sol_actual.costo) {
                        
                        sol_actual.costo = nuevo_costo;
                        restart = true;
                        mejora_encontrada = true;
                        break;
                    } else {
                        // Desahacer el movimiento
                        sol_actual.zonas_asignadas[i][j] = zona_original;
                    }
                }
            }
        } 
    } while (mejora_encontrada);
    return sol_actual;
}


// Implementacion del Restart en First improvement

/**
 * @brief Resuelve el problema usando Hill Climbing con Múltiples Restarts.
 *
 * Ejecuta el algoritmo `hill_climbing_first_improvement` un número
 * determinado de veces (`num_restarts`), cada vez comenzando desde
 * una solución inicial aleatoria diferente.
 *
 * Guarda y devuelve la MEJOR solución encontrada en todas las ejecuciones.
 *
 * @param instancia Los datos del problema.
 * @param num_restarts El número de veces que se reiniciará el algoritmo.
 * @return La mejor `Solucion` encontrada globalmente.
 */
Solucion resolver_con_restart(const Instancia& instancia, int num_restarts, double umbral_varianza) {
    
    Solucion mejor_solucion_global(instancia.N_filas, instancia.M_columnas);
    std::cout << "Iniciando Hill Climbing con " << num_restarts << " restarts..." << std::endl;

    for (int r = 0; r < num_restarts; ++r) {
        
        // Generamos una solución inicial aleatoria
        Solucion sol_inicial = generar_solucion_inicial_aleatoria(instancia);

        // Mejoramos dicha solucion con Hill Climbing
        Solucion sol_optimo_local = hill_climbing_first_improvement(instancia, sol_inicial, umbral_varianza);

        std::cout << "  Restart " << (r + 1) << "/" << num_restarts 
                  << " -> Costo (Con Penalización) " << sol_optimo_local.costo << std::endl;

        // Comparamos con la mejor solución global encontrada hasta ahora
        if (sol_optimo_local.costo < mejor_solucion_global.costo) {
            mejor_solucion_global = sol_optimo_local;
            std::cout << "  --> Nueva mejor solucion encontrada! " << std::endl;
        }
    }

    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Optimizacion finalizada." << std::endl;

    // Mostramos la mejor solucion sin penalizacion (asumiendo que se llego a una solución valida)
    double costo_sin_penalizacion = evaluar_solucion(instancia, mejor_solucion_global, std::numeric_limits<double>::infinity());
    std::cout << "Mejor Costo Final (sin penalizacion): " << costo_sin_penalizacion << std::endl;
    std::cout << "Mejor Costo Final (con penalizacion): " << mejor_solucion_global.costo << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;


    return mejor_solucion_global;
}

/**
 * @brief Lee los datos del terreno desde un archivo de texto .spp
 * 
 * El archivo debe tener el siguiente formato:
 * 
 * * - La primera línea contiene dos enteros: m n (número de filas y columnas)
 * 
 * * - Las siguientes m líneas contienen n valores flotantes cada una,
 *   separados por espacios, representando los datos del terreno.
 */
std::vector<std::vector<float>> leer_datos(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filename);
    }

    int m, n;
    file >> m >> n;
    std::vector<std::vector<float>> datos(m, std::vector<float>(n));
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            file >> datos[i][j];
        }
    }
    return datos;
}

int main(int argc, char* argv[]) {
    
    // Cargamos los datos del terreno desde un archivo
    if (argc < 4) {
        std::cerr << "Uso: " << argv[0] << " <archivo_datos.spp> <num_zonas> <alpha>" << std::endl;
        std::cerr << "Ejemplo: " << argv[0] << " instancia_ejemplo.spp 4 0.25" << std::endl;
        return 1;
    }

    std::string archivo_datos = argv[1];
    int p_zonas = std::stoi(argv[2]);
    double alpha = std::stod(argv[3]);

    bool mostrar_etiquetas = false;

    if (argc >= 5) {
        std::string etiqueta_flag = argv[4];
        if (etiqueta_flag == "--show-labels") {
            mostrar_etiquetas = true;
        }
    }

    if (alpha < 0.0 || alpha > 1.0) {
        std::cerr << "Error: alpha debe estar entre 0.0 y 1.0" << std::endl;
        return 1;
    }

    auto datos = leer_datos("instances/" + archivo_datos);
    Instancia instancia_problema(datos, p_zonas);

    for (const auto& fila : datos) {
        for (float x : fila)
            std::cout << x << " ";
        std::cout << "\n";
    }

    double varianza_total_S = calcular_varianza_total(instancia_problema);
    double umbral_varianza_max = alpha * varianza_total_S;

    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << "Instancia cargada: " << instancia_problema.N_filas << "x" << instancia_problema.M_columnas << std::endl;
    std::cout << "Numero de zonas (p): " << p_zonas << std::endl;
    std::cout << "Nivel de homogeneidad (alpha): " << alpha << std::endl;
    std::cout << "Varianza Total (Var(S)): " << varianza_total_S << std::endl;
    std::cout << "Umbral Max. Varianza por Zona (alpha * Var(S)): " << umbral_varianza_max << std::endl;
    

    // Más restarts = más tiempo, pero mayor probabilidad de una buena solución.
    int num_restarts = 20;

    // Todo el codigo corre con esta linea jajaj
    Solucion solucion_final = resolver_con_restart(instancia_problema, num_restarts, umbral_varianza_max);

    // Hacemos una copia y sumamos 1 a todas las celdas para la visualización.
    std::vector<std::vector<int>> zonas_para_mostrar = solucion_final.zonas_asignadas;
    for (int i = 0; i < instancia_problema.N_filas; ++i) {
        for (int j = 0; j < instancia_problema.M_columnas; ++j) {
            zonas_para_mostrar[i][j] += 1;
        }
    }


    // Aqui se muestra efectivamente el mapa de calor 
    
    std::cout << "Mostrando mapa de calor de la solucion final..." << std::endl;
    std::cout << "Presione cualquier tecla en la ventana del mapa para salir." << std::endl;
    
    plotHeatmap(instancia_problema.datos_terreno, 
                30,
                zonas_para_mostrar,
                mostrar_etiquetas);

    return 0;
}