# IA - Estado del Arte

## Uso y Ejecución

Para ejecutar el programa, utilice la siguiente sintaxis en la terminal:

### Compilacion:

g++ src/main.cpp src/heatmap.cpp -I/usr/include/opencv4 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -o spp_solver

### Ejecución:

./spp_solver <Carpeta/archivo_datos.spp> <num_zonas> <alpha> [--show-labels]

### Ejemplo:

./spp_solver Pequeñas/pequena_5.spp 10 0.9 --show-labels