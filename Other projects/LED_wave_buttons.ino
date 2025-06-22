unsigned int leds[4] = {11, 10, 9, 6};
unsigned int buttons[3] = {4, 3, 2};
bool inverted = false;
unsigned int TTW = 800;
unsigned int counter = 0;
int intensityMode = 3;
int intensityModes[4] = {128, 153, 191, 255};
unsigned long lastIterationOfWaveLoop = 0;
unsigned long lastButtonPress = 0;

void setup() {
  for(int i = 0; i < 4; i++) pinMode(leds[i], OUTPUT);
  for(int i = 0; i < 3; i++) pinMode(buttons[i], INPUT);
}

void loop() {
  if(millis() - lastIterationOfWaveLoop > TTW){
    counter %= 4;
    if(!inverted){
      analogWrite(leds[(counter + 3) % 4], 0);
      analogWrite(leds[counter], intensityModes[intensityMode]);
    } else {
      analogWrite(leds[(counter + 3) % 4], intensityModes[intensityMode]);
      analogWrite(leds[counter], 0);
    }
    lastIterationOfWaveLoop = millis();
    counter++;
  }
  if(millis() - lastButtonPress > 200){
    if(digitalRead(buttons[0]) == HIGH && TTW > 100)TTW -= 100;
    if(digitalRead(buttons[1]) == HIGH){
      inverted = !inverted;
      for(int i = 0; i <= 3 && inverted; i++)if(i!=counter)(analogWrite(leds[i], intensityModes[intensityMode]));
      for(int i = 0; i <= 3 && !inverted; i++)if(i!=counter)(analogWrite(leds[i], 0));
    }
    if(digitalRead(buttons[2]) == HIGH){
      intensityMode = (intensityMode + 1) % 4;
    }
    lastButtonPress = millis();
  }
}
