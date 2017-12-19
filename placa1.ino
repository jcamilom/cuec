#include <SoftwareSerial.h> // Libreria de manejo de comunicacion serial alterna
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //libreria del i2c para la comunicacion simplificada del display
#include <Keypad.h>

// Constantes
static const byte MAX_DIG_LOTE = 4;         // Máximo dígitos permitido para lote
static const unsigned char L_AST_ASCII = 42;  // Valor en ascii de "*"
static const unsigned char N_0_ASCII = 48;  // Valor en ascii de "0" 
static const unsigned char N_9_ASCII = 57;  // Valor en ascii de "9"
static const unsigned char L_A_ASCII = 65;  // Valor en ascii de "A"
static const unsigned char L_B_ASCII = 66;  // Valor en ascii de "B"
static const unsigned char L_C_ASCII = 67;  // Valor en ascii de "C"
static const unsigned char L_D_ASCII = 68;  // Valor en ascii de "D"

// Prototipo de funciones
int powint(int, int);
void getLoteNumber();
void processLoteNumber();
void lectura_datos();
void initLCD();

/* Variables globales */
// Teclado
bool modoLote = false;                      // Activar cuando se va a ingresar con lote
unsigned char nDigitos;                 // Contador de digitos ingresados
unsigned char loteArray[MAX_DIG_LOTE];      // Guarda los dígitos ingresados
unsigned int lote;                      // Guarda el valor de lote
unsigned char tecla;                        // Tecla ingresada
// LED
const int ledPin =  LED_BUILTIN;        // the number of the LED pin
int ledState = LOW;                     // ledState used to set the LED
unsigned long previousMillis = 0;       // will store last time LED was updated
const long interval = 1000;             // interval at which to blink (milliseconds)
// Menu
unsigned char menu;                     // Menu como máquina de estados
bool leerDatos;
// Variables de lectura
unsigned int area;                      // Lectura de acumulado mismo cuero
unsigned int areaFinal;                 // Valor total mismo cuero
unsigned long areaLote;                 // Sumatoria areas de lote
bool cueroListo;
bool blinkingLed;                       // Para avisar cuando acabae lote
bool startLectura;                      // Quitar luego
bool trash = false;

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

Keypad teclado = Keypad(makeKeymap(teclas), pinesF, pinesC, filas, columnas); //pone a funcionar lo de la libreria


void setup() {

    //Initialize serial and wait for port to open:
    Serialvirt.begin(9600); // Velocidad de comunicacion del otroarduino
    while (!Serialvirt) {
        ; // wait for serial port to connect. Needed for native USB
    }
    // Wait until the trash data pass
    /* while(true) {
        if(Serialvirt.available() > 1) {
            byte b1 = Serialvirt.read();
            byte b2 = Serialvirt.read();
            unsigned int dato = ((unsigned int)b1) * 256 + b2;

            if(dato != 0) {
                trash = true;
            } else if(trash) {
                break;
            }
        }
    } */

    lcd.init(); // initialize the lcd 
    lcd.init();
    lcd.backlight();
    initLCD();

    pinMode(8, INPUT);
    // set the digital pin as output:
    pinMode(ledPin, OUTPUT);   

    // Variables
    menu = 0;
    lote = 0;
    nDigitos = 0;
    leerDatos = false;
    area = 0;
    areaFinal = 0;
    cueroListo = false;
    areaLote = 0;
    blinkingLed = false;
    startLectura = false;
}

