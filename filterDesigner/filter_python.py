import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import firwin, freqz

# Especificaciones del filtro
fs = 44100  # Frecuencia de muestreo
f1, f2 = 100, 400  # Frecuencias de corte

# Diseño del filtro rechazabanda usando firwin
numtaps = (
    501  # Número de coeficientes. Puede ajustarse para obtener diferentes respuestas.
)
coeff = firwin(numtaps, [f1, f2], pass_zero=True, fs=fs)

# Verificar la respuesta en frecuencia
w, h = freqz(coeff, worN=8000, fs=fs)
plt.plot(w, 20 * np.log10(np.abs(h)))
plt.title("Respuesta en Frecuencia del Filtro FIR")
plt.xlabel("Frecuencia (Hz)")
plt.ylabel("Atenuación (dB)")
plt.grid()
plt.show()
