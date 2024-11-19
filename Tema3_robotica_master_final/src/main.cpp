#include <Arduino.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Servo.h>

/* valori transmise catre Slave cu privire la starea jocului */
#define ROUND_OVER 2 // terminarea rundei
#define GAME_STOP 99 // terminarea jocului
#define GAME_START 100 // inceputul jocului
#define PLAYER_ONE 101 // randul primului jucator
#define PLAYER_TWO 102 // randul jucatorului al doilea
#define PLAYER_ONE_RED_COLOR 103 // LED-ul rosu al primului jucator trebuie aprins
#define PLAYER_ONE_GREEN_COLOR 104 // LED-ul verde al primului jucator trebuie aprins
#define PLAYER_ONE_BLUE_COLOR 105 // LED-ul albastru al primului jucator trebuie aprins
#define PLAYER_TWO_RED_COLOR 106 // LED-ul rosu al celui de-al doilea jucator trebuie aprins
#define PLAYER_TWO_GREEN_COLOR 107 // LED-ul verde al celui de-al doilea jucator trebuie aprins
#define PLAYER_TWO_BLUE_COLOR 108 // LED-ul albastru al celui de-al doilea jucator trebuie aprins

/* valori primite de la Slave la apasarea diferitelor butoane */
#define START_BTN_PRESSED 1 // butonul de start a fost apasat
#define RED_BTN_PRESSED 109 // butonul corespunzator LED-ului rosu a fost apasat
#define GREEN_BTN_PRESSED 110 // butonul corespunzator LED-ului verde a fost apasat
#define BLUE_BTN_PRESSED 111 // butonul corespunzator LED-ului albastru a fost apasat

/* pinul aferent buzzer-ului utilizat in redarea sunetelor la inceputul/sfarsitului jocului, respectiv la apasarea unui buton corect/gresit */
#define BUZZER 2

/* pinul aferent servomotorului utilizat in indicarea progresului jocului */
#define SERVO 3

/* pinii aferenti LCD-ului utilizat in afisarea mesajului de bun venit, respectiv a punctajelor jucatorilor */
#define RS 4
#define EN 5
#define D4 6
#define D5 7
#define D6 8
#define D7 9

/* caracterul backspace */
#define BACKSPACE '\b'

/* caracterul carriage return (indica momentul cand cursorul revine la inceputul unei linii) */
#define CARRIAGE_RETURN '\r' 

/* o valoare dummy care va fi trimisa Slave-ului, necesara pentru a putea primi date de la acesta */
#define DUMMY_VALUE 0xFF

/* notele muzicale utilizate la redarea sunetelor la inceputul/sfarsitului jocului, respectiv la apasarea unui buton corect/gresit */
#define WRONG_ANSWER_NOTE 100
#define A4_NOTE 440
#define B4_NOTE 494
#define C4_NOTE 523
#define D4_NOTE 587
#define E4_NOTE 659
#define CORRECT_ANSWER_NOTE 1000

/* durata unei note muzicale */
#define NOTE_DURATION 500

/* intervalurile de reactie ale jucatorilor, utilizate pentru a calcula scorul acordat */
#define FIRST_RESPONSE_INTERVAL 500
#define SECOND_RESPONSE_INTERVAL 1000
#define THIRD_RESPONSE_INTERVAL 1500

/* intervalul de timp alocat unei rotiri ale servomotorului */
#define SERVO_ROTATION_INTERVAL 15

/* initializeaza ecranul LCD */
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

/* retine daca jocul a inceput sau nu */
bool gameStarted = false;

/* retine daca trebuie citit butonul de start */
bool readStartButton = true;

/* retine daca butonul apasat este cel corect */
bool correctButton = false;

/* retine daca trebuie sa se aleaga un nou jucator */
bool changePlayersTurn = true;

/* retine daca numele jucatorilor au fost introduse de la tastatura */
bool playersNamesEntered = false;

/* datele primite de la Slave */
uint8_t dataReceived = 0;

/* scorul primului jucator */
uint8_t playerOneScore = 0;

/* scorul celui de-al doilea jucator */
uint8_t playerTwoScore = 0;

/* pozitia absoluta a servomotorului */
uint8_t servoPosition = 0;

/* retine al carui jucator este randul */
uint8_t playerTurn = PLAYER_TWO;

