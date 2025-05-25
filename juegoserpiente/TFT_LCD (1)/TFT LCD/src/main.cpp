#include <SPI.h>                     // permite comunicacion spi entre el micro y la pantalla
#include <Adafruit_GFX.h>          // funciones de graficos basicos lineas texto etc
#include <Adafruit_ILI9341.h>       // comandos especificos para el controlador de pantalla

#define TFT_DC 7                  // definicion de pines para pantalla, botones y buzzer
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12

#define botonRight 18
#define botonLeft 19
#define botonDown 21
#define botonUp 20
#define BUZZER_PIN 9

// dimenciones de la pantalla 240*320
const int XMAX = 240;
const int YMAX = 320;
const int SIZE = 10;  // tamaño de cada comida y tamaño inicial de la serpiente
const int MAX_SNAKE_LENGTH = 100;  // tamaño maximo que puede tener la serpiente

//creamos un objeto pantalla que llamamos screen, en posicion almacenamos las cordenadas de x , y para sp y comida
Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

struct Posicion {
    int x;
    int y;
};

Posicion comida;  // guardamos la posicion de la comida
Posicion snake[MAX_SNAKE_LENGTH]; 
int snakeLength = 1;     // tamaño actual de la serpiente
uint8_t direccion = 2;   // direccion de la serpiente, arriba abajo etc
unsigned long tiempoInicio;   // calculamos el tiempo que lleva en juego
int puntaje = 0;       // cuantas comidas ha recogido, inicia en 0
bool fin = false;    // inidica si el juego termino 

// aqui encontramos las posiciones o dirrecciones donde se encuentran las paredes u obstaculos
int y1 = 80;    //altura de la primera linea horizontal
int y2 = 260;   // segunda linea horizontal inferior
int grosorLinea = 10;
int longitudLinea = 120;
int xCentro = (XMAX - longitudLinea) / 2;  // calculo para encontrar el centro y poder centrar las lineas (240 - 120)/2 = 60 (comenzaria en 60 terminaria en 120)
int xVertical = (XMAX - grosorLinea) / 2;  //(240-10)/2=115
int yVerticalInicio = 120;
int yVerticalFin = YMAX - 100;  // (320-100) entonces va desde 120 hasta 220 pixeles de h
// declaramos las funciones que usaremos luego
void generarComida();
void moverSnake();
void mostrar_info_juego();
void verificarColisiones();
void dibujarSnake();
void dibujarParedes();
void dibujarLineasParalelas();
void gameOver();
void reiniciarJuego();
void sonidoInicio();
void sonidoComer();
void gameOverSound();

// apartir de aqui comienza la inicializacion del juego
void setup() {
    Serial.begin(9600);
    screen.begin();
    screen.fillScreen(ILI9341_BLACK);
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(2);
    screen.setCursor(10, 140);
    screen.print("Bienvenido");
    screen.setCursor(10, 160);
    screen.print("Presiona una tecla para iniciar");
// con esta parte evaluamos si el jugador presiono alguna tecla para iniciar el juego. con el break rompemos el ciclo para continuar
    while (true) {
        if (digitalRead(botonRight) == HIGH || digitalRead(botonLeft) == HIGH || 
            digitalRead(botonUp) == HIGH || digitalRead(botonDown) == HIGH) {
            break;
        }
    }
// limpiamos la pantalla 
    screen.fillScreen(ILI9341_BLACK);
// asociamos los botones con interrupciones externas ;  cambiamos la variable a la direccion correspondiente
    attachInterrupt(digitalPinToInterrupt(botonRight), []() { direccion = 2; }, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonLeft), []() { direccion = 3; }, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonUp), []() { direccion = 0; }, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonDown), []() { direccion = 1; }, HIGH);
// posicion inicial de la serpiente, llamamos funciones
    snake[0] = {140, 160};
    generarComida();
    tiempoInicio = millis();  // medimos el tiempo exacto donde inicia
    dibujarParedes();   // crea los 4 bordes externos 
    dibujarLineasParalelas(); // dibuja todas las lineas de adentro 
    sonidoInicio();  // llamamos el sonido al iniciar el juego 
}
// aqui llamamos las funciones que mueven la sp, se dibuja y actualiza el puntaje
void loop() {
    moverSnake(); // lo que involucre a la serpiente
    mostrar_info_juego(); // pantalla superior datos
    delay(150); // velocidad de movimiento 
}

