#include <Arduino.h>
#include <SPI.h>

//---------------------flag-uri pe care master le trimite catre slave-------------------------

#define START_BTN_PRESSED 1 //Slave trimite acest flag catre Master pe int. SPI atunci cand butonul desemnat de incepere a jocului a fost apasat
#define ROUND_OVER 2 //Master trimite 2 pe SPI, catre Slave pentru a semnala sfarsitul unei runde

#define GAME_STOP 99 //flag trimis de Master pentru a semnala sfarsitul unui joc
#define GAME_START 100 //flag trimis de Master pentru a semnala inceputul unui joc

#define PLAYER_ONE 101 //flag trimis de Master catre Slave, runda curenta ii corespunde primului jucator
#define PLAYER_TWO 102  //flag trimis de Master catre Slave, runda curenta ii corespunde celui de al doilea jucator

/*flag-uri trimise de Master, care specifica ce culoare a LED-ului RGB, trebuie aprinsa si LED-ul carui jucator trebuie aprins*/
#define PLAYER_ONE_RED_COLOR 103 //culoare rosie de pe LED-ul RGB al primului jucator
#define PLAYER_ONE_GREEN_COLOR 104 //culoarea verde de pe LED-ul RGB al primului jucator
#define PLAYER_ONE_BLUE_COLOR 105 //culoarea albastra de pe LED-ul RGB al primmului jucator

#define PLAYER_TWO_RED_COLOR 106 //culoarea rosie de pe LED-ul RGB, al doilea jucator
#define PLAYER_TWO_GREEN_COLOR 107 //culoarea verde de pe LED-ul RGB, al doilea jucator
#define PLAYER_TWO_BLUE_COLOR 108 //culoarea albastra de pe LED-ul RGB, al doilea jucator

//------------------------------declararea PINI----------------------------

#define PLAYER_ONE_BTNS A0 //cele 3 butoane ale player 1, multiplexate pe pinul analogic 0
#define PLAYER_TWO_BTNS A1 //cele 3 butoane ale player 2, multiplexate pe pinul analogic 1
#define START_BTN A2 //butonul de start

/*am conectat toate LED-urile corespunzatoare unui jucator in serie, pentru a folosii 1 PIN pentru 3 LED-uri*/
#define PLAYER_ONE_LEDS 2 //LED-urile primului jucator, conectate la PIN-ul digital 2
#define PLAYER_TWO_LEDS 3 //LED-urile jucatorului 2, conectate la PIN-ul digital 3


/*pinii LED-ului RGB al primului jucator*/
#define PLAYER_ONE_RED_RGB 9
#define PLAYER_ONE_GREEN_RGB 8
#define PLAYER_ONE_BLUE_RGB 7

/*pinii LED-ului RGB al celui de al doilea jucator*/
#define PLAYER_TWO_RED_RGB 6
#define PLAYER_TWO_GREEN_RGB 4
#define PLAYER_TWO_BLUE_RGB 5

//valorile obtinute la apasarea butoanelor multiplexate pe pinul analogic
//le folosim pentru a pute face diferenta intre butoanele apasate
#define RED_BTN_VALUE 1023 //daca a fost apasat butonul rosu, analogRead trebuie sa citeasca aceasta valoare
#define GREEN_BTN_VALUE 1012 //daca a fost apasat butonul verde, analogRead trebuie sa citeasca aceasta valoare
#define BLUE_BTN_VALUE 975 //daca a fost apasat butonul albastru, analogRead trebuie sa citeasca aceasta valoare

#define ANALOG_THRESHOLD 5

//valorile pe care slave le trimite catre master la apasarea butoanelor
//expected_values la apasarea butoanelor multiplexate
#define RED_BTN_PRESSED 109 //trimis catre Master cand a fost apasat butonul rosu
#define GREEN_BTN_PRESSED 110 //trimis catre Mastar cand a fost apasat butonul vrede
#define BLUE_BTN_PRESSED 111 //trimis catre Master cand a fost apasat butonul albastru

bool gameStarted = false; //variabila care mentine starea jucoului, in curs de desfasurare sau terminat

volatile uint8_t dataReceived; //variabila de tip byte, care preia byte-ul trimis de master pe interfata SPI

