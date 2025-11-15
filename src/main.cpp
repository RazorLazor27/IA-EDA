#include <iostream>
#include <vector>
#include <cmath>      // Para sqrt y pow
#include <numeric>    // Para std::accumulate
#include <random>     // Para generación de números aleatorios (C++11)
#include <limits>     // Para std::numeric_limits
#include <map>        // Para agrupar valores por zona

// --------------------------------------------------------------------------
// DECLARACIÓN DE LA FUNCIÓN DE VISUALIZACIÓN
// --------------------------------------------------------------------------
// Esta función está definida en heatmap.cpp y la usaremos para mostrar
// el resultado final.
// --------------------------------------------------------------------------
void plotHeatmap(const std::vector<std::vector<float>>& M, int factor, const std::vector<std::vector<int>>& Z = {});

// --------------------------------------------------------------------------
// ESTRUCTURAS DE DATOS PRINCIPALES
// --------------------------------------------------------------------------

/**
 * @struct Instancia
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
// Usamos el motor moderno de C++ para aleatoriedad de alta calidad.
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

// --- 1. Generación de Solución Inicial (Greedy / Aleatoria) ---

/**
 * @brief Genera una solución inicial aleatoria.
 *
 * Asigna cada celda (i, j) del terreno a una zona aleatoria (0 a p-1).
 * Esto sirve como punto de partida para el Hill Climbing y es esencial
 * para la estrategia de "Restart", ya que cada "Restart" comienza
 * desde un punto aleatorio diferente.
 *
 * @param instancia Los datos del problema.
 * @return Una `Solucion` inicializada aleatoriamente.
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

// --- 2. Cálculo de Función de Evaluación ---

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

    // Retornar la varianza
    return suma_cuadrados_dif / n;
}

/**
 * @brief Calcula la función de evaluación (costo) de una solución.
 *
 * Basado en el PDF (Estado del Arte), el objetivo es minimizar la
 * pérdida de representatividad, modelada como la MINIMIZACIÓN DE
 * LA SUMA DE LAS VARIANZAS INTERNAS de cada zona.
 *
 * @param instancia Los datos del terreno (S).
 * @param solucion La partición de zonas (Z).
 * @return El costo total (Suma de Varianzas Internas) como un 'double'.
 */
double evaluar_solucion(const Instancia& instancia, const Solucion& solucion) {
    // Usamos un map para agrupar todos los valores que pertenecen a cada zona
    // Clave: ID de zona (int), Valor: Vector de valores (float)
    std::map<int, std::vector<float>> valores_por_zona;

    // 1. Agrupar valores por zona
    for (int i = 0; i < instancia.N_filas; ++i) {
        for (int j = 0; j < instancia.M_columnas; ++j) {
            int zona_id = solucion.zonas_asignadas[i][j];
            float valor_dato = instancia.datos_terreno[i][j];
            valores_por_zona[zona_id].push_back(valor_dato);
        }
    }

    // 2. Calcular la varianza de cada zona y sumarlas
    double costo_total = 0.0;
    for (int k = 0; k < instancia.num_zonas; ++k) {
        // Busca la zona k en el map. Si no existe (zona vacía), el vector estará vacío.
        costo_total += calcular_varianza(valores_por_zona[k]);
    }

    return costo_total;
}

// --------------------------------------------------------------------------
// ALGORITMO: HILL CLIMBING (FIRST IMPROVEMENT)
// --------------------------------------------------------------------------

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
Solucion hill_climbing_first_improvement(const Instancia& instancia, Solucion sol_actual) {
    
    // Calcula el costo de la solución inicial
    sol_actual.costo = evaluar_solucion(instancia, sol_actual);

    bool mejora_encontrada;
    do {
        mejora_encontrada = false;

        // Iteramos por cada celda del terreno
        for (int i = 0; i < instancia.N_filas; ++i) {
            for (int j = 0; j < instancia.M_columnas; ++j) {
                
                int zona_original = sol_actual.zonas_asignadas[i][j];

                // Probamos mover esta celda (i, j) a cada OTRA zona posible
                for (int nueva_zona = 0; nueva_zona < instancia.num_zonas; ++nueva_zona) {
                    
                    if (nueva_zona == zona_original) {
                        continue; // No probamos mover la celda a su propia zona
                    }

                    // 1. Realizar el movimiento (crear el "vecino")
                    sol_actual.zonas_asignadas[i][j] = nueva_zona;
                    
                    // 2. Evaluar el costo del vecino
                    double nuevo_costo = evaluar_solucion(instancia, sol_actual);

                    // 3. Aplicar criterio "First Improvement"
                    if (nuevo_costo < sol_actual.costo) {
                        // ¡Mejora! Aceptamos la nueva solución
                        sol_actual.costo = nuevo_costo;
                        mejora_encontrada = true;
                        
                        // Rompemos los bucles para reiniciar la búsqueda
                        // desde la nueva solución (sol_actual)
                        goto reiniciar_busqueda; // Salta al final del bucle 'do-while'
                    } else {
                        // 4. Deshacer el movimiento (no fue mejor)
                        sol_actual.zonas_asignadas[i][j] = zona_original;
                    }
                } // fin bucle nueva_zona
            } // fin bucle j
        } // fin bucle i

    reiniciar_busqueda:; // Etiqueta para el 'goto'
    
    } while (mejora_encontrada); // Repetir mientras encontremos mejoras

    // Devuelve el óptimo local encontrado
    return sol_actual;
}


