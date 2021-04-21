#include "LedControl.h" //  need the library
#include <LiquidCrystal.h>
LiquidCrystal lcd(3,4,7,8,9,13); // 3 4 7 8 9 13
LedControl lc = LedControl(12, 11, 10, 1); //DIN, CLK, LOAD, No. DRIVER
 
// pin 12 is connected to the MAX7219 pin 1
// pin 11 is connected to the CLK pin 13
// pin 10 is connected to LOAD pin 12
// 1 as we are only using 1 MAX7219

//---DEFINE SECTION---
#define TRIG_PIN 5
#define ECHO_PIN 6
#define BUTTON_PIN 2

//---DECLARE VARIABLES---
unsigned long startTimeGame = micros(), timeGame = 0, score = 0;
float delayTime = 1200;
int carPositionPre, carPosition = 0, distMicro, mapPosition = 0;
float distanceObject;
short gameStatus = 0, numberOfLives = 3;
char matrix[45][8] //game's map. It's a loop map.
{
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 0, 0, 0, 0, 1},
  {1, 1, 1, 0, 0, 0, 0, 1},
  {1, 1, 1, 0, 0, 0, 1, 1},
  {1, 1, 1, 0, 0, 0, 1, 1},
  {1, 0, 0, 0, 0, 0, 1, 1},
  {1, 0, 0, 0, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 1, 1, 1},
  {1, 0, 0, 0, 1, 1, 1, 1},
  {1, 0, 0, 0, 1, 1, 1, 1},
  {1, 0, 0, 0, 1, 1, 1, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 0, 0, 0, 0, 1, 1},
  {1, 1, 0, 0, 0, 0, 1, 1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 1, 0, 0, 0, 0, 1, 1},
  {1, 1, 0, 0, 0, 0, 1, 1},
  {1, 1, 0, 0, 0, 0, 1, 1},
  {1, 1, 0, 0, 0, 0, 1, 1},
  {1, 1, 1, 0, 0, 0, 1, 1},
  {1, 1, 1, 0, 0, 0, 0, 1},
  {1, 1, 1, 1, 0, 0, 0, 1},
  {1, 1, 1, 1, 0, 0, 0, 1},
  {1, 1, 1, 1, 0, 0, 0, 1},
  {1, 1, 0, 0, 0, 0, 0, 1},
  {1, 1, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 0, 0, 0, 0, 1},
  {1, 0, 0, 1, 1, 0, 0, 1},
  {1, 0, 0, 1, 1, 0, 0, 1},
  {0, 0, 0, 1, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 0, 0, 0, 1, 1},
  {1, 1, 1, 0, 0, 0, 1, 1},
  {1, 0, 0, 0, 0, 1, 1, 1},
  {1, 0, 0, 0, 0, 1, 1, 1}
};
 
void setup()
{
  // the zero refers to the MAX7219 number, it is zero for 1 chip
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, 2); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lcd.begin(16,2);
  lcd.clear();

}

//---SENSOR---
//Read the imput from ultrasonic sensor and measure the distance for motion control
void readSensor()
{
  digitalWrite(TRIG_PIN, LOW); //clear trig
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  distMicro = pulseIn(ECHO_PIN, HIGH); // time in microseconds
  distanceObject = 0.034 * distMicro / 2;
}

//Sometimes the sensor returns odd numbers like -100 and if you move your hand just a little it can be unstable, so there is an error measure
void calibrateSensor() 
{
    if (distanceObject > 3 && distanceObject < 27)
      if (abs(carPosition - distanceObject) > 1) // here i calculate the error
      {
        carPositionPre = carPosition;
        carPosition = distanceObject; 
      }
}

//--- GRAPHICS 8x8 MATRIX---
void displayCar()
{
  if (carPositionPre != carPosition)
  {
    lc.setLed(0, 7, carPositionPre / 3 - 1, false);
    lc.setLed(0, 7, carPosition / 3 - 1, true);  
  }
  else lc.setLed(0, 7, carPosition / 3 - 1, true);
}

void displayMap()
{
   for (int i = 7; i >= 0; i--)
    for (int j = 7; j >= 0; j--)
      lc.setLed(0, i, j, matrix[(7 - i + mapPosition) % 45][j]); // i use %45 because my map is 45x8 and in
}

//---GAME MECHANICS---
void verifyColision()
{
  for (int j = 0; j <= 7; j++)
  {
    if (matrix[(mapPosition) % 45][j] == 1 && j == carPosition / 3 - 1)
      endOfLife();
  }
}

void endOfLife()
{
    for (int i = 0; i <= 7; i++)
    for (int j = 0; j <= 7; j++)
    {
      if (i == j)
        lc.setLed(0, i, j, true);
      else if (7 - i == j)
        lc.setLed(0, i, j, true);
      else lc.setLed(0, i, j, false);
    }  
    numberOfLives--;
    if (numberOfLives <= 0)
      gameStatus = 0;
    else gameStatus = 2;
}

void changeLevel()
{
  delayTime = delayTime - 10;
}

//---LCD---
void displayScoreLife()
{
    clearScreen();
    lcd.setCursor(0, 0);
    lcd.print("Lives = ");
    lcd.print(numberOfLives);
    lcd.setCursor(0, 1);
    lcd.print("Score = ");
    lcd.print(score / 1000); 
}

void clearScreen()
{
  lcd.setCursor (0, 0);
  for (int i = 0; i < 16; ++i)
  {
    lcd.write(' ');
  }
  lcd.setCursor (0, 1);
  for (int i = 0; i < 16; ++i)
  {
    lcd.write(' ');
  }
}

void displayStartGame()
{
    clearScreen();
    lcd.setCursor(0, 0);
    lcd.print("Press Start");
    lcd.setCursor(0, 1);
    lcd.print("Score = ");
    lcd.print(score / 1000);
}

void scoreCalculus()
{
  score = score + (millis() - timeGame);
}

//---GAME STATUS AND BASIC INSTRUCTIONS---
void loop()
{
  if(gameStatus == 0)// this is game over or start of the game
  {
    displayStartGame();
    readSensor();
    calibrateSensor();
    displayMap();
    displayCar();
    mapPosition = 0;
    numberOfLives = 3;
    delayTime = 1200;
    if (digitalRead(BUTTON_PIN) == 0)
    {
      gameStatus = 1;
      score = 0;
      timeGame = millis();
    }
      
  }
  if (gameStatus == 1) // this means game is running
  {
    timeGame = millis();
    displayScoreLife();
    readSensor();
    calibrateSensor();
    displayCar();
    if (millis() - startTimeGame > delayTime)
    {
      displayMap();
      mapPosition++;
      startTimeGame = millis();
    }
    verifyColision();
    if (mapPosition % 20 == 0)
      changeLevel();
    scoreCalculus();
  }
  if (gameStatus == 2) //this means game is paused because of death, freeze and invincibility for 5s.
  {
    displayScoreLife();
    readSensor();
    calibrateSensor();
    displayCar();
    displayMap();
    if (digitalRead(BUTTON_PIN) == 0)
      gameStatus = 1;
  } 
}
