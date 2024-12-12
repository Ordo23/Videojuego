#include "SPI.h"
#include "Adafruit_GFX.h" 
#include "Adafruit_ILI9341.h"
#include "Sprite.h"
// Definiciones de pines
#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12
#define botonRight 18
#define botonLeft 19
#define botonUp 20
const int XMAX = 240; // Alto del display
const int YMAX = 320; // Ancho del display
// Variables globales de juego
int x = 0;
int y = 0;
int yInicial = 0;
// Variables de punto de partida y llegada
int puntoPartidaX, puntoPartidaY;
int puntoLlegadaX, puntoLlegadaY;
const uint8_t UP = 0;
const uint8_t RIGHT = 1;
const uint8_t LEFT = 2;
// Instancia de pantalla
Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);
GFXcanvas1 canvas(32, 32);
// Estructuras de juego
struct Coin {
    int x;
    int y;
    bool collected;
};

struct Platform {
    int x;
    int y;
    int width;
    bool isObstacle;
};
// Variables de estado del juego
int vidas = 2;
int tiempo = 150;
int puntos = 0;
unsigned long ultimoTiempo = 0;
unsigned long ultimaMoneda = 0;
int monedasRecolectadas = 0;
const int MAX_COINS = 5;
const int MAX_PLATFORMS = 4;
Coin coins[MAX_COINS];
Platform platforms[MAX_PLATFORMS];
// Variables de estado del jugador
bool saltando = false;
enum PlayerState { IDLE, RUNNING, JUMPING };
PlayerState estadoJugador = IDLE;
bool mirandoDerecha = true;
// Declaraciones de funciones (mantener igual)
void setPlayerPosition(int x, int y);
void animatePlayer();
void moverPlayer(uint8_t direccion);
void moverPlayerDerecha(void);
void moverPlayerIzquierda(void);
void moverPlayerArriba(void);
void drawUI();
void generateCoins();
void generatePlatforms();
void drawCoins();
void drawPlatforms();
void checkCollisions();
void gameOver(bool won);
void drawLevelCheckpoints();
void generateStrategicPlatforms();
void generateStrategicCoins();
// Implementación de funciones
// Posicionar jugador
void setPlayerPosition(int x1, int y1) {
    x = x1;
    y = y1;
}
// Animar jugador
void animatePlayer() {
    int count = 0;
    //while (true) {
        // Lógica para el salto
        if (saltando) {
            y -= 4; 
            if (y <= yInicial - 80) {
                saltando = false;
                estadoJugador = IDLE; // Cambia a quieto al finalizar salto
            }
        } else if (y < yInicial) {
            y += 4; 
        } else {
            y = yInicial;
        }

        // Selección de animación según el estado y la dirección
        const uint8_t* sprite;
        if (mirandoDerecha) {
            switch (estadoJugador) {
                case IDLE:
                    sprite = Player_1_Der[count]; 
                    break;
                case RUNNING:
                    sprite = Player_Run_Der[count];
                    break;
                case JUMPING:
                    sprite = Player_Jump_Der;
                    break;
            }
        } else {
            switch (estadoJugador) {
                case IDLE:
                    sprite = Player_1_Izq[count]; 
                    break;
                case RUNNING:
                    sprite = Player_Run_Izq[count];
                    break;
                case JUMPING:
                    sprite = Player_Jump_Izq;
                    break;
            }
        }

    // Renderizar sprite
    canvas.fillScreen(0);
    canvas.drawBitmap(0, 0, sprite, 23, 32, ILI9341_WHITE);
    screen.fillRect(x, y, 23, 32, ILI9341_BLACK);
    screen.drawBitmap(x, y, canvas.getBuffer(), canvas.width(), canvas.height(), ILI9341_YELLOW, ILI9341_BLACK);

    count = (count + 1) % 2;
    delay(50);

    // Reset a estado idle
    if (!saltando && digitalRead(botonRight) == LOW && digitalRead(botonLeft) == LOW) {
        estadoJugador = IDLE;
    //}
}
}
// Mover jugador a la derecha
void moverPlayerDerecha() {
    delay(50);
    if (digitalRead(botonRight) == HIGH) {
        Serial.println("Boton RIGHT");
        moverPlayer(RIGHT);
        mirandoDerecha = true;       // Actualiza la dirección
        estadoJugador = RUNNING;     // Cambia a correr
    }    
}
// Mover jugador a la izquierda
void moverPlayerIzquierda() {
    delay(50);
    if (digitalRead(botonLeft) == HIGH) {
        Serial.println("Boton LEFT");
        moverPlayer(LEFT);
        mirandoDerecha = false;      // Actualiza la dirección
        estadoJugador = RUNNING;     // Cambia a correr
    }
}
// Mover jugador hacia arriba (salto)
void moverPlayerArriba() {
    if (!saltando) {
        saltando = true;
        estadoJugador = JUMPING; // Cambia a saltar
        Serial.println("Salto");
    }
}
// Mueve el jugador
void moverPlayer(uint8_t direccion) {
    uint8_t delta = 2;
    switch (direccion) {
        case RIGHT:
            x = x + delta;
            break;
        case LEFT:
            x = x - delta;
            break;
    }
}
// Dibujar interfaz de usuario
void drawUI() {
    screen.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    screen.setTextSize(1);
    
    // Tiempo
    screen.fillRect(10, 5, 100, 15, ILI9341_BLACK);
    screen.setCursor(10, 5);
    screen.print("Time: ");
    screen.print(tiempo);
    
    // Vidas
    screen.fillRect(10, 25, 100, 15, ILI9341_BLACK);
    screen.setCursor(10, 25);
    screen.print("Lives: ");
    screen.print(vidas);
    
    // Puntos
    screen.fillRect(10, 45, 100, 15, ILI9341_BLACK);
    screen.setCursor(10, 45);
    screen.print("Score: ");
    screen.print(puntos);
}