/* culoarea LED-ului RGB al primului jucator */
uint8_t playerOneRgbColor = PLAYER_ONE_RED_COLOR;

/* culoarea LED-ului RGB al celui de-al doilea jucator */
uint8_t playerTwoRgbColor = PLAYER_TWO_RED_COLOR;

/* momentul de timp in care a inceput o runda */
unsigned long roundStartedTime = 0;

/* durata unei runde */
unsigned long roundInterval = 10000;

/* momentul de timp in care a inceput jocul */
unsigned long gameStartedTime = 0;

/* durata unui joc */
unsigned long gameInterval = 44200;

/* momentul de timp in care a avut loc ultima schimbare a culorii LED-ului RGB */
unsigned long lastColorChangeTime = 0;

/* intervalul de timp la care trebuie sa se schimbe culoarea LED-ului RGB */
unsigned long colorChangeInterval = 2000;

/* perioada de rotire a servomotorului */
unsigned long servoRotationPeriod = 36500 / 180;

/* momentul de timp in care a avut loc ultima rotire a servomotorului */
unsigned long lastRotationTime = 0;

/* numele primului jucator */
String playerOneName = "";

/* numele celui de-al doilea jucator */
String playerTwoName = "";

/* initializeaza un obiect de tip servomotor */
Servo gameProgressServo;

/*
 * Functie ce reda notele muzicale in ordinea crescatoare a frecventelor
 * 
 */
void playIncreasingNotes() {
    /* opreste redarea unui sunet anterior */
    noTone(BUZZER);

    /* reda fiecare nota muzicala pentru NOTE_DURATION milisecunde */
    tone(BUZZER, A4_NOTE);
    delay(NOTE_DURATION);
    tone(BUZZER, B4_NOTE);
    delay(NOTE_DURATION);
    tone(BUZZER, C4_NOTE);
    delay(NOTE_DURATION);
    tone(BUZZER, D4_NOTE);
    delay(NOTE_DURATION);
    tone(BUZZER, E4_NOTE);
    delay(NOTE_DURATION);
}

/*
 * Functie ce reda notele muzicale in ordinea descrescatoare a frecventelor
 * 
 */
void playDecreasingNotes() {
    /* reda fiecare nota muzicala pentru NOTE_DURATION milisecunde */
    tone(BUZZER, E4_NOTE);
    delay(NOTE_DURATION);
    tone(BUZZER, D4_NOTE);
    delay(NOTE_DURATION);
    tone(BUZZER, C4_NOTE);
    delay(NOTE_DURATION);
    tone(BUZZER, B4_NOTE);
    delay(NOTE_DURATION);
    tone(BUZZER, A4_NOTE);
    delay(NOTE_DURATION);

    /* opreste redarea sunetelor */
    noTone(BUZZER);
}

/*
 * Functie ce reda o nota muzicala corespunzatoare apasarii corecte a unui buton
 * 
 */
void playCorrectAnswerNote() {
    /* opreste redarea unui sunet anterior */
    noTone(BUZZER);

    /* reda nota muzicala pentru NOTE_DURATION milisecunde */
    tone(BUZZER, CORRECT_ANSWER_NOTE, NOTE_DURATION);
}

/*
 * Functie ce reda o nota muzicala corespunzatoare apasarii gresite a unui buton
 * 
 */
void playWrongAnswerNote() {
    /* opreste redarea unui sunet anterior */
    noTone(BUZZER);

    /* reda nota muzicala pentru NOTE_DURATION milisecunde */
    tone(BUZZER, WRONG_ANSWER_NOTE, NOTE_DURATION);
}

/*
 * Functie ce afiseaza mesajul de bun venit pe ecranul LCD
 * 
 */
void printWelcomeMessage() {
    /* goleste ecranul LCD */
    lcd.clear();

    /* afiseaza mesajul de bun venit pe doua linii */
    lcd.setCursor(0, 0);
    lcd.print("Bun venit!");
    lcd.setCursor(0, 1);
    lcd.print("Apasa pe start!");
}

/*
 * Functie ce goleste buffer-ul asociat comunicarii seriale
 * 
 */
void clearSerialBuffer() {
    /* citeste caractere cat timp buffer-ul nu e gol */
    while (Serial.available())
        Serial.read();
}

