#include <Arduino.h>  // biblioteca arduino

// definim pini leduri si butoane
const int butonstart = 3;  // buton start
const int butonstop = 2;   // buton stop
const int rgbr = 6;    // rgb rosu
const int rgbv = 5;    // rgb verde
const int led1 = 10;       // led1
const int led2 = 9;        // led2
const int led3 = 8;        // led3
const int led4 = 7;        // led4

bool incarca = false;  // verificam proces de incarcare
int nivelIncarcare = 1;  // nivel de incarcare 
unsigned long timpActualizare = 0;  // timpu de la ultima actualizare
unsigned long timpApasare = 0;  // timpu ultimei actualizare buton
const unsigned long debounceDelay = 50;  // pentru buton
const unsigned long timpApasareStop = 1000;  // cat apasam butonul de stop

int ultimaStart = LOW;  // ultima stare start
int stareStart = LOW;  //stare curenta start
int stareStop = LOW;  // stare curenta stop

bool clipoceste = false;  // clipoceste becul
unsigned long timpClipire = 0;  // schimbarea ultimei stari

void clipocesteLedCurent() {
    int ledCurent;
    if (nivelIncarcare == 1) ledCurent = led1;
    if (nivelIncarcare == 2) ledCurent = led2;
    if (nivelIncarcare == 3) ledCurent = led3;
    if (nivelIncarcare == 4) ledCurent = led4;

    if (millis() - timpClipire > 500) {  //la fiecare 500ms clipoceste
        clipoceste = !clipoceste;  //schimbam starea
        digitalWrite(ledCurent, clipoceste ? LOW : HIGH);  // on/of
        timpClipire = millis();  // refacem timpul
    }
}

void reseteazaLeds() {
    digitalWrite(led1, LOW);  // off led1
    digitalWrite(led2, LOW);  // off led2
    digitalWrite(led3, LOW);  // off led3
    digitalWrite(led4, LOW);  // off led4
}

void clipocesteToateLeds() {
    for (int i = 0; i <= 3; i++) {  
        digitalWrite(led1, HIGH);
        digitalWrite(led2, HIGH);
        digitalWrite(led3, HIGH);
        digitalWrite(led4, HIGH);
        delay(500);  
        reseteazaLeds();  // stingem toate ledurile
        delay(500);  
    }
}

void pornesteIncarcarea() {
    incarca = true;  // activeaza starea 
    nivelIncarcare = 1;  // porneste de la primul nivel (doar primul bec merge)
    timpActualizare = millis();  // seteaza timpul
    digitalWrite(rgbv, LOW);  // opreste rgb verde
    digitalWrite(rgbr, HIGH);  // porneste rgb rosu
}

void opresteIncarcarea() {
    incarca = false;  // seteaza starea = false
    digitalWrite(rgbv, HIGH);  // porneste rgb verde
    digitalWrite(rgbr, LOW);  // opreste rgb rosu
    reseteazaLeds(); // reseteaza ledurile
}

void intrerupeIncarcarea() {
    incarca = false;  // seteaza starea = false
    clipocesteToateLeds();  // toate becurile clipocesc
    opresteIncarcarea();  // opreste incarcarea
}

void actualizeazaIncarcarea() {
    if (nivelIncarcare <= 4) {  // daca nu s-a incarcat complet
        if (millis() - timpActualizare > 3000) {  // la fiecare 3s
            nivelIncarcare++;  // crestem nivelul (se mai aprinde 1 bec)
            timpActualizare = millis();  // actualizam timpul
        }
        clipocesteLedCurent();  // clipoceste ledul pentru nivelul curent
    } else {
        clipocesteToateLeds();  // clipocesc toate ledurile
        opresteIncarcarea();  // off
    }
}


void setup() {
  
    pinMode(butonstart, INPUT);  
    pinMode(butonstop, INPUT);   
    pinMode(rgbr, OUTPUT);   
    pinMode(rgbv, OUTPUT);   
    pinMode(led1, OUTPUT);       
    pinMode(led2, OUTPUT);       
    pinMode(led3, OUTPUT);     
    pinMode(led4, OUTPUT);      

    digitalWrite(rgbv, HIGH); // verde pornit
   
}

void loop() {
    int citireButonStart = !digitalRead(butonstart);
    if (citireButonStart != ultimaStart) { // ultima stare a butonului de start
        timpApasare = millis();  // actualizam timpul
    }
    if ((millis() - timpApasare) > debounceDelay) {  // pentru verificare debounce 
        if (citireButonStart != stareStart) {
            stareStart = citireButonStart; // actualizam stare buton start
                        if (stareStart == HIGH && !incarca) {
                pornesteIncarcarea();  // daca butonul este apasat incepem incarcarea daca nu se face
            }
        }
    }
    ultimaStart = citireButonStart;  // actualizam buton start
   
    stareStop = digitalRead(butonstop); // citim buton stop
    if (stareStop == LOW) {  // este apasat
        unsigned long timpulApasarii = millis();  // salvam timpul
        while (digitalRead(butonstop) == LOW) {  // daca este apasat
            if (millis() - timpulApasarii > timpApasareStop) { 
                intrerupeIncarcarea();  // nu mai incarcam
                break;  
            }
        }
    }
    if (incarca) {
        actualizeazaIncarcarea();  // procesul de incarcare
    }
}
