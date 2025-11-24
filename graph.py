import os
import glob
import re
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Configuración de estilo
sns.set_theme(style="whitegrid")
plt.rcParams.update({'figure.max_open_warning': 0})

def parse_filename(filename):
    """
    Extrae metadata del nombre del archivo.
    Maneja casos donde 'resultados_' es parte de la carpeta o del archivo.
    """
    basename = os.path.basename(filename)
    
    # --- CORRECCIÓN DE REGEX ---
    # (?:resultados_)? -> Hace que el prefijo 'resultados_' sea OPCIONAL.
    # Esto permite leer archivos como "pequena_2.spp_z4_a0.3.txt" 
    # que están dentro de carpetas como "test_results/resultados_Pequeñas/"
    match = re.search(r'(?:resultados_)?(.+)_z(\d+)_a([\d\.]+)\.txt', basename)
    
    if match:
        instancia = match.group(1)
        zonas = int(match.group(2))
        alpha = float(match.group(3))
        return instancia, zonas, alpha
    return None, None, None

def read_experiment_data(folder_path="test_results"):
    data_records = []
    
    # Buscar todos los archivos .txt recursivamente
    files = glob.glob(os.path.join(folder_path, "**/*.txt"), recursive=True)
    
    print(f"Encontrados {len(files)} archivos de resultados.")

    count_read = 0
    for file_path in files:
        instancia, zonas, alpha = parse_filename(file_path)
        
        if instancia is None:
            # Debug: avisar si algún archivo no cumple el formato (opcional)
            # print(f"Saltando archivo con nombre no reconocido: {os.path.basename(file_path)}")
            continue
            
        try:
            # Leemos el archivo, saltando la primera línea con el conteo
            df_temp = pd.read_csv(file_path, sep=" ", skiprows=1, names=["Costo_Sin", "Costo_Con", "Tiempo"])
            
            # Si el archivo está vacío o solo tiene el header, saltar
            if df_temp.empty:
                continue

            mean_costo = df_temp["Costo_Con"].mean()
            std_costo = df_temp["Costo_Con"].std()
            mean_tiempo = df_temp["Tiempo"].mean()
            
            data_records.append({
                "Instancia": instancia,
                "Zonas": zonas,
                "Alpha": alpha,
                "Costo_Promedio": mean_costo,
                "Costo_Std": std_costo,
                "Tiempo_Promedio": mean_tiempo
            })
            count_read += 1
        except Exception as e:
            print(f"Error leyendo {file_path}: {e}")

    print(f"Se procesaron correctamente {count_read} archivos.")
    return pd.DataFrame(data_records)

def plot_by_instance(df):
    instancias = df["Instancia"].unique()
    
    if len(instancias) == 0:
        print("No hay instancias para graficar.")
        return

    for inst in instancias:
        print(f"Generando gráfico para instancia: {inst}")
        
        df_inst = df[df["Instancia"] == inst].sort_values(by=["Zonas", "Alpha"])
        
        fig, axes = plt.subplots(1, 2, figsize=(15, 6))
        
        # Gráfico 1: Costo
        sns.lineplot(
            data=df_inst, 
            x="Zonas", 
            y="Costo_Promedio", 
            hue="Alpha", 
            palette="viridis", 
            marker="o",
            ax=axes[0]
        )
        axes[0].set_title(f"Costo vs Zonas (por Alpha)\nMapa: {inst}")
        axes[0].set_ylabel("Costo Promedio (Varianza Total)")
        axes[0].set_xlabel("Número de Zonas (p)")
        
        # Gráfico 2: Tiempo
        sns.lineplot(
            data=df_inst, 
            x="Zonas", 
            y="Tiempo_Promedio", 
            hue="Alpha", 
            palette="magma", 
            marker="s",
            ax=axes[1]
        )
        axes[1].set_title(f"Tiempo Ejecución vs Zonas\nMapa: {inst}")
        axes[1].set_ylabel("Tiempo Promedio (segundos)")
        axes[1].set_xlabel("Número de Zonas (p)")
        
        plt.tight_layout()
        
        safe_name = inst.replace("/", "_").replace("\\", "_")
        filename_out = f"grafico_{safe_name}.png"
        folder_out = "graficos/medianos"
        os.makedirs(folder_out, exist_ok=True)
        filename_out = os.path.join(folder_out, filename_out)
        plt.savefig(filename_out)
        print(f"  -> Guardado: {filename_out}")
        plt.close()

if __name__ == "__main__":
    df_resultados = read_experiment_data()
    
    if not df_resultados.empty:
        plot_by_instance(df_resultados)
        df_resultados.to_csv("resumen_experimentos_medianos.csv", index=False)
        print("\nTabla resumen guardada en 'resumen_experimentos_medianos.csv'")
    else:
        print("No se encontraron datos válidos en test_results/ (DataFrame vacío).")