# Códigos para Runibot 2025

## Sobre el proyecto

Este repositorio contiene los códigos desarrollados para:

- **Futbolistas de 4 ruedas**, controlados mediante mandos de **Xbox Series X/S** y **PS5**.
- **Velocista seguidor de línea**, controlado desde una **aplicación móvil** mediante conexión WiFi.

### Entorno de desarrollo

- IDE de Arduino: **Versión 2.3.4**
- Librería de Espressif para ESP32: **Versión 2.0.11**
- Enlace para instalar el soporte de placas ESP32 en el IDE:  
  `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

---

## Importante

### ⚽ Futbolistas

- Es **obligatorio especificar la dirección MAC** de cada control al inicio del código para que puedan conectarse correctamente.
- En los controles **Xbox**, es necesario tener el firmware actualizado. De lo contrario, la tarjeta puede quedarse en un bucle infinito con el mensaje `Waiting Notification`.
- Al ejecutar una animación, la respuesta del robot puede **no ser inmediata**.

### 🚗 Velocista Seguidor de Línea

- Se utilizó la librería **QTRSensors16** (modificación de la versión para 8 sensores). Esta se encuentra en la **carpeta raíz del proyecto** y debe importarse manualmente al IDE de Arduino.

  ![Importar librería](https://github.com/AnaOrozco122002/Codigos_Runibot_2025/blob/master/Velocista_WIFI/images/add.jpg)

- El sistema usa **WiFi**, por lo que el celular debe conectarse primero al **Access Point creado por la tarjeta**.
- La aplicación móvil es compatible con dispositivos Android **versión 8 o superior**.

---

## 📦 Cómo importar la librería `QTRSensors16`

La librería `QTRSensors16` es una versión modificada de la librería estándar de sensores QTR. Esta versión permite el uso de 16 sensores y es necesaria para el funcionamiento del **Velocista Seguidor de Línea**.

### Pasos para instalarla en el IDE de Arduino

1. **Ubica la carpeta `QTRSensors16`** en la raíz del proyecto (debe estar junto al archivo `.ino` principal).
2. Abre el **IDE de Arduino**.
3. Ve al menú **"Programa" → "Incluir Librería" → "Añadir biblioteca .ZIP..."**.
4. En el explorador de archivos que se abre, selecciona la carpeta `QTRSensors16` o comprímela en `.zip` y selecciona el archivo ZIP.
5. Verifica que la instalación se haya completado correctamente:  
   Ve a **"Programa" → "Incluir Librería"** y busca `QTRSensors16` en la lista.

---

## Pines utilizados

### ⚽ Futbolistas

- **PWM (control de velocidad)**:
  - Motor A: GPIO 13  
  - Motor B: GPIO 14  
  - Motor C: GPIO 23  
  - Motor D: GPIO 32

- **Dirección (control de sentido)**:
  - Motor A: GPIOs 12 y 27  
  - Motor B: GPIOs 4 y 5  
  - Motor C: GPIOs 18 y 19  
  - Motor D: GPIOs 26 y 25

- **Pantalla OLED**: Se usan los pines por defecto del bus I2C del ESP32.

### 🚗 Velocista Seguidor de Línea

- **Sensor QTR**:
  - Entrada analógica: A2  
  - Multiplexor (digitales): D9, D10, D0, D1

- **Motores**:
  - PWM:
    - Motor Derecho: D8  
    - Motor Izquierdo: D6
  - Dirección:
    - Motor Derecho: D7  
    - Motor Izquierdo: D5

- **Otros**:
  - Turbina: D4  
  - Módulo de inicio: D3

---

## ¿Cómo funciona?

### ⚽ Futbolistas

#### 🔗 Conexión de controles

- **PS5**: Mantén presionados los botones **Share + PS (logo)** por 5 segundos hasta que el LED parpadee rápidamente. Cuando se conecte, el LED quedará con **color sólido**.
- **Xbox**: Enciende el control (botón Xbox), luego mantén presionado el botón superior (Share) por 5 segundos hasta que el LED parpadee rápidamente. Al conectarse, el LED quedará en **blanco sólido**.

#### 🕹️ Manejo

- **Avanzar**: Gatillo derecho (RT en Xbox / R2 en PS5)  
- **Retroceder**: Gatillo izquierdo (LT en Xbox / L2 en PS5)  
- **Girar**: Joystick izquierdo en la dirección deseada (si se desea girar en reversa, mantener el gatillo izquierdo presionado)  
- **Animaciones**:
  - Botón superior derecho → Cara enojada 😠  
  - Botón derecho → Cara neutra 🙂  
  - Botón inferior → Parpadeo 👀  
  - Combinación botón derecho + flecha arriba → Animación especial 🎉

---

### 🚗 Velocista Seguidor de Línea

- Conecta el celular al **WiFi creado por la tarjeta**.
- La aplicación cuenta con botones de **inicio/parada**, que funcionarán correctamente **solo si el módulo de inicio está desactivado**.
