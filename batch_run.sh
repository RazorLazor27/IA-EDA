#!/bin/bash

# =================================================================
# SCRIPT MAESTRO PARA EXPERIMENTOS MASIVOS (Vía Makefile)
# =================================================================

INSTANCIAS=(
    "Medianas/mediana_1.spp"
    # "Medianas/mediana_2.spp"
    # "Medianas/mediana_3.spp"
    # "Pequeñas/pequena_4.spp"
    # "Pequeñas/pequena_5.spp"
)

ZONAS_LISTA=(5 6) #agregar 4
ALPHAS_LISTA=(0.3 0.4 0.5) #agregar 0.2

echo "============================================="
echo " PREPARANDO ENTORNO "
echo "============================================="

# Llamamos a 'make' para compilar. Si falla, detenemos el script.
make

if [ $? -ne 0 ]; then
    echo "Error crítico: 'make' falló. Revisa tu código."
    exit 1
fi

# =================================================================
# 2. Ejecución de Experimentos
# =================================================================
total_experimentos=$(( ${#INSTANCIAS[@]} * ${#ZONAS_LISTA[@]} * ${#ALPHAS_LISTA[@]} ))
contador=1

for INST in "${INSTANCIAS[@]}"; do
    for Z in "${ZONAS_LISTA[@]}"; do
        for A in "${ALPHAS_LISTA[@]}"; do
            
            echo ""
            echo "------------------------------------------------------------------"
            echo "EXPERIMENTO $contador DE $total_experimentos"
            echo "Instancia: $INST | Zonas: $Z | Alpha: $A"
            echo "------------------------------------------------------------------"
            
            ./run.sh "$INST" "$Z" "$A"
            
            ((contador++))
            
        done
    done
done


echo ""
echo "============================================="
echo " FINALIZANDO Y LIMPIANDO "
echo "============================================="

make clean

echo "Todo listo. Datos en carpeta 'test_results'."