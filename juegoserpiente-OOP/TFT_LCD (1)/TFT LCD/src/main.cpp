#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Definición de pines
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
#define BUZZER_PIN 9

// Dimensiones de la pantalla
const int XMAX = 240;
const int YMAX = 320;
const int SIZE = 10;
const int MAX_SNAKE_LENGTH = 100;

// Obstáculos
const int y1 = 80;
const int y2 = 260;
const int grosorLinea = 10;
const int longitudLinea = 120;
const int xCentro = (XMAX - longitudLinea) / 2;
const int xVertical = (XMAX - grosorLinea) / 2;
const int yVerticalInicio = 120;
const int yVerticalFin = YMAX - 100;

// Declaración adelantada para las interrupciones
class Juego;
extern Juego* juegoGlobal;

// Clase Posición
class Posicion {
public:
    int x;
    int y;
    
    Posicion(int x = 0, int y = 0) : x(x), y(y) {}
};

// Clase Serpiente
class Serpiente {
private:
    Posicion cuerpo[MAX_SNAKE_LENGTH];
    int longitud;
    uint8_t direccion;
    
public:
    Serpiente() : longitud(1), direccion(2) {
        cuerpo[0] = Posicion(140, 160);
    }
    
    void mover(Adafruit_ILI9341 &pantalla) {
        // Borrar la cola
        pantalla.fillRect(cuerpo[longitud-1].x, cuerpo[longitud-1].y, SIZE, SIZE, ILI9341_BLACK);
        
        // Mover el cuerpo
        for (int i = longitud-1; i > 0; i--) {
            cuerpo[i] = cuerpo[i-1];
        }
        
        // Mover la cabeza según la dirección
        switch(direccion) {
            case 0: cuerpo[0].y -= SIZE; break; // Arriba
            case 1: cuerpo[0].y += SIZE; break; // Abajo
            case 2: cuerpo[0].x += SIZE; break; // Derecha
            case 3: cuerpo[0].x -= SIZE; break; // Izquierda
        }
    }
    
    void crecer() {
        if (longitud < MAX_SNAKE_LENGTH) {
            cuerpo[longitud] = cuerpo[longitud-1];
            longitud++;
        }
    }
    
    void dibujar(Adafruit_ILI9341 &pantalla) {
        for (int i = 0; i < longitud; i++) {
            pantalla.fillRect(cuerpo[i].x, cuerpo[i].y, SIZE, SIZE, ILI9341_GREEN);
        }
    }
    
    Posicion getCabeza() const { return cuerpo[0]; }
    int getLongitud() const { return longitud; }
    uint8_t getDireccion() const { return direccion; }
    void setDireccion(uint8_t dir) { 
        // Evitar movimiento opuesto directo
        if ((direccion == 0 && dir != 1) || (direccion == 1 && dir != 0) ||
            (direccion == 2 && dir != 3) || (direccion == 3 && dir != 2)) {
            direccion = dir; 
        }
    }
    
    bool colisionaConCuerpo(int x, int y) const {
        for (int i = 1; i < longitud; i++) {
            if (cuerpo[i].x == x && cuerpo[i].y == y) {
                return true;
            }
        }
        return false;
    }
    
    void reiniciar() {
        longitud = 1;
        cuerpo[0] = Posicion(140, 160);
        direccion = 2;
    }
};

// Clase Comida
class Comida {
private:
    Posicion posicion;
    
    bool colisionaConObstaculos(int x, int y) {
        if (x < SIZE || x >= XMAX - SIZE || y < 30 + SIZE || y >= YMAX - SIZE) {
            return true;
        }
        if ((y >= y1 && y < y1 + grosorLinea && x >= xCentro && x < xCentro + longitudLinea) ||
            (y >= y2 && y < y2 + grosorLinea && x >= xCentro && x < xCentro + longitudLinea) ||
            (x >= xVertical && x < xVertical + grosorLinea && y >= yVerticalInicio && y < yVerticalFin)) {
            return true;
        }
        return false;
    }
    
public:
    Comida() {
        generarNueva();
    }
    