/*
 * Functie ce se ocupa de citirea numelor jucatorilor
 * 
 */
void enterPlayersNames() {
    /* numele tastat */
    static String typedName = "";

    /* retine pentru ce jucator trebuie tastat numele */
    static bool enterPlayerOneName = true;
    static bool enterPlayerTwoName = false;

    /* retine daca pe ecranul LCD trebuie afisat jucatorul caruia ii apartine numele tastat */
    static bool promptMessage = true;

    if (promptMessage) {
        /* goleste ecranul LCD */
        lcd.clear();

        /* afiseaza pe prima linie a ecranului pentru ce jucator se face tastarea numelui */
        lcd.setCursor(0, 0);
        if (enterPlayerOneName) {
            lcd.print("Jucator 1:");
        } else if (enterPlayerTwoName) {
            lcd.print("Jucator 2:");
        }

        /* muta cursorul pe urmatoarea linie */
        lcd.setCursor(0, 1);

        /* reseteaza necesitatea afisarii pe ecranul LCD a detinatorului numelui */
        promptMessage = false;
    }

    /* verifica pentru ce jucator se tasteaza numele */
    if (enterPlayerOneName) {
        /* verifica daca sunt date in buffer-ul alocat comunicarii pe portul serial care asteapta sa fie citite */
        if (Serial.available()) {
            /* citeste urmatorul caracter din buffer */
            char readChar = Serial.read();

            /* verifica daca a fost apasat Enter sau caracterul citit este Backspace */
            if (readChar == CARRIAGE_RETURN) {
                /* trebuie sa se introduca numele pentru urmatorul jucator si sa se afiseze acest lucru */
                enterPlayerOneName = false;
                enterPlayerTwoName = true;
                promptMessage = true;

                /* scurteaza numele primului jucator la 7 caractere daca este mai lung de atat */
                if (typedName.length() > 6)
                    playerOneName = typedName.substring(0, 7);
                else
                    playerOneName = typedName;

                /* reseteaza numele tastat */
                typedName = "";

                /* goleste buffer-ul alocat comunicarii seriale */
                clearSerialBuffer();
            } else if (readChar == BACKSPACE) {
                /* sterge ultimul caracter tastat si actualizeaza ecranul LCD */
                typedName.remove(typedName.length() - 1);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Jucator 1:");
                lcd.setCursor(0, 1);
                lcd.print(typedName);
            } else {
                /* concateneaza caracterul tastat la numele jucatorului */
                typedName += readChar;

                /* afiseaza pe ecranul LCD caracterul tastat */
                lcd.print(readChar);
            }
        }
    } else if (enterPlayerTwoName) {
        /* verifica daca sunt date in buffer-ul alocat comunicarii pe portul serial care asteapta sa fie citite */
        if (Serial.available()) {
            /* citeste urmatorul caracter din buffer */
            char readChar = Serial.read();

            /* verifica daca a fost apasat Enter sau caracterul citit este Backspace */
            if (readChar == CARRIAGE_RETURN) {
                /* nu mai trebuie sa se introduca numele pentru niciun jucator */
                enterPlayerTwoName = false;

                /* scurteaza numele celui de-al doilea jucator la 7 caractere daca este mai lung de atat */
                if (typedName.length() > 6)
                    playerTwoName = typedName.substring(0, 7);
                else
                    playerTwoName = typedName;

                /* reseteaza numele tastat */
                typedName = "";
            } else if (readChar == BACKSPACE) {
                /* sterge ultimul caracter tastat si actualizeaza ecranul LCD */
                typedName.remove(typedName.length() - 1);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Jucator 2:");
                lcd.setCursor(0, 1);
                lcd.print(typedName);
            } else {
                /* concateneaza caracterul tastat la numele jucatorului */
                typedName += readChar;

                /* afiseaza pe ecranul LCD caracterul tastat */
                lcd.print(readChar);
            }
        }
    } else {
        /* marcheaza sfarsitul tastarii numelor jucatorilor si reseteaza variabilele folosite */
        playersNamesEntered = true;
        enterPlayerOneName = true;
        promptMessage = true;
    }
}

/*
 * Functie ce aduce la pozitia initiala servomotorul care indica progresul jocului
 * 
 */
