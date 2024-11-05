#include <Arduino.h> 


const int startButton = 3; // buton start
const int difficultyButton = 2; //buton dificultate
const int ledRedPin = 10; 
const int ledGreenPin = 9; 
const int ledBluePin = 8; 

const int wordVector = 50; 
const char* words[wordVector] = {
    "adventure", "breeze", "canyon", "dazzling", "echo", "feather", "glimmer", "harvest",
    "illusion", "journey", "kettle", "lantern", "mystic", "nebula", "oasis", "puzzle",
    "quiver", "radiant", "serene", "tangle", "umbrella", "vortex", "whisper", "zephyr",
    "blossom", "crystal", "ember", "flutter", "galaxy", "harmony", "ignite", "jade",
    "labyrinth", "miracle", "nirvana", "opal", "prism", "quaint", "relic", "symphony",
    "twilight", "unison", "velvet", "wander", "xenon", "yearn", "zenith", "brisk",
    "clover", "delight"
};

enum gameState { IDLE, STARTING, RUNNING };
enum difficultyLevel { UNKNOWN = -1, EASY, MEDIUM, HARD };

gameState currentState = IDLE; 
difficultyLevel currentDifficulty = UNKNOWN; 

const unsigned long debounceDelay = 250; 
const unsigned int gameDuration = 30000;
unsigned long lastButtonPressTime = 0;
unsigned long lastDifficultyPressTime = 0;
unsigned long gameTime = 0; 
unsigned long wordTime = 0; 

unsigned int timeWord = 0; 
int correctWords = 0; 
String currentWord; 
String inputBuffer;

bool wordUsed[wordVector] = { false }; 
int usedWordsCount = 0;

void setLEDColor(int red, int green, int blue) { 
    analogWrite(ledRedPin, red);
    analogWrite(ledGreenPin, green);
    analogWrite(ledBluePin, blue);
}

void changeStartStopGame() { 
    if (millis() - lastButtonPressTime > debounceDelay) { 
        lastButtonPressTime = millis();

        if (currentDifficulty == UNKNOWN) { //verificam daca dificultatea e selectata
            Serial.println("Selecteaza dificultatea jocului!");
            return;
        }

        if (currentState == IDLE) { // verificam daca e jocul pornit
            currentState = STARTING;
            Serial.println("Jocul incepe");
        } else { 
            currentState = IDLE;
            correctWords = 0;
            usedWordsCount = 0; 
            memset(wordUsed, false, sizeof(wordUsed)); // resetam vec cuv folosite
            setLEDColor(255, 255, 255); // lumineaza alb - jocul e oprit
            Serial.println("Jocul s-a oprit.");
        }
    }
}

void generateWord() { 
    if (usedWordsCount >= wordVector) { // verificam daca au fost folosite toate cuvintele
        Serial.println("Toate cuvintele s-au folosit!");
        usedWordsCount = 0; 
        memset(wordUsed, false, sizeof(wordUsed)); // resetam vec cuv folosite
    }

    int index; 
    do {
        index = random(wordVector); // index random pentru cuvant
    } while (wordUsed[index]); // pana nu e folosit cuvantul

    wordUsed[index] = true;// -> used
    usedWordsCount++;

    currentWord = words[index]; // mutam cuvantul in bufferu de cuvinte pe care trebuie sa il scrie utilizatorul
    Serial.print("Scrie cuvantul:");
    Serial.println(currentWord); 
    inputBuffer = ""; 
    wordTime = millis();
}

void Countdown() {
    for (int i = 3; i > 0; i--) {
        Serial.println(i);
        setLEDColor(255, 255, 255); 
        delay(500); 
        setLEDColor(0, 0, 0);
        delay(500);
    }
    currentState = RUNNING; // start game
    gameTime = millis();
    Serial.println("Poti incepe sa scri cuvantul!");
    setLEDColor(0, 255, 0); // led = verde
    generateWord();
}

void Input() {
    while (Serial.available()) {
        char received = Serial.read(); 

        if (received == '\b' && inputBuffer.length() > 0) { // pentru a sterge caractere din cuvantul introdus de utilizator
            inputBuffer.remove(inputBuffer.length() - 1); 
        } else if (received == '\n') {
            inputBuffer.trim(); 

            if (inputBuffer.equals(currentWord)) { // verificam daca e cuvantul introdus este corect
                Serial.println("Corect!");
                correctWords++; 
                setLEDColor(0, 255, 0);
                generateWord(); 
            } else {
                Serial.println("Gresit! Incearca din nou");
                setLEDColor(255, 0, 0);
            }

            inputBuffer = "";
            wordTime = millis(); 
        } else {
            inputBuffer += received;// introducem in bufferul pentru cuvantul utilizatorului caracterul introdus de utilizator
        }
    }

    if (millis() - wordTime > timeWord) {  // timer pentru cuvant
        Serial.println("Timpul a trecut! Urmatorul cuvant:");
        generateWord();
        inputBuffer = ""; 
    }
}


void Difficulty() { 
    if (millis() - lastDifficultyPressTime > debounceDelay) { 
        lastDifficultyPressTime = millis();
        
        if (currentState == RUNNING) { // verificam daca jocul a inceput  -> nu putem schimba dificultatea
            Serial.println("Nu poți schimba dificultatea în timpul jocului!");
            return; 
        }

        currentDifficulty = (difficultyLevel)((currentDifficulty + 1) % 3); // urmatoare dificultate

        const char* difficultyText = currentDifficulty == EASY ? "Easy" :
                                     currentDifficulty == MEDIUM ? "Medium" : "Hard";
        Serial.print(difficultyText);
        Serial.println(" Dificultate aleasa!");

        timeWord = currentDifficulty == EASY ? 10000 :
                   currentDifficulty == MEDIUM ? 5000 : 3000; // timp in functie de dificultate

        Serial.print("Ai ");
        Serial.print(timeWord / 1000);
        Serial.println(" secunde pentru fiecare cuvant.");
    }
}

void setup() { 
    Serial.begin(28800);
    pinMode(ledRedPin, OUTPUT); 
    pinMode(ledGreenPin, OUTPUT); 
    pinMode(ledBluePin, OUTPUT); 

    pinMode(startButton, INPUT); 
    pinMode(difficultyButton, INPUT); 
    
    randomSeed(analogRead(A0));

   //intreruperi start si dificultate
    attachInterrupt(digitalPinToInterrupt(startButton), changeStartStopGame, FALLING);
    attachInterrupt(digitalPinToInterrupt(difficultyButton), Difficulty, FALLING);

    Serial.println("Selecteaza dificultatea pe care o doresti."); 
}

void loop() { 
    if (currentState == STARTING) { // verificam daca jocul a inceput
        Countdown(); 
    } else if (currentState == RUNNING) { 
        if (millis() - gameTime < gameDuration) {
            Input(); 
        } else { 
            Serial.print("Sfarsit joc! Ai terminat cu scorul: ");// s-a terminat timpul de joc
            Serial.println(correctWords); // scor final
            currentState = IDLE; 
            setLEDColor(255, 255, 255); 
        }
    }
}