    void generarNueva() {
        bool valida = false;
        while (!valida) {
            posicion.x = random(1, (XMAX / SIZE) - 1) * SIZE;
            posicion.y = random(1, ((YMAX - 40) / SIZE) - 1) * SIZE;
            valida = !colisionaConObstaculos(posicion.x, posicion.y);
        }
    }
    
    void dibujar(Adafruit_ILI9341 &pantalla) {
        pantalla.fillRect(posicion.x, posicion.y, SIZE, SIZE, ILI9341_RED);
    }
    
    Posicion getPosicion() const { return posicion; }
    
    bool verificarColision(int x, int y) {
        return colisionaConObstaculos(x, y);
    }
};

// Clase Juego
class Juego {
private:
    Adafruit_ILI9341 pantalla;
    Serpiente serpiente;
    Comida comida;
    int puntaje;
    unsigned long tiempoInicio;
    bool enJuego;
    
    void dibujarParedes() {
        pantalla.fillRect(0, 30, XMAX, grosorLinea, ILI9341_BLUE);
        pantalla.fillRect(0, YMAX - grosorLinea, XMAX, grosorLinea, ILI9341_BLUE);
        pantalla.fillRect(0, 0, grosorLinea, YMAX, ILI9341_BLUE);
        pantalla.fillRect(XMAX - grosorLinea, 0, grosorLinea, YMAX, ILI9341_BLUE);
    }
    
    void dibujarLineasParalelas() {
        pantalla.fillRect(xCentro, y1, longitudLinea, grosorLinea, ILI9341_BLUE);
        pantalla.fillRect(xCentro, y2, longitudLinea, grosorLinea, ILI9341_BLUE);
        pantalla.fillRect(xVertical, yVerticalInicio, grosorLinea, yVerticalFin - yVerticalInicio, ILI9341_BLUE);
    }
    
    void mostrarInformacion() {
        unsigned long tiempoActual = (millis() - tiempoInicio) / 1000;
        
        pantalla.fillRect(70, 0, 70, 30, ILI9341_BLACK);
        pantalla.setTextColor(ILI9341_WHITE);
        pantalla.setTextSize(2);
        
        pantalla.setCursor(20, 5);
        pantalla.print("Score: ");
        pantalla.setCursor(95, 5);
        pantalla.print(puntaje);
        
        pantalla.fillRect(XMAX - 65, 0, 50, 30, ILI9341_BLACK);
        pantalla.setCursor(XMAX - 95, 5);
        pantalla.print("T: ");
        pantalla.setCursor(XMAX - 65, 5);
        pantalla.print(tiempoActual);
        pantalla.print("s");
    }
    
    void sonidoComer() {
        tone(BUZZER_PIN, 1000, 200);
    }
    
    void sonidoInicio() {
        tone(BUZZER_PIN, 1500, 300);
    }
    
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
    
public:
    Juego() : pantalla(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO),
              puntaje(0), enJuego(false) {}
    
    void configurarControles() {
        attachInterrupt(digitalPinToInterrupt(botonRight), []() { 
            if (juegoGlobal->serpiente.getDireccion() != 3) 
                juegoGlobal->serpiente.setDireccion(2); 
        }, HIGH);
        
        attachInterrupt(digitalPinToInterrupt(botonLeft), []() { 
            if (juegoGlobal->serpiente.getDireccion() != 2) 
                juegoGlobal->serpiente.setDireccion(3); 
        }, HIGH);
        
        attachInterrupt(digitalPinToInterrupt(botonUp), []() { 
            if (juegoGlobal->serpiente.getDireccion() != 1) 
                juegoGlobal->serpiente.setDireccion(0); 
        }, HIGH);
        
        attachInterrupt(digitalPinToInterrupt(botonDown), []() { 
            if (juegoGlobal->serpiente.getDireccion() != 0) 
                juegoGlobal->serpiente.setDireccion(1); 
        }, HIGH);
    }
    
