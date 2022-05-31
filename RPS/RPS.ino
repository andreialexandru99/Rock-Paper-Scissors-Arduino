#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Sounds.h"

#define UNKNOWN_PICK 0
#define ROCK 1
#define PAPER 2
#define SCISSORS 3

#define TIE 0
#define PLAYER1_WON 1
#define PLAYER2_WON 2

#define RESET_PIN PC0
#define NEW_ROUND_PIN PC1

LiquidCrystal_I2C lcd(0x27,16,2);  // set LCD address to 0x27, 16X2

bool p1Ready = false;
bool p2Ready = false;

int p1Score = 0;
int p2Score = 0;

int p1Pick = UNKNOWN_PICK;
int p2Pick = UNKNOWN_PICK;

bool scoreUpdated = false;
long debounce = 0;

void configPlayer1Pins() {
  // Config input pullup pins PD0-2
  DDRD &= ~(7 << PD2);
  PORTD |= (7 << PD2);
  
  // Enable pin change interrupt
  PCICR |= (1 << PCIE2);
  // Turn on PCINT18-20
  PCMSK2 |= (7 << PCINT18);
}

void configPlayer2Pins() {
  // Config input pullup pins PB0-2
  DDRB &= ~(7 << PB0);
  PORTB |= (7 << PB0);
  
  // Enable pin change interrupt
  PCICR |= (1 << PCIE0);
  // Turn on PCINT0-2
  PCMSK0 |= (7 << PCINT0);
}

void configGameControlPins() {
  // Config input pullup pins PC0-1
  DDRC &= ~(3 << PC0);
  PORTC |= (3 << PC0);
  
  // Enable pin change interrupt
  PCICR |= (1 << PCIE1);
  // Turn on PCINT8-9
  PCMSK1 |= (3 << PCINT8);
}

void configBuzzer() {
  // Config output pin PC2
  DDRC |= (1 << PC2);
}

void configLCD() {
  // Initialize LCD and turn on backlight
  lcd.begin();
  lcd.backlight();
}

ISR(PCINT0_vect){ // Player2 input
  // Pick changes aren't allowed. Also acts as debounce mechanism.
  if (p2Ready) return;
  p2Ready = true;

  // Check which pin triggered the interrruption and update player 2's pick
  if ((PINB & (1 << PB0)) == 0) {
    p2Pick = ROCK;
    playButtonSound();
  } else if ((PINB & (1 << PB1)) == 0) {
    p2Pick = PAPER;
    playButtonSound();
  } else if ((PINB & (1 << PB2)) == 0) {
    p2Pick = SCISSORS;
    playButtonSound();
  }
}

ISR(PCINT1_vect){ // Reset / New Round input
  if (millis() - debounce < 170) return;
  debounce = millis();
    
  if ((PINC & (1 << PC0)) == 0) {
    // Reset score
    p1Ready = false;
    p2Ready = false;
    
    p1Pick = UNKNOWN_PICK;
    p2Pick = UNKNOWN_PICK;
    
    scoreUpdated = false;
    p1Score = 0;
    p2Score = 0;
    playButtonSound();
  } else if ((PINC & (1 << PC1)) == 0) {
    // Start new round
    p1Ready = false;
    p2Ready = false;
    
    p1Pick = UNKNOWN_PICK;
    p2Pick = UNKNOWN_PICK;
    
    scoreUpdated = false;
    playButtonSound();
  }
}

ISR(PCINT2_vect){ // Player1 input
  // Pick changes aren't allowed. Also acts as debounce mechanism.
  if (p1Ready) return;
  p1Ready = true;
  
  // Check which pin triggered the interrruption and update player 1's pick
  if ((PIND & (1 << PD2)) == 0) {
    p1Pick = ROCK;
    playButtonSound();
  } else if ((PIND & (1 << PD3)) == 0) {
    p1Pick = PAPER;
    playButtonSound();
  } else if ((PIND & (1 << PD4)) == 0) {
    p1Pick = SCISSORS;
    playButtonSound();
  }
}

String buildScoreLine() {
  String line = "P1 ";
  // Pad between "P1" and P1 score
  int i = 100;
  while (p1Score < i && i > 1) {
    line += " ";
    i /= 10;
  }
  line += String(p1Score) +  " -- " + String(p2Score);
  
  // Pad between P2 score and "P2"
  i = 100;
  while (p2Score < i && i > 1) {
    line += " ";
    i /= 10;
  }
  return line + " P2";
}

String buildStatusLine() {
  String line;
  if (p1Ready) {
    line = "READY      ";
  } else {
    line = "PICK       ";
  }
  if (p2Ready) {
    line += "READY";
  } else {
    line += " PICK";
  }
  return line;
}

String buildResultLine(int result) {
  if (result == TIE) {
    return "TIE          TIE";
  } else if (result == PLAYER1_WON) {
    return "WINNER     LOSER";
  } else if (result == PLAYER2_WON) {
    return "LOSER     WINNER";
  } 
}

void displayStatus() {
  // Place cursor at the start of the first line and print score
  lcd.setCursor(0, 0);
  lcd.print(buildScoreLine());
  
  // Place cursor at the start of the second line and print status
  lcd.setCursor(0, 1);
  lcd.print(buildStatusLine());
}

void displayResults(int result) {
  // Place cursor at the start of the first line and print score
  lcd.setCursor(0, 0);
  lcd.print(buildScoreLine());
  
  // Place cursor at the start of the second line and print result
  lcd.setCursor(0, 1);
  lcd.print(buildResultLine(result));
}

void updateScore(int result) {
  // Return if score has already been upgraded for this round
  if (scoreUpdated) return;
  scoreUpdated = true;

  // Only update if one player won. Ties are irrelevant
  if (result == PLAYER1_WON) {
    p1Score++;
  } else if (result == PLAYER2_WON) {
    p2Score++;
  }
}

int computeResults() {
  // Check if it's a tie
  if (p1Pick == p2Pick) {
    return TIE;
  }
  // Not a tie. Check who won
  if (p1Pick == ROCK) {
    // Player1 chose rock. Check Player2's choice
    if (p2Pick == PAPER) {
      // Player2 chose paper and won.
      return PLAYER2_WON;
    } else if (p2Pick == SCISSORS) {
      // Player2 chose scissors and lost.
      return PLAYER1_WON;
    }
  } else if (p1Pick == PAPER) {
    // Player1 chose paper. Check Player2's choice
    if (p2Pick == ROCK) {
      // Player2 chose rock and lost.
      return PLAYER1_WON;
    } else if (p2Pick == SCISSORS) {
      // Player2 chose scissors and won.
      return PLAYER2_WON;
    }
  } else if (p1Pick == SCISSORS) {
    // Player1 chose scissors. Check Player2's choice
    if (p2Pick == PAPER) {
      // Player2 chose paper and lost.
      return PLAYER1_WON;
    } else if (p2Pick == ROCK) {
      // Player2 chose rock and won.
      return PLAYER2_WON;
    }
  }
  // Error occured if this point is reached
  return -1;
}

void setup()
{
  // Call pin configuration functions
  configPlayer1Pins();
  configPlayer2Pins();
  configGameControlPins();
  configBuzzer();
  configLCD();
  // Activate interruptions
  sei();
}

void loop()
{
  int result;
  if (p1Ready && p2Ready) {
    // If both players are ready, compute and display round result
    result = computeResults();
    updateScore(result);
    displayResults(result);
  } else {
    // At least one player still has to pick. Display status
    displayStatus();
  }
}
