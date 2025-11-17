# --- Makefile para compilar el proyecto SPP Solver ---
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -O3

INCLUDES = -I/usr/include/opencv4

# librerias de OpenCV necesarias
LIBS     = -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs

# Nombre del ejecutable
TARGET   = spp_solver

# Archivos fuente
SRCS     = src/main.cpp src/heatmap.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) -o $(TARGET) $(LIBS)
	@echo "Ejecutable: ./$(TARGET)"

clean:
	rm -f $(TARGET)

.PHONY: all clean