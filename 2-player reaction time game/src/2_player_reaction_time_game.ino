#include <LiquidCrystal_I2C.h>
#include <FastLED.h>

#define NUM_LEDS_PER_PLAYER 1
#define DATA_PIN_P1 11
#define DATA_PIN_P2 6

#define BUTTON_P1 12
#define BUTTON_P2 2

struct Player {
  int button;
  CRGB* led;
  unsigned long reactionTime;
  bool pressed;
};

enum GameState {
  WAITING_FOR_START,
  COUNTDOWN,
  WAITING_RANDOM,
  REACTION_TEST,
  SHOW_RESULTS
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

CRGB p1Led[NUM_LEDS_PER_PLAYER];
CRGB p2Led[NUM_LEDS_PER_PLAYER];

Player players[2];

GameState gameState = WAITING_FOR_START;
unsigned long startTime;
int falseStartPlayer = -1;

void setup() {
  Serial.begin(9600);
  
  players[0].button = BUTTON_P1;
  players[0].led = p1Led;
  players[0].pressed = false;
  
  players[1].button = BUTTON_P2;
  players[1].led = p2Led;
  players[1].pressed = false;
  
  FastLED.addLeds<WS2812, DATA_PIN_P1, RGB>(p1Led, NUM_LEDS_PER_PLAYER);
  FastLED.addLeds<WS2812, DATA_PIN_P2, RGB>(p2Led, NUM_LEDS_PER_PLAYER);
  
  pinMode(BUTTON_P1, INPUT_PULLUP);
  pinMode(BUTTON_P2, INPUT_PULLUP);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Hold down both");
  lcd.setCursor(0, 1);
  lcd.print("buttons to start!");  
  p1Led[0] = CRGB::Black;
  p2Led[0] = CRGB::Black;
  FastLED.show();
  
  randomSeed(analogRead(0));
}

void loop() {
  switch (gameState) {
    case WAITING_FOR_START:
      if (digitalRead(BUTTON_P1) == LOW && digitalRead(BUTTON_P2) == LOW) {
        delay(50);
        gameState = COUNTDOWN;
      }
      break;
      
    case COUNTDOWN:
      runCountdown();
      gameState = WAITING_RANDOM;
      break;
      
    case WAITING_RANDOM:
      {
        players[0].led[0] = CRGB::Black;
        players[1].led[0] = CRGB::Black;
        FastLED.show();
        
        lcd.clear();
        lcd.print("Get ready...");
        
        unsigned long waitStartTime = millis();
        unsigned long waitDuration = random(250, 2000);
        bool falseStart = false;
        falseStartPlayer = -1;
        while (millis() - waitStartTime < waitDuration && !falseStart) {
          if (digitalRead(players[0].button) == LOW) {
            falseStart = true;
            falseStartPlayer = 0;
            break;
          }
          else if (digitalRead(players[1].button) == LOW) {
            falseStart = true;
            falseStartPlayer = 1;
            break;
          }
          delay(1);
        }
        
        if (falseStart) {
          gameState = SHOW_RESULTS;
        } else {
          players[0].led[0] = CRGB::Blue;
          players[1].led[0] = CRGB::Blue;
          FastLED.show();
          
          lcd.clear();
          lcd.print("Press now!");
          
          players[0].pressed = false;
          players[1].pressed = false;
          players[0].reactionTime = 0;
          players[1].reactionTime = 0;
          startTime = millis();
          gameState = REACTION_TEST;
        }
      }
      break;
      
    case REACTION_TEST:
      if (!players[0].pressed && digitalRead(BUTTON_P1) == LOW) {
        if (digitalRead(BUTTON_P1) == LOW) {
          players[0].reactionTime = millis() - startTime;
          players[0].pressed = true;
          Serial.println("Player 1 pressed: " + String(players[0].reactionTime) + "ms");
        }
      }
      
      if (!players[1].pressed && digitalRead(BUTTON_P2) == LOW) {
        if (digitalRead(BUTTON_P2) == LOW) {
          players[1].reactionTime = millis() - startTime;
          players[1].pressed = true;
          Serial.println("Player 2 pressed: " + String(players[1].reactionTime) + "ms");
        }
      }
      
      if (players[0].pressed || players[1].pressed) {
        gameState = SHOW_RESULTS;
      }
      break;
    case SHOW_RESULTS:
      showResults();
      delay(1000);
      while (digitalRead(BUTTON_P1) == HIGH && digitalRead(BUTTON_P2) == HIGH) {
        delay(10);
      }
      p1Led[0] = CRGB::Black;
      p2Led[0] = CRGB::Black;
      FastLED.show();
      players[0].pressed = false;
      players[1].pressed = false;
      players[0].reactionTime = 0;
      players[1].reactionTime = 0;
      lcd.clear();
      lcd.print("Hold down both");
      lcd.setCursor(0, 1);
      lcd.print("buttons to start!");
      gameState = WAITING_FOR_START;
      delay(300);
      break;
  }
}

void runCountdown() {
  players[0].led[0] = CRGB::Red;
  players[1].led[0] = CRGB::Red;
  FastLED.show();
  lcd.clear();
  lcd.print("Starting in 3...");
  delay(500);
  players[0].led[0] = CRGB::Yellow;
  players[1].led[0] = CRGB::Yellow;
  FastLED.show();
  lcd.clear();
  lcd.print("Starting in 2...");
  delay(500);
  players[0].led[0] = CRGB::Green;
  players[1].led[0] = CRGB::Green;
  FastLED.show();
  lcd.clear();
  lcd.print("Starting in 1...");
  delay(500);
}

void showResults() {
  lcd.clear();
  if (falseStartPlayer >= 0) {
    lcd.print("Player " + String(falseStartPlayer + 1) + " made");
    lcd.setCursor(0, 1);
    lcd.print("a false start!");
    
    players[falseStartPlayer].led[0] = CRGB::Red;
    players[1-falseStartPlayer].led[0] = CRGB::Green;
    FastLED.show();
    return;
  }
  
  if (players[0].pressed && players[1].pressed) {
    if (abs((int)(players[0].reactionTime - players[1].reactionTime)) <= 10) {
      lcd.print("It's a tie!");
      lcd.setCursor(0, 1);
      lcd.print(String(players[0].reactionTime) + "ms");

      players[0].led[0] = CRGB::Yellow;
      players[1].led[0] = CRGB::Yellow;
    } else if (players[0].reactionTime < players[1].reactionTime) {
      lcd.print("Player 1 wins!");
      lcd.setCursor(0, 1);
      lcd.print(String(players[0].reactionTime) + "ms");
      
      players[0].led[0] = CRGB::Green;
      players[1].led[0] = CRGB::Red;
    } else {
      lcd.print("Player 2 wins!");
      lcd.setCursor(0, 1);
      lcd.print(String(players[1].reactionTime) + "ms");
      
      players[0].led[0] = CRGB::Red;
      players[1].led[0] = CRGB::Green;
    }
  } else if (players[0].pressed) {
    lcd.print("Player 1 wins!");
    lcd.setCursor(0, 1);
    lcd.print(String(players[0].reactionTime) + "ms");
    
    players[0].led[0] = CRGB::Green;
    players[1].led[0] = CRGB::Red;
  } else if (players[1].pressed) {
    lcd.print("Player 2 wins!");
    lcd.setCursor(0, 1);
    lcd.print(String(players[1].reactionTime) + "ms");
    
    players[0].led[0] = CRGB::Red;
    players[1].led[0] = CRGB::Green;
  }
  FastLED.show();
}
