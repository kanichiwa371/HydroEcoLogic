

# HydroEcoLogic

## ¿Qué es HydroEcoLogic?

**HydroEcoLogic** es un algoritmo capaz de ajustarse a un tipo de cultivo, y regarlo **automáticamente** cuando sea necesario, permitiendo una mejora en la calidad de vida de los agricultores

## Contexto del proyecto

Decidimos hacer este proyecto debido a la necesidad de fincas que no pueden permitirse **altos costos** en sistemas de automatización, y que **ellos mismos** puedan montar una por su propia cuenta, y ajustar el código de la placa a su preferencia

### ¿Qué ventajas nos trae HydroEcoLogic?
Nos permite aparte de, lo anteriormente mencionado, ahorrar tiempo y comodidad, tambien nos permite gastar menos agua, lo que nos haria ser mas eficientes con el consumo del agua y, a la larga, ahorrar dinero en gastos de agua.

### La solución
La solución creada fue HydroEcoLogic, un algoritmo que toma variables del entorno (humedad del suelo, humedad del aire, temperatura) y **decide** cuando es correcto regar y por cuánto tiempo, y se autoalimenta por **energia solar**, todo completamente automático, mas, tiene **conectividad a Internet**, por lo cuál, nosotros desde nuestro móvil podemos hacer que abra el riego o lo cierre, o que nos envie las condiciones del huerto y nosotras verla en el móvil

### ¿Porque es código abierto?
A diferencia de los sistemas comerciales, HydroEcoLogic es **100% abierto**. Esto significa que:

-   Cualquier agricultor puede **descargarlo gratis** desde GitHub.
-   Puede **modificarlo** a sus necesidades (cambiar umbrales, tiempos, añadir sensores).
-   No depende de una empresa que pueda desaparecer o subir los precios.
    
-   La comunidad puede **mejorarlo** entre todos.

### ¿Que sucede si falla?
El sistema está diseñado para no dejar la planta sin agua ni encharcarla:

-   **Tiempo máximo de riego:** Si el sensor falla, la bomba se apaga sola a los 15 segundos.
    
-   **Detección de condiciones extremas:** No riega si hace mucho calor (>35°C) o si el aire está muy húmedo (>80%), evitando hongos como la _Sigatoka negra_.
    
-   **Modo manual:** Si no confías en el automático, puedes regar tú mismo desde el móvil.

 ----



#### ¿Que componentes necesitamos para hacer mi propio HydroEcoLogic?

Muy sencillo, necesitaremos los siguientes **componentes** para hacer un sistema

- Sensor de humedad del suelo
- Sensor de temperatura y humedad del aire
- Placa ESP32 con conectividad a internet
- Electroválvula
- Bomba de agua (opcional, pero estaria bien para que el agua tenga presión)
- Placa fotovoltaica
- Protoboard donde realizar las conexiones
- Y resto de componentes que necesitamos, del tipo tuberías, cajas donde poner los componentes, la tierra...

Una vez tengamos todo esto, empezemos con el proceso de instalación

## Lógica de control

El sistema opera bajo una lógica basada en umbrales. El ESP32 lee los sensores en **tiempo real**, y decide si es necesario regar, con el código recién descargado del repositorio, **regará en las siguientes condiciones**:

1.- La humedad del suelo es inferior al 30%
2.- La temperatura ambiente es inferior a 35ºC
3.- La humedad del aire es inferior al 80%

Estas condiciones de riego **evita** que se riegue ineficientemente, pero si crees que es necesario, tu mismo puedes abrir el sistema de riego desde tu móvil. 

## Instalación de HydroEcoLogic

**1ºer paso:** Necesitaremos coger la placa arduino y una protoboard, conectaremos la placa a un dispositivo y compilaremos el código, pondremos un código sencillo para comprobar que la placa funcione, probaremos a conectarla a la red y pasaremos al siguiente paso

> ¿Cómo conecto la placa a la red?
> Cuando la placa se encienda, creara una red a la cual necesitaremos conectarnos 

**2ºnd paso:** Una vez conectada, necesitaremos coger sitios donde vaya el agua, la tierra (si no lo tenemos, podemos perfectamente diseñarlos en 3D, el tanque de agua, soportes para los componentes...) y conectaremos el tanque con el huerto con tuberias, poniendo la electroválvula y la minibomba en el sistema de tuberías, y llevamos una conexión de la electroválvula hacia la Arduino, ponemos el pin y lo especificamos en el código.
**3ºer paso:** Instalaremos los sensores, y los conectaremos a la placa arduino, ajustando igual que antes con la electroválvula, los pines en el código.

Y listo, tenemos toda la parte física, vayamos a la digital.

**4ºto paso:** Tendremos que crear un bot de telegram y poner el token del bot en el código fuente, luego, encendemos la placa y debería enviar los datos por el chat de telegram

Listo, tenemos nuestro propio HydroEcoLogic listo para probar, cambiar, manipular o lo que quieras hacer.