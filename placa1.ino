#include <Key.h>
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
static const unsigned char L_D_ASCII = 68;  // Valor en ascii de "C"

// Prototipo de funciones
int powint(int, int);
void getKey();

/* Variables globales */
// Teclado
bool modoLote = false;                      // Activar cuando se va a ingresar con lote
unsigned char nDigitos = 0;                 // Contador de digitos ingresados
unsigned char loteArray[MAX_DIG_LOTE];      // Guarda los dígitos ingresados
unsigned int lote = 0;                      // Guarda el valor de lote
unsigned char tecla;                        // Tecla ingresada

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
    lcd.init(); // initialize the lcd 
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("AREA: ----");
    lcd.setCursor(0, 1);
    lcd.print("AREA PREVIA: ----");
    lcd.setCursor(0, 2);
    lcd.print("LOTE: ----");
    lcd.setCursor(0, 3);
    lcd.print("AREA TOTAL: ----");
    Serial.begin(9600); // Velocidad de comunicacion con la PC
    pinMode(8, INPUT);
    Serialvirt.begin(9600); // Velocidad de comunicacion del otroarduino
}

void loop() {    
    // Obtiene tecla (si es pulsada)
    tecla = teclado.getKey();

    // Modo lote activado, se pide el lote. Tecla A
    if (tecla == L_A_ASCII) {
        getLote();
    }
    // Modo lote desactivado. Se empieza a leer área
    else if (tecla == L_B_ASCII) {
        lcd.setCursor(6, 2);
        lcd.print("desactivado");
    }
    // POR DEFINIR - borrar???
    else if (tecla == L_C_ASCII) {
        lcd.setCursor(6, 0);
        lcd.print("0            ");
    }
/*

    //if (Serialvirt.available()) { // Verificacion que el puerto serial virtual recibe datos
    if (true) { // Pruebas Camilo; comentar cuando se conecte el otro arduino
        //delay(1);                   
        lectura_datos(); // ejecute la funcion llamada "lectura_datos"
    } //dejo de leer que habia en el puerto */

}

/* void lectura_datos() { // funcion lectura del otro arduino
    Serial.println("Leyendo datos...");
    delay(5000);
    //lecturadelotroarduino = Serialvirt.read(); // Lectura de lo que el otro arduino quiere que yo lea   
    lecturadelotroarduino = 20;

    if (lecturadelotroarduino != 0) {
        area = lecturadelotroarduino;
        b = 0;
        lcd.setCursor(6, 0);
        lcd.print("             ");
        delay(1);
        lcd.setCursor(6, 0);
        lcd.print(lecturadelotroarduino);
    }

    if ((lecturadelotroarduino == 0) && (b == 0)) {

    }
} */

void getLote() {
    modoLote = true;                // Activa el modo lote
    nDigitos = 0;
    lote = 0;

    // Limpia lote anterior del LCD ó "desactivado"
    lcd.setCursor(6, 2);
    lcd.print("/          ");
    // Limpia el valor anterior de área
    lcd.setCursor(6, 0);
    lcd.print("----");

    // Espera a que se ingrese el valor del lote
    while (tecla != L_B_ASCII) {
        tecla = teclado.getKey();
        // Se entra si se pulsa un número 0-9
        if ((tecla >= N_0_ASCII) && (tecla <= N_9_ASCII) && (nDigitos < MAX_DIG_LOTE)) {    // Max value 9.999
            loteArray[nDigitos] = (tecla - N_0_ASCII);      // ASCII -> num, luego se guarda dígito en array. 
            lcd.setCursor(6 + nDigitos, 2);
            lcd.print(loteArray[nDigitos]);
            if (nDigitos != MAX_DIG_LOTE - 1) lcd.print("/");   // Se imprime "-" si no es el último dígito permitido
            Serial.println(loteArray[nDigitos]);
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
            break;
        }
    }

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

    Serial.print("El valor ingresado fue: ");
    Serial.print(lote);
    Serial.println();

    if (modoLote) {
        lcd.setCursor(6, 0);
        lcd.print("0            ");
    }
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
