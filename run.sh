#!/bin/bash

# Verificar argumentos
if [ "$#" -lt 3 ]; then
    echo "Uso: $0 <archivo_instancia.spp> <num_zonas> <alpha>"
    echo "Ejemplo: ./run.sh instances/Pequeñas/pequena_2.spp 4 0.25"
    exit 1
fi

INSTANCIA=$1
ZONAS=$2
ALPHA=$3
NUM_EJECUCIONES=20

# Definir carpeta base de resultados
DIR_RESULTADOS="test_results"

# Construimos la ruta del archivo de salida
OUTPUT_FILE="${DIR_RESULTADOS}/resultados_${INSTANCIA}_z${ZONAS}_a${ALPHA}.txt"

# --- CORRECCIÓN ---
# Obtenemos el directorio donde vivirá este archivo específico
OUTPUT_DIR=$(dirname "$OUTPUT_FILE")

# Creamos ese directorio específico (incluyendo subcarpetas como 'resultados_Pequeñas')
if [ ! -d "$OUTPUT_DIR" ]; then
    echo "Creando directorio para resultados: $OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR"
fi
# ------------------

echo "Compilando desde src/..."
g++ src/main.cpp src/heatmap.cpp -I/usr/include/opencv4 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -o spp_solver

if [ $? -ne 0 ]; then
    echo "Error compilación. Verifica que main.cpp y heatmap.cpp estén en la carpeta src/"
    exit 1
fi

echo "Iniciando 20 ejecuciones..."
echo "Instancia: $INSTANCIA | Zonas: $ZONAS | Alpha: $ALPHA"
echo "Guardando resultados en: $OUTPUT_FILE"

# Escribir encabezado (número de ejecuciones)
echo "$NUM_EJECUCIONES" > "$OUTPUT_FILE"

for ((i=1; i<=NUM_EJECUCIONES; i++))
do
    echo -ne "Run $i... "
    # Usamos --no-gui para evitar ventanas emergentes
    SALIDA=$(./spp_solver "$INSTANCIA" "$ZONAS" "$ALPHA" --no-gui)
    
    # Extraer datos con grep y awk
    C_SIN=$(echo "$SALIDA" | grep "sin penalizacion" | awk '{print $6}')
    C_CON=$(echo "$SALIDA" | grep "con penalizacion" | awk '{print $6}')
    TIME=$(echo "$SALIDA" | grep "Tiempo de ejecucion" | awk '{print $4}')
    
    echo "$C_SIN $C_CON $TIME" >> "$OUTPUT_FILE"
    echo "Done ($TIME s)"
done

echo "Experimento finalizado."
echo "Resultados disponibles en: $OUTPUT_FILE"