USART1 sin nada <- FALTA
  USART3 con los 2 primeros como en las diapositivas, y los ultimos 3 sin nada <= BUSCARLO

I2C1 como en el documento PB8 PB9 BIEN

SPI1 como en las anteriores prácticas BIEN

LOS HILOS DE TEST ES MEJOR DEJARLOS COMENTADOS LUEGO SE HAYA COMPROBADO
EL CORRECTO FUNCIONAMIENTO DE CADA MÓDULO


COMENTARIOS EN CLASE 21/11 + AJUSTES MENCIONADOS POR EL PROFESOR
----------------------------------------------------------------------------------------- 
RELOJ <- se gestiona con un hilo, no hat ThTEST al ser variables globales
COMENTARIOS PROFESOR: mejor tener una única variable, la de segundos

LCD
ThTEST Generas L1, L2, esperamos 2 segundos y mandamos mensaje  ThTest PUT
ThLCD wait mensaje e invoco la fnción                           GET
COMENTARIOS PROFESOR: Bajar tamaño de la cola y el tipo de mensaje (tener 2 variables en vez de cuatro)

Joystick mensaje con información relativa al gesto iria a una cola        PUT
donde se recuperaría el  mensaje (en una ventana de watches vale) en el   ThTest GET
COMENTARIOS PROFESOR: Bajar tamaño de la cola y tener el mínimo posible de timers virtuales (comentario de abajo)

A TENER EN CUENTA SIEMPRE
-----------------------------------------------------------------------------------------
EN GENERAL, INTENTAR CONSUMIR EL MENOR NÚMERO DE RECURSOS POSIBLES
- Agrupar funcionalidades en hilos
- Decrementar tamaño de colas, timers (virtuales) e hilos

CADA VEZ QUE CARGUES UN PROYECTO ACTUALIZADO ACUÉRDATE DE . . .
-----------------------------------------------------------------------------------------
En options to target, debbuger, añadir el ST-LINK
y en view activar que se vea al momento los cambios
