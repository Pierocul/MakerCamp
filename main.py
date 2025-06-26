from machine import I2C, Pin
import time
import struct

# Configuración I2C: SDA=GP0, SCL=GP1
i2c = I2C(0, scl=Pin(1), sda=Pin(0), freq=100_000)

# Dirección del sensor SCD30
SCD30_ADDR = 0x61

# Función para enviar comandos al sensor
def write_cmd(cmd):
    msb = (cmd >> 8) & 0xFF
    lsb = cmd & 0xFF
    i2c.writeto(SCD30_ADDR, bytes([msb, lsb]))

# Iniciar medición continua
write_cmd(0x0010)  # Comando: Start Continuous Measurement
time.sleep(2)

# Función para leer datos del sensor
def read_measurement():
    i2c.writeto(SCD30_ADDR, b'\x03\x00')  # Comando para pedir datos
    time.sleep(0.005)
    data = i2c.readfrom(SCD30_ADDR, 18)  # Se esperan 18 bytes

    # Desempaquetar los datos (en formato IEEE 754 big-endian)
    co2 = struct.unpack('>f', data[0:4])[0]
    temp = struct.unpack('>f', data[6:10])[0]
    hum = struct.unpack('>f', data[12:16])[0]
    return co2, temp, hum

# Bucle principal
while True:
    try:
        co2, temp, hum = read_measurement()
        print("CO2:", round(co2), "ppm | Temp:", round(temp, 1), "°C | Hum:", round(hum), "%")
    except Exception as e:
        print("Error leyendo el sensor:", e)

    time.sleep(5)