uint8_t playerTurn = 0; //variabila care retine jucatorul, care joaca runda curenta
int buttonValue; //variabila care mentine valoare citita dupa apasarea butoului

bool valueInRange(int readValue, int thresholdValue) { //preia 2 valori: valoarea butonului citita de pe pinul analogic si una dintre expected values obtinute la multiplexarea butoanelor 
    return abs(readValue - thresholdValue) <= ANALOG_THRESHOLD;
    //vrifica daca valoarea citita la apasarea butonului se afla intr-un intreval [-5,5], centrat in unul dintre expected_values
}

bool readButtons(uint8_t currentPlayer){
  /*
  *functie care citeste constant starea butoanelor, corespunzatoare jucatorului care jocaca runda curenta
  */
  bool spdrUpdated = false; /*e false daca valoarea citita de pe pin-ul analogic e ce default, i.e. daca nu a fost apasat niciun buton 
  si true daca a fost apasat unul dintre butoane*/
  
  if (currentPlayer == PLAYER_ONE) { //daca este runda jucatorului 1, vor fii verifivcate doar butoanele jucatorului 1
    buttonValue = analogRead(PLAYER_ONE_BTNS); //e citita valoarea de pe pinul A0
    /*verifica care buton a fost apasat*/
    if (valueInRange(buttonValue, RED_BTN_VALUE)) { //daca a fost apasat butonul rosu
      SPDR = RED_BTN_PRESSED; // e pus in SPDR flag-ul corespunzator pentru a ii semnala lui Master ca butonul rosu a fost apasat
      spdrUpdated = true; // a fost apasat un buton
    } else if (valueInRange(buttonValue, GREEN_BTN_VALUE)) { //daca butonul verde a fost apasat
      SPDR = GREEN_BTN_PRESSED; // e pus in SPDR flag-ul corespunzator pentru a ii semnala lui Master ca butonul verde a fost apasat
      spdrUpdated = true; // a fost apasat un buton
    } else if (valueInRange(buttonValue, BLUE_BTN_VALUE)) { // daca a fost apasat butonul albastru
      SPDR = BLUE_BTN_PRESSED; // e pus in SPDR flag-ul corespunzator, pentru a ii semnala lui Master ca butonul albastru a afost apasat
      spdrUpdated = true; // a fost apasat un buton
    }
  } else if(currentPlayer == PLAYER_TWO){ //daca este runda jucatorului 2, vor fii verificate doar butoanele jucatorului 2
    buttonValue = analogRead(PLAYER_TWO_BTNS); //e citita valoarea de pe pinul A1
    /*verificam ce buton a fost apasat*/
    if (valueInRange(buttonValue, RED_BTN_VALUE)) {
      SPDR = RED_BTN_PRESSED;
      spdrUpdated = true;
    } else if (valueInRange(buttonValue, GREEN_BTN_VALUE)) {
      SPDR = GREEN_BTN_PRESSED;
      spdrUpdated = true;
    } else if(valueInRange(buttonValue, BLUE_BTN_VALUE)) {
      SPDR = BLUE_BTN_PRESSED;
      spdrUpdated = true;
    }
  }
  return spdrUpdated; //returnam aceasta valoare pentru a stii daca a fost apasat vreun buton
}

void toggleRGBLed(uint8_t currentPlayer, bool state) {
  /*
  *functie care face toggle LED-ului RGB al unui jucator
  *folosita la sfarsitul jucului pentru a stinge ambele LED-uri RGB
  *si la sfarsitul unei runde, tot pentru a stinge ambele LED-uri
  */
  if (currentPlayer == PLAYER_ONE) {
    digitalWrite(PLAYER_ONE_RED_RGB, state);
    digitalWrite(PLAYER_ONE_GREEN_RGB, state);
    digitalWrite(PLAYER_ONE_BLUE_RGB, state);
  } else if (currentPlayer == PLAYER_TWO) {
    digitalWrite(PLAYER_TWO_RED_RGB, state);
    digitalWrite(PLAYER_TWO_GREEN_RGB, state);
    digitalWrite(PLAYER_TWO_BLUE_RGB, state);
  }
}