void resetServo() {
    /* verifica daca pozitia este pozitiva */
    while (servoPosition > 0) {        
        /* muta servomotorul la o pozitie mai mica */
        gameProgressServo.write(servoPosition--);
        
        /* asteapta un interval de timp inainte de urmatoarea rotire */
        delay(SERVO_ROTATION_INTERVAL);
    }
}

/*
 * Functie ce afiseaza pe ecranul LCD detalii despre primul jucator
 * 
 */
void printFirstPlayer() {
    /* goleste ecranul LCD si afiseaza pe prima linie jucatorul curent, iar pe a doua numele sau */
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Jucator 1: ");
    lcd.setCursor(0, 1);
    lcd.print(playerOneName);
}

/*
 * Functie ce afiseaza pe ecranul LCD detalii despre al doilea jucator
 * 
 */
void printSecondPlayer() {
    /* goleste ecranul LCD si afiseaza pe prima linie jucatorul curent, iar pe a doua numele sau */
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Jucator 2:");
    lcd.setCursor(0, 1);
    lcd.print(playerTwoName);
}

/*
 * Functie ce afiseaza pe ecranul LCD castigatorul si scorul acestuia
 * 
 */
void printWinner() {
    /* goleste ecranul LCD */
    lcd.clear();

    /* seteaza cursorul pe prima linie */
    lcd.setCursor(0, 0);

    /* verifica castigatorul */
    if (playerOneScore > playerTwoScore) {
        /* afiseaza pe prima linie numele castigatorului, in cazul acesta primul jucator */
        lcd.print("Castiga " + playerOneName);
        
        /* afiseaza pe a doua linie scorul castigatorului */
        lcd.setCursor(0, 1);
        lcd.print("Scor: " + String(playerOneScore));
    } else if (playerOneScore < playerTwoScore) {
        /* afiseaza pe prima linie numele castigatorului, in cazul acesta al doilea jucator */
        lcd.print("Castiga " + playerTwoName);

        /* afiseaza pe a doua linie scorul castigatorului */
        lcd.setCursor(0, 1);
        lcd.print("Scor: " + String(playerTwoScore));
    } else {
        /* afiseaza pe prima linie faptul ca a fost egalitate */
        lcd.print("Egalitate");

        /* afiseaza pe a doua linie scorul jucatorilor */
        lcd.setCursor(0, 1);
        lcd.print("Scor: " + String(playerTwoScore));
    }
}

/*
 * Functie ce actualizeaza scorul jucatorului curent
 * 
 * @param currentPlayer - jucatorul curent
 * @param responseTime - timpul de reactie in milisecunde
 */
void updateScore(uint8_t currentPlayer, unsigned long responseTime) {
    /* punctele care urmeaza sa fie adaugate scorului */
    uint8_t points = 0;
    
    /* verifica in ce interval se incadreaza timpul de raspuns al jucatorului */
    if (responseTime <= FIRST_RESPONSE_INTERVAL) {
        /* cea mai rapida reactie, adauga 4 puncte */
        points = 4;
    } else if (responseTime > FIRST_RESPONSE_INTERVAL && responseTime <= SECOND_RESPONSE_INTERVAL) {
        /* o reactie mai putin rapida, adauga 3 puncte */
        points = 3;
    } else if (responseTime > SECOND_RESPONSE_INTERVAL && responseTime <= THIRD_RESPONSE_INTERVAL) {
        /* o reactie medie, adauga 2 puncte */
        points = 2;
    } else {
        /* o reactie lenta, adauga 1 punct */
        points = 1;
    }

    /* goleste ecranul LCD */
    lcd.clear();

    /* actualizeaza scorul jucatorului curent si afiseaza pe prima linie a ecranului */
    lcd.setCursor(0, 0);
    if (currentPlayer == PLAYER_ONE) {
        playerOneScore += points;
        lcd.print("Scor " + playerOneName + ": " + String(playerOneScore));
    } else if (currentPlayer == PLAYER_TWO) {
        playerTwoScore += points;
        lcd.print("Scor " + playerTwoName + ": " + String(playerTwoScore));
    }
}

/*
 * Functie ce verifica ce buton a fost apasat
 * 
 * @param currentPlayer - jucatorul curent
 */