void loop() {    
    // Obtiene tecla (si es pulsada)
    tecla = teclado.getKey();

    if (tecla != NO_KEY) {

        switch(menu) {

            // Inicio
            case 0:
                // Tecla A. Modo lote activado
                if(tecla == L_A_ASCII) {
                    modoLote = true;                // Activa el modo lote
                    nDigitos = 0;
                    lote = 0;

                    // Limpia el LCD
                    initLCD();

                    // Pone el cursor
                    lcd.setCursor(6, 2);
                    lcd.print("/    ");

                    menu = 1;
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

    // Se leen datos con el modoLote desactivado
    //if(!modoLote && (Serialvirt.available() > 0)) {
    /* if(!modoLote && (Serialvirt.available() > 0)) {
        initLCD();
        leerDatos = true;
        menu = 0; // quitar luego
        //area = 1;
        //areaFinal = 0;
        //Serial.print("Medición sin lote2\n");
        //startLectura = false;
    } */

    //if(leerDatos && (Serialvirt.available() > 1)) {
    //if(leerDatos && (Serialvirt.available() > 0)) {
    if((Serialvirt.available() > 1)) {
        lectura_datos();

        /* byte b11 = Serialvirt.read();
        byte b22 = Serialvirt.read();

        unsigned int arr = (b11 * 256) + b22;

        if(arr != 0) {
            Serialvirt.println(arr);
            Serialvirt.print("MSB (b1): ");
            Serialvirt.print(b11);
            Serialvirt.print(", LSB (b2): ");
            Serialvirt.print(b22);
            Serialvirt.println();
        } */
    }

/*     if(Serialvirt.available() > 1) {
        byte b1 = Serialvirt.read();
        byte b2 = Serialvirt.read();

        unsigned int areaInt = ((unsigned int)b1) * 256 + b2;
        
        // Limipia area en LCD
        lcd.setCursor(6, 0);
        lcd.print("     ");
        // Imprime dato en LCD
        lcd.setCursor(6, 0);
        lcd.print(b1);

        // Limipia area en LCD
        lcd.setCursor(13, 1);
        lcd.print("     ");
        // Imprime dato en LCD
        lcd.setCursor(13, 1);
        lcd.print(b2);

        // Limipia area en LCD
        lcd.setCursor(6, 2);
        lcd.print("     ");
        // Imprime dato en LCD
        lcd.setCursor(6, 2);
        lcd.print(areaInt);
    } */

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
    if ((tecla >= N_0_ASCII) && (tecla <= N_9_ASCII) && (nDigitos < MAX_DIG_LOTE)) {    // Max value 9.999
        loteArray[nDigitos] = (tecla - N_0_ASCII);      // ASCII -> num, luego se guarda dígito en array. 
        lcd.setCursor(6 + nDigitos, 2);
        lcd.print(loteArray[nDigitos]);
        if (nDigitos != MAX_DIG_LOTE - 1) lcd.print("/");   // Se imprime "-" si no es el último dígito permitido
        //Serial.println(loteArray[nDigitos]);
        nDigitos++;
    }
    // Se borra el último dígito, si existe alguno
    else if ((tecla == L_C_ASCII) && (nDigitos > 0)) {
        nDigitos--;
        loteArray[nDigitos] = 0;    // No necesario pero buena práctica
        lcd.setCursor(6 + nDigitos, 2);
        lcd.print("/ ");
    }
    // Se cancela el modo lote
    else if (tecla == L_D_ASCII) {
        modoLote = false;
        nDigitos = 0;
        lote = 0;                    
        processLoteNumber();
        leerDatos = false;
    }
    // Se procesa el valor ingresado
    else if(tecla == L_B_ASCII) {
        processLoteNumber();
        if(modoLote) {
            leerDatos = true;
            blinkingLed = false;
        }
    }
}

void processLoteNumber() {
    // Computa el valor del lote
    for (byte i = nDigitos; i > 0; i--) {
        lote = lote + loteArray[i - 1] * powint(10, nDigitos - i);            
    }

    // Limpia lote y área del LCD si se cancela el modo lote ó no se ingresa valor
    if ((nDigitos == 0) || !modoLote) {
        // Limpia lote anterior del LCD ó "desactivado"
        lcd.setCursor(6, 2);
        lcd.print("----       ");
        // Limpia el valor anterior de área
        lcd.setCursor(6, 0);
        lcd.print("----");
        
        // Necesario para caso en que no se ingresaron dígitos
        modoLote = false;
        
    }
    // Borra "/" cuando hubo menos de MAX_DIG_LOTE dígitos
    else if (nDigitos != MAX_DIG_LOTE) {
        lcd.setCursor(6 + nDigitos, 2);
        lcd.print(" ");
    }

    menu = 0;    
}

void lectura_datos() {
    // Lee dato enviado
    byte b1 = Serialvirt.read();
    byte b2 = Serialvirt.read();

    unsigned int arr = (b1 * 256) + b2;

    if(arr != 0) {
        Serialvirt.println(arr);
        Serialvirt.print("MSB (b1): ");
        Serialvirt.print(b1);
        Serialvirt.print(", LSB (b2): ");
        Serialvirt.print(b2);
        Serialvirt.println();
    }

    // Limipia area en LCD
    lcd.setCursor(6, 0);
    lcd.print("     ");
    // Imprime dato en LCD
    lcd.setCursor(6, 0);
    lcd.print(b1);

    // Limipia area en LCD
    lcd.setCursor(13, 1);
    lcd.print("     ");
    // Imprime dato en LCD
    lcd.setCursor(13, 1);
    lcd.print(b2);

    // Limipia area en LCD
    lcd.setCursor(6, 2);
    lcd.print("     ");
    // Imprime dato en LCD
    lcd.setCursor(6, 2);
    lcd.print(arr);
    
    // No se ha completado un cuero
    /* if(area != 0) {
        // Se actualiza area previa
        if(cueroListo) {
            // Limpia valor anterior areaPrevia LCD
            lcd.setCursor(13, 1);
            lcd.print("     ");
            lcd.setCursor(13, 1);
            lcd.print(areaFinal);

            areaFinal = 0;

            cueroListo = false;
        }

        // Se guarda el area del cuero actual y se imprime
        areaFinal = area;        

        // Limipia area en LCD
        lcd.setCursor(6, 0);
        lcd.print("     ");
        // Imprime dato en LCD
        lcd.setCursor(6, 0);
        lcd.print(areaFinal);        
    } */
    // Final cuero
    /* else if(!cueroListo) {

        // Espera de una nueva lectura de area != 0
        cueroListo = true;

        // Se acumulan las areas
        if(modoLote) {
            // Se imprime valor area total
            areaLote = areaLote + (long)areaFinal;
            lcd.setCursor(12, 3);
            lcd.print("      ");
            lcd.setCursor(12, 3);
            lcd.print(areaLote);
            // Se imprime valor del lote
            lote--;
            lcd.setCursor(6, 2);
            lcd.print(lote);
        }

        // Mirar si se acabó el lote ó no hay lote
        if(lote == 0 && modoLote) {
            // Terminar lote
            leerDatos = false;
            menu = 0;
            cueroListo = false;
            blinkingLed = true;
            // Se imprime el valor final del lote
            lcd.setCursor(12, 3);
            lcd.print(areaLote);
        }
        else if(!modoLote) {
            leerDatos = false;
            menu = 0;
            cueroListo = true;
            blinkingLed = false;
        }
    } */

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
