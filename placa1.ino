#include <SoftwareSerial.h> // Libreria de manejo de comunicacion serial alterna
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //libreria del i2c para la comunicacion simplificada del display
#include <Keypad.h>

// Constantes
static const byte MAX_DIG_LOTE = 4;         // Máximo dígitos permitido para lote
static const unsigned char N_0_ASCII = 48;  // Valor en ascii de "0" 
static const unsigned char N_9_ASCII = 57;  // Valor en ascii de "9"
static const unsigned char L_A_ASCII = 65;  // Valor en ascii de "A"
static const unsigned char L_B_ASCII = 66;  // Valor en ascii de "B"
static const unsigned char L_C_ASCII = 67;  // Valor en ascii de "C"
static const unsigned char L_D_ASCII = 68;  // Valor en ascii de "D"
static const unsigned char STX_ASCII = 123; // Valor en ascii de "{"
static const unsigned char ETX_ASCII = 125; // Valor en ascii de "}"

// Prototipo de funciones
int powint(int, int);
void getLoteNumber();
void processLoteNumber();
void lectura_datos();
void initLCD();
void serialFlush();

/* Variables globales */
// Teclado
bool modoLote;                              // Activar cuando se va a ingresar con lote
unsigned char nDigitos;                     // Contador de digitos ingresados
unsigned char loteArray[MAX_DIG_LOTE];      // Guarda los dígitos ingresados
unsigned int lote;                          // Guarda el valor de lote
unsigned char tecla;                        // Tecla ingresada
// LED
const int ledPin =  LED_BUILTIN;            // the number of the LED pin
int ledState = LOW;                         // ledState used to set the LED
unsigned long previousMillis;               // will store last time LED was updated
const long interval = 1000;                 // interval at which to blink (milliseconds)
// Menu
unsigned char menu;                         // Menu como máquina de estados
// Variables de lectura
float area;                                 // Lectura de acumulado mismo cuero
unsigned int areaFinal;                     // Valor total mismo cuero
unsigned long areaLote;                     // Sumatoria areas de lote
bool blinkingLed;                           // Para avisar cuando acabae lote
bool areaPreviaExistente;                   // Para saber si existe un areaPrevia guardada

/* Configuración del LCD */
LiquidCrystal_I2C lcd(0x27, 20, 4);

/* Comunicación serial arduino */
// Pines
const int RX_1 = 0; // Pin rx del arduino
const int TX_1 = 1; // Pin tx del arduino
// Declaracion de pines usados para la comunicaciony el nombre del puerto virtual (serialvirt)
SoftwareSerial Serialvirt(RX_1, TX_1); 

/* Teclado */
const byte filas = 4;
const byte columnas = 4;
byte pinesF[filas] = {11,10,9,8};
byte pinesC[columnas] = {7,6,5,3};
 
char teclas[filas][columnas] = { 
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

// Pone a funcionar lo de la libreria
Keypad teclado = Keypad(makeKeymap(teclas), pinesF, pinesC, filas, columnas);

void setup() {

    // Initialize serial and wait for port to open:
    Serialvirt.begin(9600); // Velocidad de comunicacion del otroarduino
    while (!Serialvirt) {
        ; // wait for serial port to connect. Needed for native USB
    }

    lcd.init();         // initialize the lcd 
    lcd.init();         // **???
    lcd.backlight();
    initLCD();          // Custom init function

    pinMode(8, INPUT);
    // set the digital pin as output:
    pinMode(ledPin, OUTPUT);   

    // Variables
    previousMillis = 0;
    menu = 0;
    lote = 0;
    nDigitos = 0;
    area = 0;
    areaFinal = 0;
    areaLote = 0;
    modoLote = false;
    blinkingLed = false;
    areaPreviaExistente = false;
}

void loop() {    
    // Obtiene tecla
    tecla = teclado.getKey();

    if(tecla != NO_KEY) {

        switch(menu) {

            // Inicio
            case 0:
                // Tecla A. Modo lote activado
                if(tecla == L_A_ASCII) {
                    nDigitos = 0;
                    lote = 0;
                    // Se evita que se escriba una area que ya no existe
                    areaPreviaExistente = false;

                    // Limpia el LCD
                    initLCD();

                    // Pone el cursor
                    lcd.setCursor(6, 2);
                    lcd.print("/    ");

                    menu = 1;
                    
                    // Tambien se deesbloquea lectura sin lote al intentar ingresar un lote
                    blinkingLed = false;
                    serialFlush();
                }
                // Tecla B. Desbloqueo luego de terminar un lote.
                else if(tecla == L_B_ASCII) {
                    // Apaga led titilante
                    blinkingLed = false;
                    // Limpia el LCD
                    initLCD();
                    // Limpiar el buffer serial
                    serialFlush();
                    // Se evita que se escriba una area que ya no existe
                    areaPreviaExistente = false;
                }
                break;

            // Se pide el valor del lote
            case 1:
                getLoteNumber();
                break;            

            default:
                break;
        }
    }

    // Lectura de datos en modoLote || sin lote
    // El modo sin lote es bloqueado luego de terminar un lote. (Presionar "B" para activar)
    if((modoLote && (Serialvirt.available() > 0)) || (!blinkingLed && (Serialvirt.available() > 0))) {
        lectura_datos();
    }

    if(blinkingLed) {
       unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= interval) {
            // save the last time you blinked the LED
            previousMillis = currentMillis;

            // if the LED is off turn it on and vice-versa:
            if (ledState == LOW) {
                ledState = HIGH;
            } else {
                ledState = LOW;
            }

            // set the LED with the ledState of the variable:
            digitalWrite(ledPin, ledState);
        }
    }
}