    void iniciar() {
        pantalla.begin();
        pantalla.fillScreen(ILI9341_BLACK);
        pantalla.setTextColor(ILI9341_WHITE);
        pantalla.setTextSize(2);
        
        pantalla.setCursor(10, 140);
        pantalla.print("Bienvenido");
        pantalla.setCursor(10, 160);
        pantalla.print("Presiona una tecla");
        
        while (true) {
            if (digitalRead(botonRight) == HIGH || digitalRead(botonLeft) == HIGH || 
                digitalRead(botonUp) == HIGH || digitalRead(botonDown) == HIGH) {
                break;
            }
        }
        
        pantalla.fillScreen(ILI9341_BLACK);
        configurarControles();
        
        tiempoInicio = millis();
        enJuego = true;
        
        dibujarParedes();
        dibujarLineasParalelas();
        sonidoInicio();
    }
    
    void actualizar() {
        if (!enJuego) return;
        
        serpiente.mover(pantalla);
        
        Posicion cabeza = serpiente.getCabeza();
        bool colisionObstaculo = comida.verificarColision(cabeza.x, cabeza.y);
        bool colisionCuerpo = serpiente.colisionaConCuerpo(cabeza.x, cabeza.y);
        
        if (colisionObstaculo || colisionCuerpo) {
            gameOver();
            return;
        }
        
        if (cabeza.x == comida.getPosicion().x && cabeza.y == comida.getPosicion().y) {
            serpiente.crecer();
            puntaje++;
            sonidoComer();
            comida.generarNueva();
        }
        
        serpiente.dibujar(pantalla);
        comida.dibujar(pantalla);
        mostrarInformacion();
    }
    
    void gameOver() {
        enJuego = false;
        gameOverSound();
        
        pantalla.fillScreen(ILI9341_BLACK);
        int centerX = XMAX / 2;
        int centerY = YMAX / 2;
        
        pantalla.drawCircle(centerX, centerY, 50, ILI9341_WHITE);
        pantalla.drawLine(centerX - 20, centerY - 20, centerX - 10, centerY - 10, ILI9341_WHITE);
        pantalla.drawLine(centerX - 10, centerY - 20, centerX - 20, centerY - 10, ILI9341_WHITE);
        pantalla.drawLine(centerX + 10, centerY - 20, centerX + 20, centerY - 10, ILI9341_WHITE);
        pantalla.drawLine(centerX + 20, centerY - 20, centerX + 10, centerY - 10, ILI9341_WHITE);
        pantalla.drawLine(centerX - 15, centerY + 20, centerX + 15, centerY + 20, ILI9341_WHITE);
        
        pantalla.setTextColor(ILI9341_WHITE);
        pantalla.setTextSize(4);
        pantalla.setCursor(centerX - 90, centerY + 60);
        pantalla.print("YOU LOSE");
        
        delay(2000);
        pantalla.fillScreen(ILI9341_BLACK);
        pantalla.setTextSize(2);
        pantalla.setCursor(50, 150);
        pantalla.print("Presiona para reiniciar");
        
        while (true) {
            if (digitalRead(botonRight) == HIGH || digitalRead(botonLeft) == HIGH || 
                digitalRead(botonUp) == HIGH || digitalRead(botonDown) == HIGH) {
                reiniciar();
                break;
            }
        }
    }
    
    void reiniciar() {
        serpiente.reiniciar();
        puntaje = 0;
        comida.generarNueva();
        tiempoInicio = millis();
        enJuego = true;
        
        pantalla.fillScreen(ILI9341_BLACK);
        dibujarParedes();
        dibujarLineasParalelas();
        sonidoInicio();
    }
};

// Variable global para las interrupciones
Juego* juegoGlobal;

// Instancia del juego
Juego juego;

void setup() {
    Serial.begin(9600);
    juegoGlobal = &juego;
    juego.iniciar();
}

void loop() {
    juego.actualizar();
    delay(150);
}