void drawLevelCheckpoints() {
    // Punto de partida en verde
    screen.fillRect(puntoPartidaX, puntoPartidaY, 30, 10, ILI9341_GREEN);
    
    // Punto de llegada en rojo
    screen.fillRect(puntoLlegadaX, puntoLlegadaY, 30, 10, ILI9341_RED);
}

void generateCoins() {
    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].x = random(20, XMAX - 20);
        coins[i].y = random(100, YMAX - 100);
        coins[i].collected = false;
    }
}

void generatePlatforms() {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        platforms[i].x = random(20, XMAX - 50);
        platforms[i].y = random(100, YMAX - 100);
        platforms[i].width = random(30, 80);
        platforms[i].isObstacle = false;
    }
}

void generateStrategicPlatforms() {
    platforms[0].x = 20;
    platforms[0].y = YMAX * 0.8;
    platforms[0].width = 60;

    platforms[1].x = XMAX - 80;
    platforms[1].y = YMAX * 0.6;
    platforms[1].width = 60;

    platforms[2].x = 40;
    platforms[2].y = YMAX * 0.4;
    platforms[2].width = 50;

    platforms[3].x = XMAX - 90;
    platforms[3].y = YMAX * 0.2;
    platforms[3].width = 50;
}

void generateStrategicCoins() {
    coins[0].x = platforms[0].x + platforms[0].width / 2;
    coins[0].y = platforms[0].y - 10;

    coins[1].x = platforms[1].x + platforms[1].width / 2;
    coins[1].y = platforms[1].y - 10;

    coins[2].x = platforms[2].x + platforms[2].width / 2;
    coins[2].y = platforms[2].y - 10;

    coins[3].x = platforms[3].x + platforms[3].width / 2;
    coins[3].y = platforms[3].y - 10;

    coins[4].x = puntoLlegadaX + 15;
    coins[4].y = puntoLlegadaY - 10;

    for (int i = 0; i < MAX_COINS; i++) {
        coins[i].collected = false;
    }
}

void drawCoins() {
    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].collected) {
            screen.fillCircle(coins[i].x, coins[i].y, 5, ILI9341_YELLOW);
        }
    }
}

void drawPlatforms() {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        screen.fillRect(platforms[i].x, platforms[i].y, platforms[i].width, 10, ILI9341_GREEN);
    }
}

void checkCollisions() {
    // Colisión con monedas
    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].collected && 
            x < coins[i].x + 10 && x + 23 > coins[i].x &&
            y < coins[i].y + 10 && y + 32 > coins[i].y) {
            coins[i].collected = true;
            puntos += 10;
            monedasRecolectadas++;
            
            // Agregar vida cada 10 monedas
            if (monedasRecolectadas % 10 == 0 && vidas < 3) {
                vidas++;
            }
        }
    }

    // Colisión con plataformas (igual que antes)
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (x < platforms[i].x + platforms[i].width && 
            x + 23 > platforms[i].x &&
            y + 32 > platforms[i].y && 
            y + 32 < platforms[i].y + 10) {
            y = platforms[i].y - 32;
        }
    }
}

void gameOver(bool won) {
    screen.fillScreen(ILI9341_BLACK);
    screen.setCursor(30, 150);
    screen.setTextSize(2);
    
    if (won) {
        screen.setTextColor(ILI9341_GREEN);
        screen.println("YOU WIN!");
    } else {
        screen.setTextColor(ILI9341_RED);
        screen.println("GAME OVER");
    }
    
    screen.setCursor(30, 200);
    screen.setTextSize(1);
    screen.print("Score: ");
    screen.print(puntos);
    
    // Detener el juego
    while(true);
}

void setup() {
    Serial.begin(9600);
    
    // Configuración de interrupciones
    attachInterrupt(digitalPinToInterrupt(botonRight), moverPlayerDerecha, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonLeft), moverPlayerIzquierda, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonUp), moverPlayerArriba, HIGH);

    screen.begin();
    screen.setRotation(0);
    screen.fillScreen(ILI9341_BLACK);

    // Definir puntos de partida y llegada
    puntoPartidaX = 10;
    puntoPartidaY = YMAX - 20;
    puntoLlegadaX = XMAX - 40;
    puntoLlegadaY = 20;

    // Posición inicial del jugador (importante para respetar la mecánica original)
    x = puntoPartidaX;
    y = YMAX - 32;  // Ajustado para estar en el punto de partida
    yInicial = y;   // Establecer altura inicial

        // Posicionar jugador inicial
    setPlayerPosition(x, y);

    randomSeed(analogRead(0));

    // Generar elementos del juego
    generateStrategicPlatforms();
    generateStrategicCoins();
}

void loop() {   
    // Dibujar puntos de partida y llegada
    drawLevelCheckpoints();
    
    // Dibujar elementos del juego
    drawCoins();
    drawPlatforms();


    

    // Dibujar interfaz
    drawUI();

    // Animación del jugador
    animatePlayer();

    // Verificar colisiones
    checkCollisions();

if (millis() - ultimoTiempo >= 1000) {
    ultimoTiempo = millis();
    tiempo--;
    
}

if (tiempo <= 0) {
        gameOver(false); // Termina el juego si el tiempo llega a 0
    }

    if (x >= puntoLlegadaX && x <= puntoLlegadaX + 30 && y <= puntoLlegadaY + 10) {
        gameOver(true);
    }
}