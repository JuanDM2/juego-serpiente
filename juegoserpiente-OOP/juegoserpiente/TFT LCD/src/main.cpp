#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include "Sprite.h"

#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12

#define botonRight 18
#define botonLeft 19
#define botonDown 21
#define botonUp 20
#define BUZZER_PIN 9 // Define el pin al que está conectado el buzzer
#define IMG_WIDTH 100  // Ancho del sprite
#define IMG_HEIGHT 100 // Alto del sprite


const int XMAX = 240;
const int YMAX = 320;
const int SIZE = 10;
const int MAX_SNAKE_LENGTH = 100; // Tamaño máximo de la serpiente

Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// Estructura para representar posiciones en la pantalla
struct Posicion {
    int x;
    int y;
};

// Variables del juego
Posicion comida;
Posicion snake[MAX_SNAKE_LENGTH]; // Arreglo de posiciones para la serpiente
int snakeLength = 1; // Longitud inicial de la serpiente
uint8_t direccion = 2; // Derecha por defecto
unsigned long tiempoInicio;
int puntaje = 0; // Variable para el puntaje
bool nivel2 = false; // Bandera para indicar si estamos en nivel 2
bool nivel3 = false;  // Bandera para indicar si estamos en nivel 3
bool fin = false;


// Variables para las líneas paralelas
int y1 = 80;  // Coordenada Y de la primera línea horizontal
int y2 = 260; // Coordenada Y de la segunda línea horizontal
int grosorLinea = 10; // Grosor de la línea
int longitudLinea = 120; // Longitud de la línea
int xCentro = (XMAX - longitudLinea) / 2;  // Centrado horizontalmente
int xVertical = (XMAX - grosorLinea) / 2;  // Centrado en X para la línea vertical
int yVerticalInicio = 120;  // Coordenada Y de inicio de la línea vertical
int yVerticalFin = YMAX - 100;  // Coordenada Y de fin de la línea vertical

// Declaración de funciones
void generarComida();
void moverSnake();
void mostrar_info_juego(); // Función para mostrar tiempo y puntaje
void verificarColisiones();
void dibujarSnake();
void dibujarParedes(); // Nueva función para dibujar las paredes
void dibujarLineasParalelas(); // Función para dibujar dos líneas paralelas
void gameOver();
void resetGame();
void mostrarMensajeNivel2();
void dibujarObstaculosNivel2();
void mostrarMensajeNivel3();
void dibujarObstaculosNivel3();
void mostrarMensajeFin();
void sonidoInicio();
void sonidoComer();
void sonidoGameOver();
void detenerSonido();
void victorySound();
void reiniciarJuego();

// Función setup
void setup() {
Serial.begin(9600);
Serial.println("Juego Snake");
screen.fillScreen(ILI9341_GREEN);

// Esperar a que el jugador presione cualquier botón antes de continuar
Serial.println("¡¡¡¡¡¡¡¡¡¡¡¡¡¡Bienvenido¡¡¡¡¡¡¡¡¡");
Serial.println("Presiona cualquier boton para jugar");

screen.begin();
// Mostrar imagen o mensaje de bienvenida en la pantalla
screen.drawBitmap(20, 50, ini, 200, 200, ILI9341_GREEN);

// Esperar que cualquier botón sea presionado
while (true) {
    if (digitalRead(botonRight) == HIGH || digitalRead(botonLeft) == HIGH || digitalRead(botonUp) == HIGH || digitalRead(botonDown) == HIGH) {
        break;  // Rompe el bucle si se presiona un botón
    }
}

// Inicializar la pantalla y empezar el juego solo después de presionar un botón
screen.fillScreen(ILI9341_BLACK);

// Configurar pines de los botones con interrupciones
attachInterrupt(digitalPinToInterrupt(botonRight), []() { direccion = 2; }, HIGH);
attachInterrupt(digitalPinToInterrupt(botonLeft), []() { direccion = 3; }, HIGH);
attachInterrupt(digitalPinToInterrupt(botonUp), []() { direccion = 0; }, HIGH);
attachInterrupt(digitalPinToInterrupt(botonDown), []() { direccion = 1; }, HIGH);

// Posición inicial de la serpiente
snake[0] = {140, 160};

// Generar primera comida y tiempo inicial
generarComida();
tiempoInicio = millis();

// Dibujar las paredes en el setup
dibujarParedes();

// Dibujar las líneas paralelas en el nivel 1
dibujarLineasParalelas();

// Llamar sonido de inicio
sonidoInicio();

}