// --------------------------------------------------------------------------
// ALGORITMO PRINCIPAL: HILL CLIMBING CON RESTART
// --------------------------------------------------------------------------

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
Solucion resolver_con_restart(const Instancia& instancia, int num_restarts) {
    
    Solucion mejor_solucion_global(instancia.N_filas, instancia.M_columnas);
    // El costo inicial es infinito
    
    std::cout << "Iniciando Hill Climbing con " << num_restarts << " restarts..." << std::endl;

    for (int r = 0; r < num_restarts; ++r) {
        
        // 1. Generar una solución inicial aleatoria (PUNTO DE PARTIDA)
        Solucion sol_inicial = generar_solucion_inicial_aleatoria(instancia);

        // 2. Optimizarla con Hill Climbing (encontrar el óptimo local)
        Solucion sol_optimo_local = hill_climbing_first_improvement(instancia, sol_inicial);

        std::cout << "  Restart " << (r + 1) << "/" << num_restarts 
                  << " -> Costo (Varianza Total): " << sol_optimo_local.costo << std::endl;

        // 3. Comparar con la mejor solución encontrada hasta ahora
        if (sol_optimo_local.costo < mejor_solucion_global.costo) {
            mejor_solucion_global = sol_optimo_local;
            std::cout << "  *** ¡Nueva mejor solucion encontrada! ***" << std::endl;
        }
    }

    std::cout << "---------------------------------------------------" << std::endl;
    std::cout << "Optimizacion finalizada." << std::endl;
    std::cout << "Mejor costo (Varianza Total) encontrado: " 
              << mejor_solucion_global.costo << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;

    return mejor_solucion_global;
}


// --------------------------------------------------------------------------
// FUNCIÓN PRINCIPAL (main)
// --------------------------------------------------------------------------
// Aquí es donde unimos todo.
// --------------------------------------------------------------------------
int main() {
    
    // --- 1. Lectura de Instancia ---
    // (Datos de ejemplo del 'main.cpp' original)
    std::vector<std::vector<float>> datos = {
        {10.1, 12.3, 15.8, 18.2},
        {11.5, 13.7, 16.9, 19.4},
        {13.2, 15.1, 18.3, 20.7},
        {15.8, 17.6, 20.1, 22.9}
    };
    
    // Definimos el número de sensores (zonas) que queremos
    int p_zonas = 4;
    
    Instancia instancia_problema(datos, p_zonas);

    // Definimos cuántos "restarts" queremos hacer.
    // Más restarts = más tiempo, pero mayor probabilidad de una buena solución.
    int num_restarts = 20;

    // --- 2. Resolver el Problema ---
    // Esto ejecuta:
    //   - Generación de Solución Inicial (aleatoria, para cada restart)
    //   - Cálculo de Función de Evaluación (dentro del Hill Climbing)
    //   - Hill Climbing First Improvement
    Solucion solucion_final = resolver_con_restart(instancia_problema, num_restarts);

    // --- 3. Visualizar la Solución Final ---
    // Usamos la función de 'heatmap.cpp' para mostrar
    // el mapa de calor de los DATOS (instancia_problema.datos_terreno)
    // con las ZONAS (solucion_final.zonas_asignadas) superpuestas.
    std::cout << "Mostrando mapa de calor de la solucion final..." << std::endl;
    std::cout << "Presione cualquier tecla en la ventana del mapa para salir." << std::endl;
    
    plotHeatmap(instancia_problema.datos_terreno, 
                30, // Factor de zoom (más grande para ver mejor)
                solucion_final.zonas_asignadas);

    return 0;
}