# Definición de compilador y flags
CXX = g++
CXXFLAGS = -I/usr/include/opencv4
LDFLAGS = -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs
TARGET = spp_solver
SRC_DIR = src

# Archivos fuente
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/heatmap.cpp

# Regla por defecto (lo que pasa cuando escribes 'make')
all: $(TARGET)

# Regla de compilación
$(TARGET): $(SOURCES)
	@echo "Compilando proyecto..."
	$(CXX) $(SOURCES) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET)
	@echo "Compilación exitosa: $(TARGET) generado."

# Regla de limpieza (lo que pasa cuando escribes 'make clean')
clean:
	@echo "Limpiando archivos temporales..."
	rm -f $(TARGET)
	@echo "Limpieza completada."