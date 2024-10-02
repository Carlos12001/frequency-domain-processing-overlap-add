import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import firwin, freqz

fs = 44100  # Suponiendo una frecuencia de muestreo de 44100 Hz

# Diseño de un filtro FIR que atenúa frecuencias en el rango de 500Hz a 1500Hz
h = firwin(numtaps=501, cutoff=[500, 1500], fs=fs, pass_zero=False)

print("Coeficientes del filtro:", h)

# Respuesta en frecuencia del filtro
w, H = freqz(h, worN=8000)
magnitude = 20 * np.log10(np.abs(H))

# Gráfico de la respuesta en frecuencia
plt.figure()
plt.plot(w * fs / (2 * np.pi), magnitude)
plt.title("Respuesta en Frecuencia")
plt.xlabel("Frecuencia [Hz]")
plt.ylabel("Magnitud [dB]")
plt.grid(True)
plt.show()