void checkPressedButton(uint8_t currentPlayer) {
    /* primeste de la Slave informatii referitoare la butonul apasat */
    uint8_t pressedButton = SPI.transfer(DUMMY_VALUE);

    /* verifica daca a fost apasat butonul corect */
    if (pressedButton == RED_BTN_PRESSED && 
        ((currentPlayer == PLAYER_ONE && playerOneRgbColor == PLAYER_ONE_RED_COLOR) || 
        (currentPlayer == PLAYER_TWO && playerTwoRgbColor == PLAYER_TWO_RED_COLOR))) {
        /* actualizeaza scorul jucatorului curent */
        updateScore(currentPlayer, millis() - lastColorChangeTime);

        /* retine ca a fost apasat butonul corect */
        correctButton = true;

        /* reda o nota muzicala */
        playCorrectAnswerNote();
    } else if (pressedButton == GREEN_BTN_PRESSED && 
        ((currentPlayer == PLAYER_ONE && playerOneRgbColor == PLAYER_ONE_GREEN_COLOR) || 
        (currentPlayer == PLAYER_TWO && playerTwoRgbColor == PLAYER_TWO_GREEN_COLOR))) {
        /* actualizeaza scorul jucatorului curent */
        updateScore(currentPlayer, millis() - lastColorChangeTime);

        /* retine ca a fost apasat butonul corect */
        correctButton = true;

        /* reda o nota muzicala */
        playCorrectAnswerNote();
    } else if (pressedButton == BLUE_BTN_PRESSED && 
        ((currentPlayer == PLAYER_ONE && playerOneRgbColor == PLAYER_ONE_BLUE_COLOR) ||
        (currentPlayer == PLAYER_TWO && playerTwoRgbColor == PLAYER_TWO_BLUE_COLOR))) {
        /* actualizeaza scorul jucatorului curent */
        updateScore(currentPlayer, millis() - lastColorChangeTime);

        /* retine ca a fost apasat butonul corect */
        correctButton = true;

        /* reda o nota muzicala */
        playCorrectAnswerNote();
    } else if (pressedButton == RED_BTN_PRESSED || pressedButton == GREEN_BTN_PRESSED || pressedButton == BLUE_BTN_PRESSED) {
        /* reda o nota muzicala corespunzatoare apasarii gresite a unui buton */
        playWrongAnswerNote();
    }
}

/*
 * Functie ce schimba culoarea LED-ului RGB pentru jucatorul curent
 * 
 * @param currentPlayer - jucatorul curent
 */
void changeRgbColor(uint8_t currentPlayer) {
    /* verifica jucatorul curent */
    if (currentPlayer == PLAYER_ONE) {
        /* trece la urmatoarea culoare a LED-ului RGB pentru primul jucator */
        if (playerOneRgbColor == PLAYER_ONE_RED_COLOR) {
            playerOneRgbColor = PLAYER_ONE_GREEN_COLOR;
        } else if (playerOneRgbColor == PLAYER_ONE_GREEN_COLOR) {
            playerOneRgbColor = PLAYER_ONE_BLUE_COLOR;
        } else {
            playerOneRgbColor = PLAYER_ONE_RED_COLOR;
        }

        /* aplica o intarziere */
        delay(50);

        /* transmite catre Slave noua culoare a LED-ului RGB */
        SPI.transfer(playerOneRgbColor);
    } else if (currentPlayer == PLAYER_TWO) {
        /* trece la urmatoarea culoare a LED-ului RGB pentru cel de-al doilea jucator */
        if (playerTwoRgbColor == PLAYER_TWO_RED_COLOR) {
            playerTwoRgbColor = PLAYER_TWO_GREEN_COLOR;
        } else if (playerTwoRgbColor == PLAYER_TWO_GREEN_COLOR) {
            playerTwoRgbColor = PLAYER_TWO_BLUE_COLOR;
        } else {
            playerTwoRgbColor = PLAYER_TWO_RED_COLOR;
        }

        /* aplica o intarziere */
        delay(50);

        /* transmite catre Slave noua culoare a LED-ului RGB */
        SPI.transfer(playerTwoRgbColor);
    }
}

/*
 * Functie ce trece la urmatorul jucator la finalul unei runde
 * 
 */