// Función loop
void loop() {
    moverSnake();
    mostrar_info_juego();  // Mostrar tiempo y puntaje
    delay(150); // Velocidad del juego
    
}


// Función para reproducir un sonido cuando la serpiente come la comida
void sonidoComer() {
    tone(BUZZER_PIN, 1000, 200); // Tono de 1000Hz por 200ms
}


// Función para reproducir un sonido de inicio
void sonidoInicio() {
    tone(BUZZER_PIN, 1500, 300); // Tono de 1500Hz por 300ms
}

void victorySound() {
    // Tono 1: 1000Hz durante 100ms
    tone(BUZZER_PIN, 1000, 100);  
    delay(100);  // Esperar 100 ms

    // Tono 2: 1200Hz durante 100ms
    tone(BUZZER_PIN, 1200, 100);
    delay(100);  // Esperar 100 ms

    // Tono 3: 1500Hz durante 150ms
    tone(BUZZER_PIN, 1500, 150);
    delay(150);  // Esperar 150 ms

    // Tono 4: 2000Hz durante 300ms
    tone(BUZZER_PIN, 2000, 300);
    delay(300);  // Esperar 300 ms

    // Final: Apagar el buzzer
    noTone(BUZZER_PIN);
}

// Función para reproducir un sonido de Game Over
void gameOverSound() {
    // Tono 1: 300Hz durante 300ms
    tone(BUZZER_PIN, 300, 300);  
    delay(300);  // Esperar 300 ms

    // Tono 2: 350Hz durante 150ms
    tone(BUZZER_PIN, 350, 150);
    delay(150);  // Esperar 150 ms

    // Tono 3: 250Hz durante 400ms
    tone(BUZZER_PIN, 250, 400);
    delay(400);  // Esperar 400 ms

    // Tono 4: 200Hz durante 500ms
    tone(BUZZER_PIN, 200, 500);
    delay(500);  // Esperar 500 ms

    // Final: Apagar el buzzer
    noTone(BUZZER_PIN);
}

void winSound() {
    // Tono 1: 500Hz durante 500ms (sonido de victoria largo)
    tone(BUZZER_PIN, 500, 500);
    delay(500);  // Esperar 500 ms

    // Tono 2: 550Hz durante 300ms
    tone(BUZZER_PIN, 550, 300);
    delay(300);  // Esperar 300 ms

    // Tono 3: 600Hz durante 500ms
    tone(BUZZER_PIN, 600, 500);
    delay(500);  // Esperar 500 ms

    // Tono 4: 650Hz durante 600ms
    tone(BUZZER_PIN, 650, 600);
    delay(600);  // Esperar 600 ms

    // Tono 5: 700Hz durante 700ms (nota más alta)
    tone(BUZZER_PIN, 700, 700);
    delay(700);  // Esperar 700 ms

    // Tono 6: 750Hz durante 800ms
    tone(BUZZER_PIN, 750, 800);
    delay(800);  // Esperar 800 ms

    // Final: Apagar el buzzer
    noTone(BUZZER_PIN);
}


// Función para detener el sonido
void detenerSonido() {
    noTone(BUZZER_PIN); // Detiene cualquier tono en el buzzer
}

// Definir los obstáculos del nivel 2
struct Obstaculo {
    int x, y, ancho, alto;
};

// Lista de obstáculos
Obstaculo obstaculos[] = {
   
    {0, 0, XMAX, 30},     // Pared superior
    {0, YMAX - SIZE, XMAX, SIZE},  // Pared inferior
    {0, 0, SIZE, YMAX},      // Pared izquierda
    {XMAX - SIZE, 0, SIZE, YMAX},  // Pared derecha
    //OBTACULOS NIVEL 1
    {xCentro, y1, longitudLinea, grosorLinea},  // Línea horizontal 1
    {xCentro, y2, longitudLinea, grosorLinea},  // Línea horizontal 2
    {xVertical, yVerticalInicio, grosorLinea, yVerticalFin - yVerticalInicio}, // Línea vertical
    //OBTACULOS NIVEL 2
    {60, 120, 60, SIZE},    // Obstáculo 
    {180, 150, 60, SIZE},   // Obstáculo
    {120, 210, 60, SIZE},    // Obstáculo  
    {0, 180, 60, SIZE},   // Obstáculo
     //OBTACULOS NIVEL 3
    {200, 180, SIZE, 110},   // Obstáculo 
    {200, 70, SIZE, 60},     // Obstáculo 
    {30, 220, SIZE, 60},     // Obstáculo 
    {115, 290, SIZE, 20},    // Obstáculo 
    {115, 30, SIZE, 30}     // Obstáculo 
};
bool colisionaConSerpiente(int x, int y) {
    for (int i = 0; i < snakeLength; i++) {
        if (snake[i].x == x && snake[i].y == y) {
            return true; // La comida está en una posición ocupada por la serpiente
        }
    }
    return false; // No colisiona con la serpiente
}