void sonidoComer() {
    tone(BUZZER_PIN, 1000, 200);
}

void sonidoInicio() {
    tone(BUZZER_PIN, 1500, 300);
}
// este es un arreglo de frecuencias con el fin de simular un sonido mas concreto al chocar
void gameOverSound() {
    tone(BUZZER_PIN, 300, 300);  
    delay(300);
    tone(BUZZER_PIN, 350, 150);
    delay(150);
    tone(BUZZER_PIN, 250, 400);
    delay(400);
    tone(BUZZER_PIN, 200, 500);
    delay(500);
    noTone(BUZZER_PIN);
}
// verifica si x o y colisiona con los limites del juego o paredes internas
bool colisionaConSerpiente(int x, int y) {    // verifica si x o y coinciden con algun segmento del la sp
    for (int i = 0; i < snakeLength; i++) {
        if (snake[i].x == x && snake[i].y == y) {
            return true;
        }
    }
    return false;
}

bool colisionaConObstaculos(int x, int y) {
    if (x < SIZE || x >= XMAX - SIZE || y < 30 + SIZE || y >= YMAX - SIZE) {  // evita que la sp se salga del area de juego o colisiones con paredes
        return true;
    }
    if ((y >= y1 && y < y1 + grosorLinea && x >= xCentro && x < xCentro + longitudLinea) ||     // verifica que y este dentro  del grosor de la linea horizontal y1
        (y >= y2 && y < y2 + grosorLinea && x >= xCentro && x < xCentro + longitudLinea) ||
        (x >= xVertical && x < xVertical + grosorLinea && y >= yVerticalInicio && y < yVerticalFin)) {
        return true;
    }
    return false;
}

// posicion aleatoria, revisar que no este en un obstaculo ni sobre la sp
void generarComida() {
    bool comidaGenerada = false;
    while (!comidaGenerada) {
        comida.x = random(1, (XMAX / SIZE) - 2) * SIZE;
        comida.y = random(1, ((YMAX - 40) / SIZE) - 2) * SIZE;
        if (!colisionaConSerpiente(comida.x, comida.y) &&    // se repite hasta que colisione con obstaculos o la misma sp
            !colisionaConObstaculos(comida.x, comida.y)) {
            comidaGenerada = true;
        }
    }
    screen.fillRect(comida.x, comida.y, SIZE, SIZE, ILI9341_GREEN);  // borra el ultipo segmento de la cola
}

