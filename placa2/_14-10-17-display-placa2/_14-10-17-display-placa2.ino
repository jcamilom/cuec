static const unsigned char STX_ASCII = 123; // Valor en ascii de "{", el inicio de trama elegido
static const unsigned char ETX_ASCII = 125; // Valor en ascii de "}", el fin de trama elegido

/**
 * Se almacenara cuantos focos apagados hay por cada barrida 
 * (entiendase barrida por leer los 64 focos)
 */
byte apagados;
byte lectura;
// ** puede ser borrada
float area;
// Defino en una lista los pines que funcionaran como pulsos
unsigned int myPins[] = {13, 12, 11, 10, A0, A1, A2, A3};
// Valor para guardar si se ha iniciado trama o no
bool tramaStarted;
// Valores para ignorar datos al encender la máquina
bool startingMachine;
byte count;

void setup() {
    // Interrptor en este caso seria el sensor
    pinMode(A5, INPUT);

    // Pines de lectura
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);

    // Pines de pulsos
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    pinMode(11, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(A0, OUTPUT); 
    pinMode(A1, OUTPUT);
    pinMode(A2, OUTPUT); 
    pinMode(A3, OUTPUT);

    Serial.begin(9600); // Velocidad de comunicaciona serial

    apagados = 0;       // Asegurar que cada vez que arranca el programa este en cero
    area = 0;           // Asegurar que cada vez que arranca el programa este en cero
    startingMachine = true;
    tramaStarted = false;
    count = 0;
    //delay(2000);
}

void loop() {
    lectura = digitalRead(A5); // Va a leer el sensor del piñon    

    // Si el sensor del piñon leyo un diente, entonces entre hacer barridos y vaya sumando areas
    if(lectura == LOW) {
        
        // Cada que haga una nueva barrida, asegurarse que el conteo de focos apagados se reinicie
        apagados = 0;
        
        /**
         * "for" que me hace el barrido, prendiendo los pines como pulsos definididos
         * en la lista myPins y leyendo las entradas
         */
        for(int i = 0; i < 8; i++) {
            
            digitalWrite(myPins[i], HIGH); // Prende la fuente
            // Tiempo necesario para darle respiro a la ejecucion del codigo y no hayan incoherencias
            delay(6);
            
            if(digitalRead(2) == 0) apagados++;
            if(digitalRead(3) == 0) apagados++;
            if(digitalRead(4) == 0) apagados++;
            if(digitalRead(5) == 0) apagados++;
            if(digitalRead(6) == 0) apagados++;
            if(digitalRead(7) == 0) apagados++;
            if(digitalRead(8) == 0) apagados++;
            if(digitalRead(9) == 0) apagados++;

            // Apaga la fuente y se reinicia el proceso pero con otra fuente
            digitalWrite(myPins[i], LOW);
        }
        // Hasta aqui ya se hizo la barrida de los 64 focos, se almacena por
        // cada barrida el numero de focos apagados (apagados) y se reinicia cuando empiece otra barrida

        /* if(apagados != 0) {        
            Serial.print("Dato a enviar: ");
            Serial.print(apagados);
            Serial.println();
        } */

        // Se entra cuando se prende la máquina y se empiezan a generar ceros
        if(startingMachine) {
            // Se ignoran los 20 primeros ceros (no consecutivos)
            if(apagados == 0) count++;
            if(count == 20) startingMachine = false;
        }
        // LÓGICA GENERAL DE LA MÁQUINA        
        // Dato != 0 y no se ha iniciado trama -> Se inicia una trama
        else if(apagados != 0 && !tramaStarted) {
            tramaStarted = true;
            Serial.write(STX_ASCII);
            Serial.write(apagados);
        }
        // Dato != 0 y ya se ha iniciado trama -> Mandar dato
        else if(apagados != 0 && tramaStarted) {
            Serial.write(apagados);
        }
        // Dato = 0 y se habia iniciado trama -> Finalizar trama
        else if(apagados == 0 && tramaStarted) {
            Serial.write(ETX_ASCII);
            tramaStarted = false;
        }

        /**
         * Como el apagados se reinicia cuando empieza otra barrida, "area"
         * irá almacenando ese conteo pero ya convertido en decimas cuadradas 
         */
        area = area + ((apagados * 2.54 * 2.54) / 100.0);
        //delay(1);

        int intArea = (int)area;
        /* if(intArea != 0) {
            Serial.print("Area a enviar: ");
            Serial.print(intArea);
            Serial.println();
        } */
        
        /**
         * Si al hacer la barrida conto que todos los focos estaban prendidos, entonces
         * reinicie el proceso de sumas de areas pues se considera que ya conto 
         * el area de una pieza, esto se hace obviamente dentro de la lectura del sensor en diente.
         */
        if(apagados == 0) {
            area = 0;
        }
    }
    // Hasta aqui llega las ordenes que se dan cuando el sensor registra un diente del piñon.

    /**
     * Este while es muy importante, porque si el sensor habia acabado de registrar un diente, 
     * entonces impide que vuelva a ejecutar las ordenes de ese suceso para un mismo diente.
     */
    while(lectura == LOW) {
        /**
         * Es decir, se acabo de leer un diente, hubo una barrida y como el 
         * sensor seguira sensando el mismo diente entonces entrara a este while y 
         * hasta que no registre el hueco no podra salir del ciclo y volver a empezar.
         */
        lectura = digitalRead(A5);
    }
}
