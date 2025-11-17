# IA - Estado del Arte

## Requisitos Previos

* **Compilador C++**: Compatible con C++17 (ej. `g++`).
* **OpenCV 4**: Librerías de desarrollo (`libopencv-dev` o equivalente).

## Compilación

El proyecto cuenta con un `Makefile` configurado para facilitar la compilación. Simplemente ejecute el siguiente comando en la raíz del proyecto:

```bash
make
```
En su defecto, tambien se puede realizar la compilacion manual como:
```bash
g++ src/main.cpp src/heatmap.cpp -I/usr/include/opencv4 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -o spp_solver
```

## Ejecución:
Para visualizar el heatmap de manera pura, solo debemos el nombre del archivo, numero de zonas y el alpha correspondiente.

Si se quiere ver la distribución de zonas según su ID (ver a que numero corresponde cual zona), agregamos una flag como ultimo parametro "--show-labels"

```bash
./spp_solver <Carpeta/archivo_datos.spp> <num_zonas> <alpha> [--show-labels]
```
## Ejemplo:
```bash
./spp_solver Pequeñas/pequena_5.spp 10 0.9 
![no se que hace esto.](/visual_test_results/readme/Map_1.png)
./spp_solver Pequeñas/pequena_2.spp 4 0.25 --show-labels
![alo](/visual_test_results/readme/Map_2.png)

```