void switchPlayers() {
    /* goleste ecranul LCD */
    lcd.clear();

    /* afiseaza pe prima linie a ecranului scorul jucatorului curent */
    lcd.setCursor(0, 0);
    if (playerTurn == PLAYER_ONE) {
        lcd.print("Scor " + playerTwoName + ": " + String(playerTwoScore));
        playerTurn = PLAYER_TWO;
    } else if (playerTurn == PLAYER_TWO) {
        lcd.print("Scor " + playerOneName + ": " + String(playerOneScore));
        playerTurn = PLAYER_ONE;
    }

    /* reseteaza necesitatea schimbarii jucatorului curent */
    changePlayersTurn = false;

    /* transmite catre Slave noul jucator */
    SPI.transfer(playerTurn);
}

/*
 * Functie ce se ocupa de oprirea unei runde
 * 
 */
void stopRound() {
    /* opreste redarea unui sunet anterior */
    noTone(BUZZER);

    /* goleste ecranul LCD si afiseaza scorul actualizat al ambilor jucatori */
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scor " + playerOneName + ": " + String(playerOneScore));
    lcd.setCursor(0, 1);
    lcd.print("Scor " + playerTwoName + ": " + String(playerTwoScore));

    /* transmite catre Slave faptul ca s-a oprit runda */
    SPI.transfer(ROUND_OVER);
}

/*
 * Functie ce se ocupa de inceputul unei runde
 * 
 */
void startGame() {
    /* afiseaza pe ecranul LCD detalii despre primul jucator */
    printFirstPlayer();

    /* reda notele muzicale in ordinea crescatoare a frecventelor */
    playIncreasingNotes();

    /* afiseaza pe ecranul LCD detalii despre al doilea jucator */
    printSecondPlayer();

    /* reda notele muzicale in ordinea descrescatoare a frecventelor */
    playDecreasingNotes();

    /* transmite catre Slave faptul ca a inceput jocul */
    SPI.transfer(GAME_START);

    /* retine ca a inceput jocul */
    gameStarted = true;

    /* retine momentul de timp in care a inceput jocul */
    gameStartedTime = millis();

    /* reseteaza scorurile jucatorilor */
    playerOneScore = 0;
    playerTwoScore = 0;

    /* reseteaza momentul de timp in care a avut loc ultima rotire a servomotorului */
    lastRotationTime = 0;

    /* reseteaza ultima informatie primita de la Slave */
    dataReceived = 0;
}

/*
 * Functie ce se ocupa de gestionarea unui joc
 * 
 */
void handleGame() {
    /* verifica daca au trecut servoRotationPeriod milisecunde de la ultima rotire a servomotorului si daca se mai poate roti */
    if ((millis() - lastRotationTime) >= servoRotationPeriod && servoPosition <= 180) {
        /* roteste servomotorul cu un grad */
        gameProgressServo.write(servoPosition++);

        /* retine momentul de timp in care a avut loc ultima rotire a servomotorului */
        lastRotationTime = millis();
    }

    /* verifica daca trebuie sa se schimbe jucatorii */
    if (changePlayersTurn) {
        /* trece la urmatorul jucator */
        switchPlayers();

        /* aplica o intarziere */
        delay(1000);

        /* retine momentul de timp in care a inceput noua runda */
        roundStartedTime = millis();

        /* reseteaza momentul de timp in care a avut loc ultima schimbare a culorii LED-ului RGB */
        lastColorChangeTime = 0;
    }

    /* verifica daca au trecut colorChangeInterval milisecunde de la ultima schimbare a culorii LED-ului RGB */
    if ((millis() - lastColorChangeTime) >= colorChangeInterval) {
        /* schimba culoarea LED-ului RGB pentru jucatorul curent */
        changeRgbColor(playerTurn);

        /* opreste redarea unui sunet anterior */
        noTone(BUZZER);

        /* aplica o intarziere */
        delay(150);

        /* reseteaza faptul ca butonul apasat a fost cel corect */
        correctButton = false;

        /* retine momentul de timp in care a avut loc ultima schimbare a culorii LED-ului RGB */
        lastColorChangeTime = millis();
    }

    /* verifica daca nu s-a apasat butonul corect inca */
    if (!correctButton) {
        /* verifica ce buton a fost apasat de catre jucatorul curent */
        checkPressedButton(playerTurn);
    }

    /* verifica daca au trecut roundInterval milisecunde de la inceputul rundei */
    if ((millis() - roundStartedTime) >= roundInterval) {
        /* opreste runda */
        stopRound();

        /* marcheaza necesitatea schimbarii jucatorilor */
        changePlayersTurn = true;

        /* reseteaza faptul ca butonul apasat a fost cel corect */
        correctButton = false;

        /* aplica o intarziere */
        delay(2500);

        /* ignora intarzierea aplicata pentru a nu afecta calculul duratei unui joc */
        gameStartedTime += 2500;
    }
}

