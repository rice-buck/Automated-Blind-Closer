#define STEP_PIN 18
#define DIR_PIN 19
#define ENABLE_PIN 21
#define BUTTON_PIN 23
#define LDR_PIN 34

//full step
const int stepsPerRevolution = 200;

int lightThreshold = 3000;
int darkThreshold = 1500;

bool isOpen = false;

//store what the button was last doing -- using INPUT_PULLUP --    button not pressed = HIGH      button pressed = LOW
bool lastButtonState = HIGH; //HIGH is default for pullup

unsigned long previousMillis = 0;
const unsigned long oneMinuteInterval = 60000; //one minute in milliseconds

void setup() {
  Serial.begin(115200);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);

  digitalWrite(ENABLE_PIN, HIGH); //motor off
  digitalWrite(DIR_PIN, HIGH); //choose direction

  Serial.println("Setup complete");
}

void loop() {

  int lightValue = analogRead(LDR_PIN);

  Serial.print("LV: ");
  Serial.println(lightValue);

  //gets state of button 
  bool buttonState = digitalRead(BUTTON_PIN);

  //looks for when button is pressed
  if (lastButtonState == HIGH && buttonState == LOW){
    Serial.println("Manual toggle");
    rotateOneRevolution();
    previousMillis = millis(); //start minute timer
  }

  lastButtonState = buttonState;

  //check if override is active
  bool manualOverride = (millis() - previousMillis) < oneMinuteInterval;

  if(!manualOverride) {
  //when its dark
  if(lightValue < darkThreshold && isOpen){
    Serial.println("Closing blinds for darkness");
    rotateOneRevolution();
  }
  //when its light
  if(lightValue > lightThreshold && !isOpen){
    Serial.println("Opening blinds for light");
    rotateOneRevolution();
    }
  } else {
    Serial.println("Maunual Override Active");
  }
  delay(300);
}

//motor retate function 
void rotateOneRevolution(){
  digitalWrite(ENABLE_PIN, LOW); //motor on
  delayMicroseconds(10);

  //check if blinds open or not and choose direction
  if(isOpen) {
    Serial.println("Closing Blinds");
    digitalWrite(DIR_PIN, LOW);
  } else {
    Serial.println("Opening Blinds");
    digitalWrite(DIR_PIN, HIGH);
  }

  delayMicroseconds(10); //give time for DIR to settle

  for (int i = 0; i < stepsPerRevolution; i++){
    //pulses for motor
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1500); //smaller delay = faster motor
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1500);
  }
  isOpen = !isOpen; //toggles state of t/f
  digitalWrite(ENABLE_PIN, HIGH); //disable motor with enable pin to save power and stuff
}