void setRGBColor(uint8_t currentPlayer, uint8_t rgbColor) {
  /*
  *functie folosita in timpul jocului pentru a aprinde o culoare a LED-ului RGB a jucatorului care joaca runda curenta
  *coloarea care trebuie aprinsa este primita printr-un flag trimis pe SPI de Master
  */
  if (currentPlayer == PLAYER_ONE) { //daca este runda jucatorului 1
    //este aprinsa coloarea primita ca parametru
    if (rgbColor == PLAYER_ONE_RED_COLOR) {
      digitalWrite(PLAYER_ONE_RED_RGB, HIGH);
      digitalWrite(PLAYER_ONE_GREEN_RGB, LOW);
      digitalWrite(PLAYER_ONE_BLUE_RGB, LOW);
    } else if (rgbColor == PLAYER_ONE_GREEN_COLOR) {
      digitalWrite(PLAYER_ONE_RED_RGB, LOW);
      digitalWrite(PLAYER_ONE_GREEN_RGB, HIGH);
      digitalWrite(PLAYER_ONE_BLUE_RGB, LOW);
    } else if (rgbColor == PLAYER_ONE_BLUE_COLOR) {
      digitalWrite(PLAYER_ONE_RED_RGB, LOW);
      digitalWrite(PLAYER_ONE_GREEN_RGB, LOW);
      digitalWrite(PLAYER_ONE_BLUE_RGB, HIGH);
    }
  } else if (currentPlayer == PLAYER_TWO) { //daca este runda jucatorului 2
    //este aprinsa culoarea primita ca paramentru
    if (rgbColor == PLAYER_TWO_RED_COLOR) {
      digitalWrite(PLAYER_TWO_RED_RGB, HIGH);
      digitalWrite(PLAYER_TWO_GREEN_RGB, LOW);
      digitalWrite(PLAYER_TWO_BLUE_RGB, LOW);
    } else if (rgbColor == PLAYER_TWO_GREEN_COLOR) {
      digitalWrite(PLAYER_TWO_RED_RGB, LOW);
      digitalWrite(PLAYER_TWO_GREEN_RGB, HIGH);
      digitalWrite(PLAYER_TWO_BLUE_RGB, LOW);
    } else if (rgbColor == PLAYER_TWO_BLUE_COLOR) {
      digitalWrite(PLAYER_TWO_RED_RGB, LOW);
      digitalWrite(PLAYER_TWO_GREEN_RGB, LOW);
      digitalWrite(PLAYER_TWO_BLUE_RGB, HIGH);
    }
  }
}

void togglePlayersLeds(uint8_t currentPlayer, bool state) {
  /*
  *folosita in gameStop() pentru a stinge LED-urile celor 2 jucatori la sfarsitul jocului 
  */
  if (currentPlayer == PLAYER_ONE) {
    digitalWrite(PLAYER_ONE_LEDS, state);
  } else if (currentPlayer == PLAYER_TWO) {
    digitalWrite(PLAYER_TWO_LEDS, state);
  }
}

void startGame(){
  /*
  * functie apelata la apasarea butonului de start
  */
  gameStarted = true; //seteaza variabila true, pentru a semnifica ca jocul a inceput
  digitalWrite(PLAYER_ONE_LEDS, HIGH); //aprinde LED-urile celor 2 jucatori
  digitalWrite(PLAYER_TWO_LEDS, HIGH);
}

void stopGame() {
  /*
  *functie apelata la sfarsitul unui joc
  *stinge LED-urile si LED-urile RGB ale ambilor playeri
  */
  toggleRGBLed(PLAYER_ONE, LOW);
  toggleRGBLed(PLAYER_TWO, LOW);
  togglePlayersLeds(PLAYER_ONE, LOW);
  togglePlayersLeds(PLAYER_TWO, LOW);
  gameStarted = false; //seteaza variabila la false
  Serial.println("Jocul s-a oprit!");
}