void getLoteNumber() {
    // Se entra si se pulsa un número 0-9
    if((tecla >= N_0_ASCII) && (tecla <= N_9_ASCII) && (nDigitos < MAX_DIG_LOTE)) {    // Max value 9.999
        loteArray[nDigitos] = (tecla - N_0_ASCII);      // ASCII -> num, luego se guarda dígito en array. 
        lcd.setCursor(6 + nDigitos, 2);
        lcd.print(loteArray[nDigitos]);
        if(nDigitos != MAX_DIG_LOTE - 1) lcd.print("/");   // Se imprime "/" si no es el último dígito permitido
        nDigitos++;
    }
    // Se borra el último dígito, si existe alguno
    else if((tecla == L_C_ASCII) && (nDigitos > 0)) {
        nDigitos--;
        loteArray[nDigitos] = 0;    // No necesario pero buena práctica
        lcd.setCursor(6 + nDigitos, 2);
        lcd.print("/ ");
    }
    // Se cancela el modo lote
    else if(tecla == L_D_ASCII) {
        nDigitos = 0;
        processLoteNumber();
    }
    // Se procesa el valor ingresado
    else if(tecla == L_B_ASCII) {
        processLoteNumber();
        // Se va a empezar un lote
        if(modoLote) {
            // Limpiar el buffer serial
            serialFlush();
            // Poner "0" en area y areaLote
            lcd.setCursor(6, 2);
            lcd.print("0   ");
            lcd.setCursor(12, 3);
            lcd.print("0       ");
        }
    }
}

void processLoteNumber() {
    // Siempre se vuelve al menú principal al puslar la B
    menu = 0;

    // Computa el valor del lote
    for(byte i = nDigitos; i > 0; i--) {
        lote = lote + loteArray[i - 1] * powint(10, nDigitos - i);            
    }

    // Limpia lote y área del LCD si se cancela el modo lote ó no se ingresa valor
    if(nDigitos == 0) {
        // Limpia lote anterior del LCD ó "desactivado"
        lcd.setCursor(6, 2);
        lcd.print("----");
        // Limpia el valor anterior de área
        lcd.setCursor(6, 0);
        lcd.print("----");

        // Exit this function, modoLote will be false.
        return;       
    }
    // Borra "/" cuando hubo menos de MAX_DIG_LOTE dígitos
    else if(nDigitos != MAX_DIG_LOTE) {
        lcd.setCursor(6 + nDigitos, 2);
        lcd.print(" ");
    }

    // Se llega acá sólo cuando se ingresó un valor válido
    modoLote = true;    
}

void lectura_datos() {

    byte incomingByte = Serialvirt.read();

    // Inicio de trama
    if(incomingByte == STX_ASCII) {
        area = 0;
        
        // AREA PREVIA
        // Antes de sobreescribir areaFinal, pasar este valor a areaPrevia
        if(areaPreviaExistente) {
            // Limipia area previa en LCD
            lcd.setCursor(13, 1);
            lcd.print("     ");
            lcd.setCursor(13, 1);
            lcd.print(areaFinal);
        }
    }
    // Fin de trama
    else if(incomingByte == ETX_ASCII) {
        areaFinal = (unsigned int)area;

        // AREA
        // Limipia area en LCD
        lcd.setCursor(6, 0);
        lcd.print("     ");
        // Imprime dato en LCD
        lcd.setCursor(6, 0);
        lcd.print(areaFinal);

        // A partir de ahora existirá un valor
        areaPreviaExistente = true;

        // Sólo para modoLote
        if(modoLote) {
            // LOTE
            // Actualiza el valor del lote
            lote--;
            lcd.setCursor(6, 2);
            lcd.print("    ");
            lcd.setCursor(6, 2);
            lcd.print(lote);

            // AREA TOTAL
            // Actualiza el valor del área total
            areaLote = areaLote + (long)areaFinal;
            lcd.setCursor(12, 3);
            lcd.print("        ");
            lcd.setCursor(12, 3);
            lcd.print(areaLote);

            // Termina lote, enciende piloto y bloquea lectura
            if(lote == 0) {
                modoLote = false;
                blinkingLed = true;
                areaLote = 0;
            }
        }
    }
    // Dato de trama
    else {
        area = area + ((float)incomingByte * 2.54 * 2.54 / 100.0);
        // Limipia area en LCD
        lcd.setCursor(6, 0);
        lcd.print("     ");
        // Imprime dato en LCD
        lcd.setCursor(6, 0);
        lcd.print((unsigned int)area);
    }
}

void serialFlush(){
    while(Serial.available() > 0) {
        char t = Serial.read();
    }
}

void initLCD() {
    lcd.setCursor(0, 0);
    lcd.print("AREA: ----");
    lcd.setCursor(0, 1);
    lcd.print("AREA PREVIA: ----");
    lcd.setCursor(0, 2);
    lcd.print("LOTE: ----");
    lcd.setCursor(0, 3);
    lcd.print("AREA TOTAL: ----");
}

int powint(int x, int y) {
    int val = x;
    for (int z = 0; z <= y; z++) {
        if (z == 0)
            val = 1;
        else
            val = val * x;
    }
    return val;
}
