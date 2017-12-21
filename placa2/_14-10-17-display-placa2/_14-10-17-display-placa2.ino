static const unsigned char STX_ASCII = 123; // Valor en ascii de "{"
static const unsigned char ETX_ASCII = 125; // Valor en ascii de "}"

byte conteodeapagadosporpulso; //se almacenara cuantos focos apagados hay por cada barrida ( entiendase barrida por leer los 64 focos)
float lectura;
float area;
int POT = A0; // Pin de entrada de senal del potenciometro
int POT_valor = 0; // Variable de almacenamiento del porcentaje del potenciometro 
int porcentaje; // Variable de conversion a porcentaje

bool tramaStarted = false;      // Valor para guardar si se ha iniciado trama o no

void setup() {
    pinMode(A5, INPUT); //interrptor en este caso seria el sensor
    pinMode(2, INPUT); //lectura 
    pinMode(3, INPUT); //lectura
    pinMode(4, INPUT); //lectura
    pinMode(5, INPUT); //lectura
    pinMode(6, INPUT); //lectura
    pinMode(7, INPUT); //lectura
    pinMode(8, INPUT); //lectura  
    pinMode(9, INPUT); //lectura  

    pinMode(13, OUTPUT); //pulsos
    pinMode(12, OUTPUT); //pulsos
    pinMode(11, OUTPUT); //pulsos
    pinMode(10, OUTPUT); //pulsos
    pinMode(A0, OUTPUT); //pulsos  
    pinMode(A1, OUTPUT); //pulsos
    pinMode(A2, OUTPUT); //pulsos  
    pinMode(A3, OUTPUT); //pulsos

    Serial.begin(9600); //velocidad de comunicaciona serial

    conteodeapagadosporpulso = 0; //asegurar que cada vez que arranca el programa este en cero
    area = 0; //asegurar que cada vez que arranca el programa este en cero
    delay(2000);
}

void loop() {
    lectura = digitalRead(A5); //va  a leer el sensor del piñon
    int myPins[] = {13, 12, 11, 10, A0, A1, A2, A3}; //Defino en una lista los pines que funcionaran como pulsos

    if (lectura == 0) { // Si el sensor del piñon leyo un diente, entonces entre hacer barridos y vaya sumando areas
        conteodeapagadosporpulso = 0; //Cada que haga una nueva barrida, asegurarse que el conteo de focos apagados se reinicie
        for (int i = 0; i < 8; i++) { //for que me hace el barrido, prendiendo los pines como pulsos definididos en la lista myPins y leyendo las entradas
            digitalWrite(myPins[i], HIGH); //prende la fuente
            delay(6); //tiempo necesario para darle respiro a la ejecucion del codigo y no hayan incoherencias
            if (digitalRead(2) == 0) {
                conteodeapagadosporpulso = conteodeapagadosporpulso + 1;
            }
            if (digitalRead(3) == 0) {
                conteodeapagadosporpulso = conteodeapagadosporpulso + 1;
            }
            if (digitalRead(4) == 0) {
                conteodeapagadosporpulso = conteodeapagadosporpulso + 1;
            }
            if (digitalRead(5) == 0) {
                conteodeapagadosporpulso = conteodeapagadosporpulso + 1;
            }
            if (digitalRead(6) == 0) {
                conteodeapagadosporpulso = conteodeapagadosporpulso + 1;
            }
            if (digitalRead(7) == 0) {
                conteodeapagadosporpulso = conteodeapagadosporpulso + 1;
            }
            if (digitalRead(8) == 0) {
                conteodeapagadosporpulso = conteodeapagadosporpulso + 1;
            }
            if (digitalRead(9) == 0) {
                conteodeapagadosporpulso = conteodeapagadosporpulso + 1;
            }
            digitalWrite(myPins[i], LOW); //apaga la fuente y se reinicia el proceso pero con otra fuente
        } //hasta aqui ya se hizo la barrida de los 64 focos, se almacena por cada barrida el numero de focos apagados (conteodeapagadosporpulso) y se reinicia cuando empiece otra barrida

        if(conteodeapagadosporpulso != 0) {
            Serial.print("Dato a enviar: ");
            Serial.print(conteodeapagadosporpulso);
            Serial.println();
        }

        // No se ha iniciado trama y se lee cero
        if (conteodeapagadosporpulso == 0 && !tramaStarted) {
            // No se hace nada
        }
        // Dato != 0 y no se ha iniciado trama -> Se inicia una trama
        else if (conteodeapagadosporpulso != 0 && !tramaStarted) {
            tramaStarted = true;
            Serial.write(STX_ASCII);
            Serial.write(conteodeapagadosporpulso);
        }
        // Dato != 0 y ya se ha iniciado trama -> mande dato
        else if (conteodeapagadosporpulso != 0 && tramaStarted) {
            Serial.write(conteodeapagadosporpulso);
        }
        // Dato = 0 y se habia iniciado trama -> Finalizar trama
        else if (conteodeapagadosporpulso == 0 && tramaStarted) {
            Serial.write(ETX_ASCII);
        }

        //area=area+((conteodeapagadosporpulso*2.54*2.54)/100.0);  //como el conteodeapagadosporpulso se reinicia cuando empieza otra barrida, "area" irá almacenando ese conteo pero ya convertido en decimas cuadradas 
        //delay(1);
        /* Serial.write(int(area));
        if (conteodeapagadosporpulso==0){ //si al hacer la barrida conto que todos los focos estaban prendidos, entonces reinicie el proceso de sumas de areas pues se considera que ya conto el area de una pieza, esto se hace obviamente dentro de la lectura del sensor en diente
          area=0;
        } */
    } //hasta aqui llega las ordenes que se dan cuando el sensor registra un diente del piñon.

    while (lectura == 0) { // este while es muy importante, porque si el sensor habia acabado de registrar un diente, entonces impide que vuelva a ejecutar las ordenes de ese suceso para un mismo diente
        lectura = digitalRead(A5); //Es decir, se acabo de leer un diente, hubo una barrida y como el sensor seguira sensando el mismo diente entonces entrara a este while y hasta que no registre el hueco no podra salir del ciclo y volver a empezar. 
    }
}