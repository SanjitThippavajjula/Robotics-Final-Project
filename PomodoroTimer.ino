#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int startButton = 8;
const int tenMinButton = A0;
const int fiveMinButton = A1;
const int oneMinButton = A2;
const int pauseButton = 10;
const int stopButton = 9;
const int skipButton = A4;

const int redLED = 7;
const int greenLED = A5;
const int buzzer = 13;

unsigned long lastDebounceTime = 0;
unsigned long lastActivityTime = 0;
unsigned long lastPressTime = 0;

unsigned long timerStart = 0;
unsigned long previousMillis = 0;
unsigned long countdownDuration = 0;
unsigned long originalWorkDuration = 0;

bool isPaused = false;
bool inWorkMode = true;
bool timerRunning = false;
bool skipRequested = false;

int sessionCount = 0;
int workDuration = 0;
int currentCountdown = 0;

void setup() {
  lcd.begin(16, 2);
  pinMode(startButton, INPUT);
  pinMode(tenMinButton, INPUT);
  pinMode(fiveMinButton, INPUT);
  pinMode(oneMinButton, INPUT);
  pinMode(pauseButton, INPUT);
  pinMode(stopButton, INPUT);
  pinMode(skipButton, INPUT);
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  splashScreen();
  showSelectWorkTime();
}

void splashScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pomodoro Timer");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
}

void showSelectWorkTime() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Work Time:");
  lcd.setCursor(0, 1);
  lcd.print("Min: ");
  lcd.print(workDuration);
  lcd.print(" Min");
}

void loop() {
  handleButtons();

  if (timerRunning && !isPaused) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
      previousMillis = currentMillis;

      if (currentCountdown > 0) {
        currentCountdown--;
        displayTimer(currentCountdown);
      } else {
        handleSessionEnd();
      }
    }
  }

  if (skipRequested) {
    skipRequested = false;
    handleSessionEnd();
  }
}

void handleButtons() {
  if (digitalRead(tenMinButton) == HIGH) {
    delay(10);
    if (millis() - lastPressTime < 1000) {
      workDuration += 10;
      showAddedTime(10);
    } else {
      if (workDuration >= 10) workDuration -= 10;
    }
    showSelectWorkTime();
    lastPressTime = millis();
    delay(300);
  }

  if (digitalRead(fiveMinButton) == HIGH) {
    delay(10);
    if (millis() - lastPressTime < 1000) {
      workDuration += 5;
      showAddedTime(5);
    } else {
      if (workDuration >= 5) workDuration -= 5;
    }
    showSelectWorkTime();
    lastPressTime = millis();
    delay(300);
  }

  if (digitalRead(oneMinButton) == HIGH) {
    delay(10);
    if (millis() - lastPressTime < 1000) {
      workDuration += 1;
      showAddedTime(1);
    } else {
      if (workDuration >= 1) workDuration -= 1;
    }
    showSelectWorkTime();
    lastPressTime = millis();
    delay(300);
  }

  if (digitalRead(startButton) == HIGH) {
    if (workDuration < 10) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Add At least");
      lcd.setCursor(0, 1);
      lcd.print("10 Min Work Time");
      delay(3000);
      showSelectWorkTime();
      return;
    }

    originalWorkDuration = workDuration * 60;
    currentCountdown = originalWorkDuration;
    inWorkMode = true;
    sessionCount++;
    startTimer("Work Timer", redLED);
  }

  if (digitalRead(pauseButton) == HIGH && timerRunning) {
    isPaused = !isPaused;
    lcd.setCursor(0, 1);
    lcd.print(isPaused ? "Paused         " : "Resumed        ");
    delay(1000);
  }

  if (digitalRead(stopButton) == HIGH) {
    timerRunning = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press Start to");
    lcd.setCursor(0, 1);
    lcd.print("Begin Timer");
    delay(2000);
    showSelectWorkTime();
  }

  if (digitalRead(skipButton) == HIGH) {
    lcd.setCursor(0, 1);
    lcd.print("Skip Pressed    ");
    delay(2000);
    skipRequested = true;
  }
}

void showAddedTime(int min) {
  lcd.setCursor(0, 1);
  lcd.print("Added ");
  lcd.print(min);
  lcd.print(" Min     ");
  delay(1000);
}

void displayTimer(int secondsLeft) {
  lcd.setCursor(0, 1);
  int minutes = secondsLeft / 60;
  int seconds = secondsLeft % 60;
  char buffer[17];
  sprintf(buffer, "%02d:%02d Remaining ", minutes, seconds);
  lcd.print(buffer);
}

void startTimer(const char* label, int ledPin) {
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(ledPin, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(label);
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);
  tone(buzzer, 1000, 500);
  currentCountdown = (inWorkMode ? originalWorkDuration : currentCountdown);
  previousMillis = millis();
  timerRunning = true;
}

void handleSessionEnd() {
  tone(buzzer, 2000, 500);
  delay(1000);
  noTone(buzzer);
  timerRunning = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Session ");
  lcd.print(sessionCount);
  lcd.print(" Done");
  delay(2000);

  if (sessionCount % 4 == 0) {
    startBreak("Long Break", originalWorkDuration * 0.75);
  } else {
    startBreak("Short Break", originalWorkDuration * 0.25);
  }
}

void startBreak(const char* label, unsigned long durationSec) {
  inWorkMode = false;
  currentCountdown = durationSec;
  startTimer(label, greenLED);
}