/*
 * Functie ce se ocupa de terminarea unui joc
 * 
 */
void stopGame() {
    /* reseteaza necesitatea schimbarii jucatorilor */
    changePlayersTurn = true;

    /* reseteaza faptul ca jocul a inceput */
    gameStarted = false;

    /* reseteaza faptul ca butonul apasat a fost cel corect */
    correctButton = false;

    /* retine faptul ca trebuie citit butonul de start */
    readStartButton = true;

    /* marcheaza faptul ca jucatorii trebuie sa-si introduca numele din nou */
    playersNamesEntered = false;

    /* reseteaza momentul de timp in care a avut loc ultima schimbare a culorii LED-ului RGB */
    lastColorChangeTime = 0;

    /* transmite catre Slave faptul ca jocul s-a oprit */
    SPI.transfer(GAME_STOP);

    /* goleste buffer-ul asociat comunicarii seriale */
    clearSerialBuffer();

    /* afiseaza pe ecranul LCD castigatorul si scorul acestuia */
    printWinner();

    /* reda notele muzicale in ordinea crescatoare a frecventelor */
    playIncreasingNotes();

    /* reda notele muzicale in ordinea descrescatoare a frecventelor */
    playDecreasingNotes();

    /* reseteaza servomotorul la pozitia initiala */
    resetServo();

    /* afiseaza mesajul de bun venit pe ecranul LCD */
    printWelcomeMessage();
}

void setup() {
    /* incepe comunicarea seriala */
    Serial.begin(9600);
    
    /* dezactiveaza Slave-ul */
    digitalWrite(SS, HIGH);

    /* initializeaza comunicarea prin SPI in modul Master */
    SPI.begin();

    /* seteaza frecventa ceasului la 1/4 din frecventa ceasului placutei */
    SPI.setClockDivider(SPI_CLOCK_DIV4);

    /* initializeaza ecranul LCD cu dimensiunea acestuia */
    lcd.begin(16, 2);

    /* afiseaza mesajul de bun venit pe ecranul LCD */
    printWelcomeMessage();
    
    /* ataseaza obiectului de tip servomotor pinul folosit */
    gameProgressServo.attach(SERVO);

    /* reseteaza servomotorul la pozitia initiala */
    gameProgressServo.write(0);

    /* initializeaza buzzer-ul ca output */
    pinMode(BUZZER, OUTPUT);
}

void loop() {
    /* activeaza Slave-ul */
    digitalWrite(SS, LOW);

    /* verifica daca trebuie citit butonul de start */
    if (readStartButton == true) {
        /* primeste de la Slave informatii */
        dataReceived = SPI.transfer(DUMMY_VALUE);
    }

    /* verifica atat daca Slave-ul a transmis faptul ca butonul de start a fost apasat, cat si ca nu a inceput jocul */
    if (dataReceived == START_BTN_PRESSED && gameStarted == false) {
        /* reseteaza faptul ca trebuie citit butonul de start */
        readStartButton = false;

        /* verifica daca au fost introduse numele jucatorilor de la tastatura */
        if (playersNamesEntered == false) {
            /* citeste numele jucatorilor */
            enterPlayersNames();
        } else {
            /* incepe jocul */
            startGame();
        }
    }

    /* verifica daca jocul a inceput */
    if (gameStarted == true) {
        /* verifica daca au trecut gameInterval milisecunde de la inceputul jocului */
        if ((millis() - gameStartedTime) <= gameInterval) {
            /* gestioneaza jocul */
            handleGame();
        } else {
            /* opreste jocul */
            stopGame();
        }
    }

    /* aplica o intarziere in microsecunde */
    delayMicroseconds(20);

    /* dezactiveaza Slave-ul */
    digitalWrite(SS, HIGH);

    /* aplica o intarziere */
    delay(50);
}