// Función para verificar si la comida colide con algún obstáculo
bool colisionaConObstaculos(int x, int y) {
    // Lista de obstáculos (todos los rectángulos)
    for (int i = 0; i < static_cast<int>(sizeof(obstaculos) / sizeof(obstaculos[0])); i++) {
        // Obtener los límites de cada obstáculo
        int xObstaculo = obstaculos[i].x;
        int yObstaculo = obstaculos[i].y;
        int anchoObstaculo = obstaculos[i].ancho;
        int altoObstaculo = obstaculos[i].alto;

        // Verificar si la comida colisiona con el obstáculo
        if (x >= xObstaculo && x < xObstaculo + anchoObstaculo &&
            y >= yObstaculo && y < yObstaculo + altoObstaculo) {
            return true; // Colisiona con el obstáculo
        }
    }

    return false; // No colisiona con ningún obstáculo
}
// Función para generar la comida asegurándose de que no esté en un obstáculo
void generarComida() {
    bool comidaGenerada = false;

    while (!comidaGenerada) {
        comida.x = random(1, (XMAX / SIZE) - 2) * SIZE;
        comida.y = random(1, ((YMAX- 40) / SIZE) - 2) * SIZE;

       // Serial.print("Generando comida en: ");
       // Serial.print(comida.x);
       // Serial.print(", ");
       // Serial.println(comida.y);

        // Verificar si la comida colisiona con la serpiente
        if (colisionaConSerpiente(comida.x, comida.y)) {
            continue; // Si colide, intentamos nuevamente
        }

        // Verificar si la comida colisiona con los obstáculos
        if (colisionaConObstaculos(comida.x, comida.y)) {
            continue; // Si colide, intentamos nuevamente
        }

        // Si no colide, la comida es válida
        comidaGenerada = true;
    }

    // Dibujar la comida
    screen.fillRect(comida.x, comida.y, SIZE, SIZE, ILI9341_GREEN);
}


// Función para mover la serpiente en la dirección actual
void moverSnake() {
    // Borrar la última posición de la serpiente (excepto la cabeza)
    screen.fillRect(snake[snakeLength - 1].x, snake[snakeLength - 1].y, SIZE, SIZE, ILI9341_BLACK);
    
    // Mover el cuerpo de la serpiente
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    
    // Calcular nueva posición de la cabeza
    Posicion nuevaPos = snake[0];
    switch (direccion) {
        case 0: nuevaPos.y -= SIZE; break; // Arriba
        case 1: nuevaPos.y += SIZE; break; // Abajo
        case 2: nuevaPos.x += SIZE; break; // Derecha
        case 3: nuevaPos.x -= SIZE; break; // Izquierda
    }
    snake[0] = nuevaPos;

    // Verificar si la serpiente ha comido la comida
    if (nuevaPos.x == comida.x && nuevaPos.y == comida.y) {
        if (snakeLength < MAX_SNAKE_LENGTH) {
            snakeLength++;
        }
        puntaje++; // Incrementar el puntaje
        sonidoComer();
        generarComida(); // Generar nueva comida
        
    }

    // Si el puntaje llega a 5, pasar al nivel 2
    if (puntaje >= 4 && !nivel2) {
        nivel2 = true;
        mostrarMensajeNivel2();
        dibujarObstaculosNivel2(); // Dibujar nuevos obstáculos del nivel 2
        dibujarParedes();           // Redibujar las paredes
        dibujarLineasParalelas();   // Redibujar las líneas paralelas
    }

    // Si el puntaje llega a 10, pasar al nivel 3
    if (puntaje >= 8 && !nivel3) {
        nivel3 = true;
        mostrarMensajeNivel3();
        dibujarObstaculosNivel3(); // Dibujar nuevos obstáculos del nivel 3
        dibujarParedes();           // Redibujar las paredes
        dibujarObstaculosNivel2();
        dibujarLineasParalelas();   // Redibujar las líneas paralelas
    }

    verificarColisiones();  // Verificar las colisiones en cada nivel
    dibujarSnake();         // Dibujar la serpiente
    if (puntaje >= 12 && !fin) {
        fin = true;
        mostrarMensajeFin();

        //Esperar hasta que el usuario presione un boton
         while (true) {
        if (digitalRead(botonRight) == HIGH || digitalRead(botonLeft) == HIGH || digitalRead(botonUp) == HIGH || digitalRead(botonDown) == HIGH) {
            reiniciarJuego();  // Reiniciar el juego
            break;
        }
    }
    }

    verificarColisiones();  // Verificar las colisiones en cada nivel
    dibujarSnake();         // Dibujar la serpiente
}


