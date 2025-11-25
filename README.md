USART1 sin nada <- FALTA
  USART3 con los 2 primeros como en las diapositivas, y los ultimos 3 sin nada <= BUSCARLO

I2C1 como en el documento Pb8 PB9 BIEN

SPI1 como en las anteriores prácticas BIEN


LOS HILOS DE TEST ES MEJOR DEJARLOS COMENTADOS

----------------------------------------------------------------------------------------- comentarios 21/11
RELOJ <- se gestiona con un hilo, no hat ThTEST al ser variables globales

LCD
ThTEST Generas L1, L2, esperamos 2 segundos y mandamos mensaje  ThTest PUT
ThLCD wait mensaje e invoco la fnción                           GET

Joystick mensaje con información relativa al gesto iria a una cola        PUT
donde se recuperaría el  mensaje (en una ventana de watches vale) en el   ThTest GET