// borrar el ultimo seg de la cola
void moverSnake() {
    screen.fillRect(snake[snakeLength - 1].x, snake[snakeLength - 1].y, SIZE, SIZE, ILI9341_BLACK);
    for (int i = snakeLength - 1; i > 0; i--) {  // desplaza el cuerdo de tal forma que cada segmento se mueve a a la posicion anterior
        snake[i] = snake[i - 1];
    }
   // calculamos la cabeza segun la direccion (ariiba ya lo usamos igualmente)
    Posicion nuevaPos = snake[0]; // actualiza la cabeza segun la nueva posicion 
    switch (direccion) {
        case 0: nuevaPos.y -= SIZE; break;
        case 1: nuevaPos.y += SIZE; break;
        case 2: nuevaPos.x += SIZE; break;
        case 3: nuevaPos.x -= SIZE; break;
    }
    snake[0] = nuevaPos;
    if (nuevaPos.x == comida.x && nuevaPos.y == comida.y) {  // si la cabeza coincide con la comida, suma punto, aumenta la sp, suena, mas comida
        if (snakeLength < MAX_SNAKE_LENGTH) {
            snakeLength++;
        }
        puntaje++;
        sonidoComer();
        generarComida();
    }
    // si hay colicion se dibuja la nueva posicion de la sp
    verificarColisiones();
    dibujarSnake();
}
// dibujamos la sp con cuadrados, de igual forma la comida
void dibujarSnake() {
    for (int i = 0; i < snakeLength; i++) {
        screen.fillRect(snake[i].x, snake[i].y, SIZE, SIZE, ILI9341_GREEN);
    }
    screen.fillRect(comida.x, comida.y, SIZE, SIZE, ILI9341_RED);
}
// calculo seg desde q comenzo el juego
void mostrar_info_juego() {
    unsigned long tiempoActual = millis();
    unsigned long tiempoJugado = (tiempoActual - tiempoInicio) / 1000;
    screen.fillRect(70, 0, 70, 30, ILI9341_BLACK); // borra el puntaje anterior
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(2);
    screen.setCursor(20, 5);  
    screen.print("Score: ");
    screen.setCursor(95, 5);
    screen.print(puntaje);    // muestra el puntaje actual
    screen.fillRect(XMAX - 65, 0, 50, 30, ILI9341_BLACK);  // muestra el tiempo en seg, este linea genera un rectangulo con esas dim 
    screen.setCursor(XMAX - 95, 5);
    screen.print("T: ");
    screen.setCursor(XMAX - 65, 5);
    screen.print(tiempoJugado);
    screen.print("s");
}
// si la cabeza toca obstaculo termina el juego
void verificarColisiones() {
    Posicion& cabeza = snake[0];
    if (colisionaConObstaculos(cabeza.x, cabeza.y)) {
        gameOver();
    }
    // si la cabeza toca su cuerpo pierde
    for (int i = 1; i < snakeLength; i++) {
        if (cabeza.x == snake[i].x && cabeza.y == snake[i].y) {
            gameOver();
        }
    }
}
// suena segun la frecuencia, realizamos el dibujo pintando pixeles esperar que se presione tecla para reiniciar
void gameOver() {
    gameOverSound();
    screen.fillScreen(ILI9341_BLACK);
    int centerX = XMAX / 2;  // buscamos el centro para dibujar la cara final
    int centerY = YMAX / 2;
    int r = 50;
    screen.drawCircle(centerX, centerY, r, ILI9341_WHITE);
    screen.drawLine(centerX - 20, centerY - 20, centerX - 10, centerY - 10, ILI9341_WHITE);  // en general son las coordenadas de los pixeles que pintamos para la cara
    screen.drawLine(centerX - 10, centerY - 20, centerX - 20, centerY - 10, ILI9341_WHITE);
    screen.drawLine(centerX + 10, centerY - 20, centerX + 20, centerY - 10, ILI9341_WHITE);
    screen.drawLine(centerX + 20, centerY - 20, centerX + 10, centerY - 10, ILI9341_WHITE);
    screen.drawLine(centerX - 15, centerY + 20, centerX + 15, centerY + 20, ILI9341_WHITE);
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(4);
    screen.setCursor(centerX - 90, centerY + 60);  // esta parte pinta el mensaje "you lose"
    screen.print("YOU LOSE");
    delay(2000);
    screen.fillScreen(ILI9341_BLACK);

// esperamos nuevamente a que el usuario presione una tecla para volver a iniciar 
    while (true) {
        if (digitalRead(botonRight) == HIGH || digitalRead(botonLeft) == HIGH || 
            digitalRead(botonUp) == HIGH || digitalRead(botonDown) == HIGH) {
            reiniciarJuego();
            break;
        }
    }
}

// long sp, puntaje, posicion inicial, tiempo de inicio obstaculos y pantalla se limpia para volver al inicio
void reiniciarJuego() {
    screen.fillScreen(ILI9341_BLACK);
    snakeLength = 1;
    puntaje = 0;
    snake[0] = {140, 160};
    generarComida();
    tiempoInicio = millis();
    dibujarParedes();
    dibujarLineasParalelas();
    sonidoInicio();
}
// crea los bordes azules 
void dibujarParedes() {
    screen.fillRect(0, 30, XMAX, SIZE, ILI9341_BLUE);  
    screen.fillRect(0, YMAX - SIZE, XMAX, SIZE, ILI9341_BLUE); 
    screen.fillRect(0, 0, SIZE, YMAX, ILI9341_BLUE);
    screen.fillRect(XMAX - SIZE, 0, SIZE, YMAX, ILI9341_BLUE);
}
// obstaculos internos
void dibujarLineasParalelas() {
    screen.fillRect(xCentro, y1, longitudLinea, grosorLinea, ILI9341_BLUE);
    screen.fillRect(xCentro, y2, longitudLinea, grosorLinea, ILI9341_BLUE);
    screen.fillRect(xVertical, yVerticalInicio, grosorLinea, yVerticalFin - yVerticalInicio, ILI9341_BLUE);
}