#!/bin/bash

# ==========================================
# CONFIGURACIÓN DE LA EJECUCIÓN
# ==========================================

# Define aquí la subcarpeta y el archivo
# El C++ busca automáticamente dentro de "instances/", así que solo pon lo que sigue:
CARPETA_TIPO="Pequeñas"   # Ej: Pequeñas, Medianas
ARCHIVO="pequena_1.spp"   # Nombre del archivo

# Parámetros del algoritmo
NUM_ZONAS=4
ALPHA=0.25
MOSTRAR_ETIQUETAS=1       # 1 = Ver números, 0 = Solo colores

# Nombre del ejecutable
EJECUTABLE="./spp_solver"

# Construcción de la ruta que espera el C++ (Pequeñas/pequena_1.spp)
ARGUMENTO_ARCHIVO="$CARPETA_TIPO/$ARCHIVO"

# ==========================================
# LÓGICA DE COMPILACIÓN Y EJECUCIÓN
# ==========================================

# 1. Verificar si existe el Makefile
if [ ! -f "Makefile" ]; then
    echo "Error: No se encuentra el archivo 'Makefile' en la raíz."
    exit 1
fi

# 2. Compilar si es necesario (o si el ejecutable no existe)
echo "--- Verificando estado del proyecto ---"
make -q # Comprueba si está actualizado
if [ $? -ne 0 ]; then
    echo "Cambios detectados o ejecutable faltante. Compilando..."
    make
    if [ $? -ne 0 ]; then
        echo "❌ Error crítico en la compilación."
        exit 1
    fi
else
    echo "✅ El proyecto ya está compilado y actualizado."
fi

# 3. Ejecutar el programa
echo ""
echo ">>> EJECUTANDO SOLVER <<<"
echo "📂 Instancia: instances/$ARGUMENTO_ARCHIVO"
echo "🎯 Zonas:     $NUM_ZONAS"
echo "📊 Alpha:     $ALPHA"
echo "🏷️  Etiquetas: $([ "$MOSTRAR_ETIQUETAS" -eq 1 ] && echo "SÍ" || echo "NO")"
echo "------------------------------------------"

# Ejecución
$EJECUTABLE "$ARGUMENTO_ARCHIVO" "$NUM_ZONAS" "$ALPHA" "$MOSTRAR_ETIQUETAS"

echo ""
echo ">>> Fin del proceso <<<"