// Función para dibujar la serpiente
void dibujarSnake() {
    for (int i = 0; i < snakeLength; i++) {
        screen.fillRect(snake[i].x, snake[i].y, SIZE, SIZE, ILI9341_GREEN);
    }
    screen.fillRect(comida.x, comida.y, SIZE, SIZE, ILI9341_RED); // Redibujar la comida
}

// Función para mostrar el tiempo jugado y el puntaje en la parte superior derecha
void mostrar_info_juego() {
    unsigned long tiempoActual = millis();
    unsigned long tiempoJugado = (tiempoActual - tiempoInicio) / 1000;

    // Limpiar solo el área donde se muestra el puntaje
    screen.fillRect(70, 0, 70, 30, ILI9341_BLACK);  // Limpiar área del puntaje
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(2);
    screen.setCursor(20, 5);  // Posición para mostrar la palabra "Score:"
    screen.print("Score: ");
    screen.setCursor(95, 5);  // Posición para mostrar el valor del puntaje
    screen.print(puntaje);

    // Mostrar el tiempo en la parte superior derecha
    screen.fillRect(XMAX - 65, 0, 50, 30, ILI9341_BLACK);  // Limpiar área del tiempo
    screen.setCursor(XMAX - 95, 5);  // Posición para mostrar el tiempo en la parte superior derecha
    screen.print("T: ");
    screen.setCursor(XMAX - 65, 5);  // Posición para mostrar el valor del tiempo
    screen.print(tiempoJugado);
    screen.print("s");
}

void verificarColisiones() {
    Posicion& cabeza = snake[0];
      // Imprimir las coordenadas de la cabeza de la serpiente para depuración
    //Serial.print("Cabeza en: x=");
    //Serial.print(cabeza.x);
    //Serial.print(", y=");
    //Serial.println(cabeza.y);
    // Colisión con las paredes (bordes)
    if (cabeza.x < SIZE || cabeza.x >= XMAX - SIZE || cabeza.y < 30 + SIZE || cabeza.y >= YMAX - SIZE) {  // La pared superior está ajustada a 30
        gameOver();
        
    }

    // Colisión con el cuerpo
    for (int i = 1; i < snakeLength; i++) {
        if (cabeza.x == snake[i].x && cabeza.y == snake[i].y) {
            gameOver();
        }
    }

    // Colisiones con las líneas paralelas (estas no cambian, siguen activas en todos los niveles)
    if ((cabeza.y >= y1 && cabeza.y < y1 + grosorLinea && cabeza.x >= xCentro && cabeza.x < xCentro + longitudLinea) ||
        (cabeza.y >= y2 && cabeza.y < y2 + grosorLinea && cabeza.x >= xCentro && cabeza.x < xCentro + longitudLinea) ||
        (cabeza.x >= xVertical && cabeza.x < xVertical + grosorLinea && cabeza.y >= yVerticalInicio && cabeza.y < yVerticalFin)) {
        gameOver();
    }


// Verificar colisiones con los obstáculos del nivel 2, solo si estamos en nivel 2
    if (nivel2) {
         //Serial.println("Comprobando colisiones con los obstáculos del nivel 2...");
    
        // Obstáculo 1: (60, 120) -> ocupa las coordenadas (60,120) hasta (60+60,120+10)
        if (cabeza.x >= 60 && cabeza.x < 60 + 60 && cabeza.y >= 120 && cabeza.y < 120 + SIZE) {
            gameOver();
        }

        // Obstáculo 2: (180, 150) -> ocupa las coordenadas (180,150) hasta (180+60,150+10)
        if (cabeza.x >= 180 && cabeza.x < 180 + 60 && cabeza.y >= 150 && cabeza.y < 150 + SIZE) {
            gameOver();
        }

        if (cabeza.x >= 120 && cabeza.x < 120 + 60 && cabeza.y >= 210 && cabeza.y < 210 + SIZE) {
            gameOver();
        }
        if (cabeza.x >= 0 && cabeza.x < 0 + 60 && cabeza.y >= 180 && cabeza.y < 180 + SIZE) {
            gameOver();
        }
    }

   // Verificar colisiones con los obstáculos del nivel 3
    if (nivel3) {
        // Imprimir las coordenadas de los obstáculos para depuración
        //Serial.println("Comprobando colisiones con los obstáculos del nivel 3...");
    
        // Obstáculo 2 (coordenadas 200, 180, tamaño 60x110)
        if (cabeza.x >= 200 && cabeza.x < 200 + SIZE && cabeza.y >= 180 && cabeza.y < 180 + 110) {
            //Serial.println("Colisión con el Obstáculo 2 del nivel 3");
            gameOver();
        }

        // Obstáculo 3 (coordenadas 200, 70, tamaño 60x60)
        if (cabeza.x >= 200 && cabeza.x < 200 + SIZE && cabeza.y >= 70 && cabeza.y < 70 + 60) {
            //Serial.println("Colisión con el Obstáculo 3 del nivel 3");
            gameOver();
        }

        // Obstáculo 4 (coordenadas 30, 220, tamaño 60x60)
        if (cabeza.x >= 30 && cabeza.x < 30 + SIZE && cabeza.y >= 220 && cabeza.y < 220 + 60) {
            //Serial.println("Colisión con el Obstáculo 4 del nivel 3");
            gameOver();
        }

        // Obstáculo 5 (coordenadas 115, 290, tamaño 60x20)
        if (cabeza.x >= 115 && cabeza.x < 115 + SIZE && cabeza.y >= 290 && cabeza.y < 290 + 20) {
            //Serial.println("Colisión con el Obstáculo 5 del nivel 3");
            gameOver();
        }

        // Obstáculo 6 (coordenadas 115, 30, tamaño 60x30)
        if (cabeza.x >= 115 && cabeza.x < 115 +SIZE && cabeza.y >= 30 && cabeza.y < 30 + 30) {
            //Serial.println("Colisión con el Obstáculo 6 del nivel 3");
            gameOver();
        }
    } 
    
    }