void handleDataReceived(uint8_t dataReceived) {
  /*
  *functie care gestioneaza flag-urile primite de la Master pe interfata SPI
  */
  if (dataReceived != 0xFF) //daca nu a primit valoarea dummy afiseaza pe Serial ce a primit
    Serial.println("dataReceived = " + String(dataReceived));
  switch (dataReceived) { // in functie de ce a primit pe SPI
    case GAME_START: //daca a primit flag-ul GAME_START apeleaza functia startGame()
      startGame();
      break;

    case GAME_STOP: //daca a primit flag-ul GAME_STOP apeleaza functia stopGame()
      stopGame();
      break;
    
    //primeste de la Master player-ul curent pentru a stii, sa implementeze logice de citire a butoanelor si de aprindere a LED-urilor
    case PLAYER_ONE:
      playerTurn = PLAYER_ONE;
      Serial.println("randul playerului 1");
      break;

    case PLAYER_TWO:
      playerTurn = PLAYER_TWO;
      Serial.println("randul playerului 2");
      break;

    case ROUND_OVER: //daca prieste flag-ul ROUND-over stinge LED-urile RGB
      toggleRGBLed(PLAYER_TWO, LOW);
      toggleRGBLed(PLAYER_ONE, LOW);
      break;

    /*primeste un flag corespunzator unei culori a unui LED RGB si aprinde acel LED*/
    case PLAYER_ONE_RED_COLOR: //led_ul rosu trebuie sa se aprinda
      setRGBColor(PLAYER_ONE, PLAYER_ONE_RED_COLOR);
      break;

    case PLAYER_ONE_GREEN_COLOR:
      setRGBColor(PLAYER_ONE, PLAYER_ONE_GREEN_COLOR);
      break;

    case PLAYER_ONE_BLUE_COLOR:
      setRGBColor(PLAYER_ONE, PLAYER_ONE_BLUE_COLOR);
      break;

    case PLAYER_TWO_RED_COLOR: //led_ul rosu trebuie sa se aprinda
      setRGBColor(PLAYER_TWO, PLAYER_TWO_RED_COLOR);
      break;

    case PLAYER_TWO_GREEN_COLOR:
      setRGBColor(PLAYER_TWO, PLAYER_TWO_GREEN_COLOR);
      break;

    case PLAYER_TWO_BLUE_COLOR:
      setRGBColor(PLAYER_TWO, PLAYER_TWO_BLUE_COLOR);
      break;
  }
}

ISR(SPI_STC_vect) { //ISR pentru primirea de date pe interfata SPI
  uint8_t output = 0;
  bool spdrUpdated = false;
  dataReceived = SPDR; //variabila in care salvam ce primim in registrul SPDR
  handleDataReceived(dataReceived); //se apeleaza functia de gestionare a flag-urilor de fiecare data cand am  primit o ceva pe ISR
  if (gameStarted) { //daca jocul a inceput 
    spdrUpdated = readButtons(playerTurn);// se citeste valoarea generata de apasarea butoanelor
  }
  if (!spdrUpdated) //daca nu a fost apasat niciun buton 
    SPDR = output; //Slave pune in SPDR 0
}

void setup() {
  Serial.begin(9600);   // debugging
  
  // turn on SPI in slave mode
  SPCR |= (1 << SPE);

  // have to send on master in, slave out
  pinMode(MISO, OUTPUT);

  // now turn on interrupts
  SPI.attachInterrupt();

  pinMode(PLAYER_ONE_LEDS, OUTPUT);
  pinMode(PLAYER_TWO_LEDS, OUTPUT);
  
  togglePlayersLeds(PLAYER_ONE, LOW);
  togglePlayersLeds(PLAYER_TWO, LOW);

  pinMode(PLAYER_ONE_BLUE_RGB, OUTPUT);
  pinMode(PLAYER_ONE_GREEN_RGB, OUTPUT);
  pinMode(PLAYER_ONE_RED_RGB, OUTPUT);

  pinMode(PLAYER_TWO_BLUE_RGB, OUTPUT);
  pinMode(PLAYER_TWO_GREEN_RGB, OUTPUT);
  pinMode(PLAYER_TWO_RED_RGB, OUTPUT);

  toggleRGBLed(PLAYER_ONE, LOW);
  toggleRGBLed(PLAYER_TWO, LOW);
}

void loop() {
  if (digitalRead(START_BTN) == 0 && digitalRead(SS) == LOW) {//daca a fost apasat butonul de start si Slave-ul e selectat
    SPDR = START_BTN_PRESSED; //Slave pune in SPDR un flag pentru a ii semnala lui Master acest lucru
  }
} 