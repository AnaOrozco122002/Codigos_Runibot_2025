# Códigos para Runibot 2025
### Sobre el código
----------------------------------

- Este es un código creado para los carros Futbolistas de 4 ruedas que se manejan con controles de Xbox Series X/S y PS5 y Seguidor de Linea que se maneja mediante una App
- La versión del IDE de Arduino utilizada fue la 2.3.4
- La versión de la libreria de Espressif para programación es la 2.0.11
- El Link para programar tarjetas Esp32:
`<link>` : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json


### Importante
----------------------------------

* Es necesario especificar la MAC de cada control al inicio del código para que se puedan conectar adecuadamente

* Es necesario en el control de Xbox actualizar el software a la ultima versión de lo contrario la tarjeta se puede quedar en un loop infinito de Waiting Notification

* Al utilizar alguna animación el código no responderá de manera inmediata. 

### Pines Usados
----------------------------------

Se utilizan los siguientes pines

* Pines de control del PWM: Motor A: 13, Motor B: 14, Motor C: 23, Motor D: 32
* Pines de control de dirección: Motor A: 12 y 27, Motor B: 4 y 5, Motor C: 18 y 19, Motor D: 26 y 25
* Para la pantalla se usan los pines por defecto para conexión por bus de la tarjeta esp32.

### Cómo funciona
----------------------------------

* ### Conectar Controles

- Para conectar el control de PS5 se debe oprimir el botón de Share y PS (Logo de playstation) durante 5 segundos hasta que empiece a parpadear de manera rapido, cuando se conecte tendra un color solido permanente indicando la conexión.

-Para conectar el control de Xbox se debe encender al oprimir el icono de xbox, cuando este parpadeando se debe oprimir el botón que esta en la parte superior (share) durante 5 segundos hasta que empiece a parpadear de manera rapida, cuando se conecte el control tendra un color solido de color blanco.

* ### Manejo

- Para andar hacia adelante se debe oprimir el gatillo derecho (Rt en Xbox y R2 en playstation)

- Para andar hacia atras se debe oprimir el gatillo Izquierdo (Lt en Xbox y L2 en playstation)

- Para girar se puede realizar girando el joystick izquierdo en la dirección de giro (para girar hacia atras es necesario oprimir el gatillo izquierdo)

- Para realizar las animaciones correspondientes se debe oprimir los botones ubicados en la parte derecha de los controles, al oprimir el superior se muestra una cara enojada, al oprimir el boton derecho es la cara por defecto, al oprimir el boton inferior parpadea y al oprimir la combinación de botones del boton derecho y la flecha hacia arriba se presenta la animación.