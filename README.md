# Tema3_Quick_Time
# Descrierea Task-urilor
Proiectul "Quick Time" reprezintă un joc competitiv destinat testării reflexelor, fiind conceput pentru doi jucători care concurează pentru a obține cel mai mare punctaj. Folosind două plăci Arduino Uno care comunică între ele prin protocolul SPI, sistemul gestionează componente precum LED-uri, butoane, un LCD și un servomotor pentru a crea o experiență interactivă și captivantă.

## Fluxul jocului:
Jocul începe cu afișarea unui mesaj de bun venit pe ecranul LCD. Pornirea jocului poate fi realizată în mai multe moduri, fie prin apăsarea oricărui buton, fie prin desemnarea unui buton specific sau adăugarea unui buton dedicat exclusiv pentru start. Noi am ales sa folosim un buton separat.

Pe parcursul jocului, fiecare jucător controlează un set de trei butoane, fiecare asociat unui LED de o anumită culoare. În timpul rundei, LED-ul RGB al jucătorului activ se aprinde într-o culoare care corespunde unui dintre butoane. Jucătorul trebuie să apese rapid butonul corect pentru a obține puncte, punctajul fiind calculat în funcție de viteza reacției.

La finalul fiecărei runde, LCD-ul afișează scorul actualizat al ambilor jucători, menținând în permanență un rezumat vizual al progresului fiecăruia.

## Finalizarea jocului:
Durata întregului joc este indicată de rotația completă a unui servomotor, care se deplasează treptat pe parcursul sesiunii. La sfârșit, ecranul LCD afișează numele câștigătorului și scorul final timp de câteva secunde, înainte de a reveni la mesajul de start.

## Detalii tehnice ale implementării
## Protocolul SPI:
Pentru a gestiona numărul mare de componente, două plăci Arduino comunică între ele. Arduino-ul master controlează LCD-ul, servomotorul și logica generală a jocului, în timp ce Arduino-ul slave gestionează LED-urile și butoanele, trimițând informații către master despre starea acestora.

## Butoane:
Jocul folosește câte trei butoane pentru fiecare jucător, asociate fiecărui LED de o anumită culoare. Modul de pornire al jocului poate fi flexibil, incluzând variante precum utilizarea oricărui buton, unui buton specific sau unui buton suplimentar dedicat. Noi am folosit un buton dedicat.

## LED-uri:

Fiecare buton este conectat la un LED de o culoare diferită, asigurând o asociere vizuală clară.
LED-urile RGB indică culoarea butonului care trebuie apăsat în timpul rundei active. Acestea rămân stinse atunci când nu este runda jucătorului corespunzător.
## LCD:
Controlat prin biblioteca LiquidCrystal, afișează scorurile actualizate ale jucătorilor în fiecare moment.
Luminozitatea și contrastul sunt reglate pentru o vizibilitate optimă, iar liniile de date sunt conectate la pini D4-D7.
## Servomotor:
Servomotorul începe la 0° și se rotește în sens antiorar pentru a indica progresul jocului. Mișcarea este configurată astfel încât o rotație completă să marcheze finalul sesiunii. Acesta revine de la 180 grade, înapoi la 0 grade, la sfârșitul jocului.
## Bonus-uri
Am adaugat posibilitatea jucatorilor de a-si introduce numele la inceputul unui joc. Dupa ce unul dintre ei apasa butonul de start, acestia vor fii rugati sa isi introduca numele pe interfata Serial. Am adaugat si un Buzzer, care la inceputul si la sfarsitul unui joc canta un sir de note muzicale si care semnaleaza raspunsurile corecte si gresite prin sunete specifice.
# Componente utilizate
- 6x LED-uri (2 grupuri de câte 3 leduri, în cadrul unui grup trebuie să avem culori diferite)
- 2x LED RGB (1 pentru fiecare jucător)
- 7x butoane (3 pentru fiecare jucător si unul pentru start)
- 1x LCD
- 1x servomotor
- 2x Breadboard
- 1x Buzzer
- 22x rezistoare (14x 220 Ohm; 1x 330 Ohm; 3x 1 kOhm, 2x 5.1 kOhm, 2x 100 kOhm)
- 1x Potentiometru (50k)
- Fire de legatura
- 2x Arduino Uno
