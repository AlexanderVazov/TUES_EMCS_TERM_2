#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20, 16, 2);

unsigned int currentStage;
unsigned long startTime = 0;
const unsigned long reqT1 = 1000;
const unsigned long reqT2 = 2000;

class Player {
  unsigned int R_LED;
  unsigned int G_LED;
  unsigned int B_LED;
public:
  unsigned int button;
  bool finished;
  unsigned long timeTaken;
  Player(int R_LED, int G_LED, int B_LED, int button) {
    this->R_LED = R_LED;
    this->G_LED = G_LED;
    this->B_LED = B_LED;
    this->button = button;
    pinMode(this->R_LED, OUTPUT);
    pinMode(this->G_LED, OUTPUT);
    pinMode(this->B_LED, OUTPUT);
    pinMode(this->button, INPUT_PULLUP);
  }
  void setColor(int red, int green, int blue) {
    analogWrite(R_LED, red);
    analogWrite(G_LED, green);
    analogWrite(B_LED, blue);
  }
};

Player p1(12, 11, 10, 13);
Player p2(5, 4, 3, 2);

void setColorBoth(int r = 0, int g = 0, int b = 0);
void lcd_print(String string, int row = 0);
void lcd_clear(int row = 0);
void game();
void stage(int stage = -1);

void setup() {
  lcd.init();
  lcd.backlight();
  randomSeed(analogRead(0));
}

void loop() {
  while (digitalRead(p1.button) == LOW && digitalRead(p2.button) == LOW) {
    if (startTime == 0) {
      startTime = millis();
      stage(0);
    } else if (millis() - startTime >= reqT2) {
      stage(2);
    } else if (millis() - startTime >= reqT1) {
      stage(1);
    }
    if (!(digitalRead(p1.button) == LOW && digitalRead(p2.button) == LOW)) stage(-1);
  }
}

void setColorBoth(int r, int g, int b) {
  p1.setColor(r, g, b);
  p2.setColor(r, g, b);
}

void lcd_print(String string, int row) {
  lcd_clear(row);
  if (row < 0) row = 0;
  else if (row > 1) row = 1;
  lcd.setCursor(0, row);
  lcd.print(string);
}

void lcd_clear(int row) {
  lcd.setCursor(0, row);
  lcd.print("                ");
}

void game() {
  setColorBoth();
  unsigned int TTW = random(0, 5000);
  String string;
  delay(TTW);
  setColorBoth(0, 0, 255);
  unsigned int start = millis();
  lcd_print("Game has started", 0);
  while (!p1.finished && !p2.finished) {
    if (p1.finished && !p2.finished) {
      string = String(p1.timeTaken) + "|" + String(millis() - start);
    } else if (!p1.finished && p2.finished) {
      string = String(millis() - start) + "|" + String(p2.timeTaken);
    } else string = String(millis() - start) + "|" + String(millis() - start);
    lcd_print(string, 1);
    if (digitalRead(p1.button) == LOW) {
      p1.timeTaken = millis() - start;
      p1.finished = true;
    }
    if (digitalRead(p2.button) == LOW) {
      p2.timeTaken = millis() - start;
      p2.finished = true;
    }
  }
  p1.finished = false;
  p2.finished = false;
  string = String(p1.timeTaken) + "|" + String(p2.timeTaken);
  lcd_print(string, 1);
  if (p1.timeTaken == p2.timeTaken) {
    setColorBoth(255, 255, 0);
    lcd_print("Tie!", 0);
  } else if (p1.timeTaken < p2.timeTaken) {
    p1.setColor(0, 255, 0);
    p2.setColor(255, 0, 0);
    lcd_print("Player 1 wins!", 0);
  } else if (p1.timeTaken > p2.timeTaken) {
    p1.setColor(255, 0, 0);
    p2.setColor(0, 255, 0);
    lcd_print("Player 2 wins!", 0);
  }
}

void stage(int stage) {
  switch (stage) {
    default:
    case -1:
      lcd_print("Awaiting start", 0);
      setColorBoth();
      break;
    case 0:
      setColorBoth(255, 0, 0);
      lcd_print("Starting in 3", 0);
      break;
    case 1:
      setColorBoth(255, 255, 0);
      lcd_print("Starting in 2", 0);
      break;
    case 2:
      setColorBoth(0, 255, 0);
      lcd_print("Starting in 1", 0);
      game();
      break;
  }
}