// Función para mostrar "Game Over"
void gameOver() {
    // Reproducir el sonido de Game Over
    gameOverSound();
    
    // Mostrar pantalla de Game Over
    screen.fillScreen(ILI9341_RED);  // Fondo rojo para indicar que el juego terminó
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(3);
    screen.setCursor(50, YMAX / 2 - 20);
    screen.print("Game Over");
     screen.fillScreen(ILI9341_BLACK);
     screen.drawBitmap(20, 50, perder, 200, 180, ILI9341_WHITE);

    // Enviar mensaje por Serial
    Serial.println("HAZ PERDIDO LA BATALLA, PERO NO LA GUERRA");
     Serial.println("--------------------------------------------------------------");
    Serial.println("Presiona cualquier boton para volver a jugar");

    // Esperar a que el usuario presione un botón para reiniciar
    while (true) {
        if (digitalRead(botonRight) == HIGH || digitalRead(botonLeft) == HIGH || digitalRead(botonUp) == HIGH || digitalRead(botonDown) == HIGH) {
            reiniciarJuego();  // Reiniciar el juego
            break;
        }
    }
}

void reiniciarJuego() {
    // Limpiar la pantalla
    screen.fillScreen(ILI9341_BLACK);

    // Reiniciar las variables del juego
    snakeLength = 1;  // Longitud inicial de la serpiente
    puntaje = 0;  // Reiniciar el puntaje
    nivel2 = false;  // Reiniciar el nivel
    nivel3 = false;  // Reiniciar el nivel

    // Posición inicial de la serpiente
    snake[0] = {140, 160};

    // Generar la primera comida
    generarComida();

    // Restablecer el tiempo
    tiempoInicio = millis();

    // Dibujar las paredes y líneas paralelas nuevamente
    dibujarParedes();
    dibujarLineasParalelas();

    // Llamar sonido de inicio
    sonidoInicio();

    // Reiniciar el juego y comenzar desde el inicio
    loop();  // Volver a ejecutar el loop (reinicia el juego)
}

