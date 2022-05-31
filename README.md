# Rock-Paper-Scissors-Arduino
This is a Rock Paper Scissors game on an Arduino.

## How to use
1. Clone the repository
2. Open project in Arduino IDE
3. Include the provided library in the project ( Sketch -> Include library -> Add .ZIP library... and then browse to and include the archive )
4. Connect your Arduino board (after assembling the circuit - instructions below) to your laptop and upload

## Hardware needed
- Arduino Uno
- 8 pushbuttons
- A passive buzzer
- A 1602 LCD with a I2C interface

For wiring consult the "Wiring Diagram.png" picture.

## How to play
- Each player has three buttons corresponding to each choice (Rock / Paper / Scissors)
  - Player 1 has buttons connected to pins 2-4 on the board (2 -> Rock, 3 -> Paper, 4 -> Scissors)
  - Player 2 has buttons connected to pins 8-10 on the board (8 -> Rock, 9 -> Paper, 10 -> Scissors)
- Game control is realized through the buttons connected to pins A0 and A1 on the board (A0 -> Reset Score, A1 -> New Round)
- At the start of each round, the display will show which players still have to pick
- After both players have made their pick, the display will show the winner and loser and update the score (permanently shown at the top of the LCD)
  - A player can't change their pick during the round
- Then, to start a new round, press the New Round button mentioned above
- To reset the score and start over, press the Reset Score button mentioned above

**The button press sounds are different each time. Repeatedly press either the New Round button or Reset Score button to play a little song (works with any buttons but, since players are limited to making one choice per round, it's easier with the game control buttons)**

## 3rd-party libraries used
- https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library -> for using the 1602 LCD with the I2C interface
- https://github.com/robsoncouto/arduino-songs/tree/master/nevergonnagiveyouup -> used part of this for button sounds and song notes