// Función para mostrar el mensaje de Nivel 2
void mostrarMensajeNivel2() {
    victorySound();
    screen.fillScreen(ILI9341_BLUE);
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(3);
    screen.setCursor(50, YMAX / 2 - 20);
    screen.print("Nivel 2");
    delay(2000);  // Esperar 2 segundos

    // Limpiar mensaje de éxito
    screen.fillScreen(ILI9341_BLACK);
}
void mostrarMensajeNivel3() {
    victorySound();
    screen.fillScreen(ILI9341_BLUE); // Fondo azul para nivel 3
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(3);
    screen.setCursor(50, YMAX / 2 - 20);
    screen.print("Nivel 3");
    delay(2000);  // Esperar 2 segundos

    // Limpiar mensaje 
    screen.fillScreen(ILI9341_BLACK);
}
void mostrarMensajeFin() {
    winSound();
    screen.fillScreen(ILI9341_BLUE); // Fondo azul para nivel Fin
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(3);
    screen.setCursor(50, YMAX / 2 - 20);
    screen.print("GANASTE");
    screen.fillScreen(ILI9341_BLACK);
    screen.drawBitmap(20, 50, win, 200, 200, ILI9341_WHITE);
    screen.fillScreen(ILI9341_BLACK);
    screen.drawBitmap(20, 50, winner, 200, 200, ILI9341_WHITE);
    delay(2000);  // Esperar 2 segundos
    
    
    // Enviar mensaje por Serial
    Serial.println("!!!FELICIDADES!!!, Has completado el juego como todo un gurerro");
    Serial.println("-----------------------------------------------------------------");
    Serial.println("Presiona cualquier boton para volver a jugar");

} 

// Función para dibujar los nuevos obstáculos en el nivel 2
void dibujarObstaculosNivel2() {
    // Nuevos obstáculos, líneas adicionales o rectángulos
    screen.fillRect(60, 120, 60, SIZE, ILI9341_BLUE); // Obstáculo en el nivel 2
    screen.fillRect(180, 150, 60, SIZE, ILI9341_BLUE); // Otro obstáculo en el nivel 2
    screen.fillRect(120, 210, 60, SIZE, ILI9341_BLUE); // Más obstáculos en el nivel 2
    screen.fillRect(0, 180, 60, SIZE, ILI9341_BLUE); // Más obstáculos en el nivel 2
   
}
void dibujarObstaculosNivel3() {
    // Dibujar los obstáculos del nivel 3
   // screen.fillRect(30, 60, SIZE, 100, ILI9341_BLUE); // Ejemplo de obstáculo MELO
    screen.fillRect(200, 180, SIZE, 110, ILI9341_BLUE); // Ejemplo de obstáculo MELO
    screen.fillRect(200, 70, SIZE, 60, ILI9341_BLUE); // Ejemplo de obstáculo MELO
    screen.fillRect(30, 220, SIZE, 60, ILI9341_BLUE); // Ejemplo de obstáculo MELO
    screen.fillRect(115, 290, SIZE, 20, ILI9341_BLUE); // Ejemplo de obstáculo MELO  
    screen.fillRect(115, 30, SIZE, 30, ILI9341_BLUE); // Ejemplo de obstáculo MELO
}
// Función para dibujar las paredes (bordes)
void dibujarParedes() {
    // Dibujar pared superior, comenzando desde Y=30 para dejar espacio para los textos
    screen.fillRect(0, 30, XMAX, SIZE, ILI9341_BLUE); // Pared superior (ajustada)

    // Dibujar la pared inferior
    screen.fillRect(0, YMAX - SIZE, XMAX, SIZE, ILI9341_BLUE); // Pared inferior

    // Dibujar pared izquierda
    screen.fillRect(0, 0, SIZE, YMAX, ILI9341_BLUE); // Pared izquierda

    // Dibujar pared derecha
    screen.fillRect(XMAX - SIZE, 0, SIZE, YMAX, ILI9341_BLUE); // Pared derecha
}

// Función para dibujar las líneas paralelas
void dibujarLineasParalelas() {
    // Dibujar la primera línea horizontal
    screen.fillRect(xCentro, y1, longitudLinea, grosorLinea, ILI9341_BLUE);
    
    // Dibujar la segunda línea horizontal
    screen.fillRect(xCentro, y2, longitudLinea, grosorLinea, ILI9341_BLUE);

   // screen.drawBitmap(xCentro, y2, trin , 120, 73,ILI9341_ORANGE);
    
    // Dibujar la línea vertical
    screen.fillRect(xVertical, yVerticalInicio, grosorLinea, yVerticalFin - yVerticalInicio, ILI9341_BLUE);
}