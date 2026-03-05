#include <WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h> 


// --- Fill in your credentials here ---
#define WIFI_SSID         "Buckeye Home"
#define WIFI_PASS         "BuckeyeHomeEst2013"
#define APP_KEY           "6e937133-51cd-48d1-a77e-13bb88c02884"
#define APP_SECRET        "46f1fde3-5949-4986-9770-bf337407cef4-6680d578-8ed0-4428-84d8-98a5acc33161"
#define SWITCH_ID         "69954ef05987ba9b0604bfb0"

// --- Hardware Pins ---
#define STEP_PIN 18
#define DIR_PIN 19
#define ENABLE_PIN 21
#define BUTTON_PIN 23
#define LDR_PIN 34

// --- Motor & Logic Constants ---
const int stepsPerRevolution = 3200;
const int stepsToOpenClose = stepsPerRevolution * 4;
const int lightThreshold = 2500;
const int darkThreshold = 200;
const unsigned long oneHourInterval = 3600000;

// --- State Variables ---
bool isOpen = true;
bool lastButtonState = HIGH;
bool manualOverrideActive = false;
unsigned long previousMillis = 0;

// --- Alexa/App Callback ---
// This function runs when you toggle the button on your phone or talk to Alexa
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Request received: %s\n", state ? "Open" : "Close");
  
  if (state != isOpen) {
    openCloseRotate2();
    previousMillis = millis(); // Treat Alexa command as a manual override
    manualOverrideActive = true;
  }
  return true; 
}

void setup() {

  Serial.begin(115200);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LDR_PIN, INPUT);
  
  digitalWrite(ENABLE_PIN, HIGH); // Start with motor driver disabled (off)
  
  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" Connected!");

  // Initialize SinricPro
  SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
  mySwitch.onPowerState(onPowerState);
  SinricPro.begin(APP_KEY, APP_SECRET);
  
  Serial.println("Setup complete and connected to SinricPro");
}

void loop() {
  // Keeps the connection to Alexa/Phone alive
  SinricPro.handle(); 

  // Create a reference so the compiler knows this is a Switch device
  SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];

  int lightValue = analogRead(LDR_PIN);
  bool buttonState = digitalRead(BUTTON_PIN);

  // 1. Physical Button Logic
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Manual physical toggle");
    openCloseRotate2();
    previousMillis = millis();
    manualOverrideActive = true;
    
    mySwitch.sendPowerStateEvent(isOpen);
  }
  lastButtonState = buttonState;

  // 2. Manual Override Timer
  if (manualOverrideActive && millis() - previousMillis >= oneHourInterval) {
    manualOverrideActive = false;
    Serial.println("Manual override expired. Returning to Auto (LDR) mode.");
  }

  // 3. Automatic LDR Logic (Only runs if no manual override is active)
  if (!manualOverrideActive) {
    if (lightValue < darkThreshold && isOpen) {
      Serial.println("Closing blinds for darkness");
      openCloseRotate2();
      mySwitch.sendPowerStateEvent(false); 
    } else if (lightValue > lightThreshold && !isOpen) {
      Serial.println("Opening blinds for light");
      openCloseRotate2();
      mySwitch.sendPowerStateEvent(true); 
    }
  }

  delay(10); 
}

// Function to handle the motor movement with acceleration/deceleration
void openCloseRotate2() {
  if (isOpen) {
    Serial.println("Closing Blinds...");
    digitalWrite(DIR_PIN, HIGH);
  } else {
    Serial.println("Opening Blinds...");
    digitalWrite(DIR_PIN, LOW);
  }

  digitalWrite(ENABLE_PIN, LOW); // Enable motor driver
  
  const int totalSteps = stepsToOpenClose;
  const int rampSteps = 800;
  const int startDelay = 1000;
  const int cruiseDelay = 250;

  for (int i = 0; i < totalSteps; i++) {
    int stepDelay = cruiseDelay;
    
    // Acceleration
    if (i < rampSteps) {
      stepDelay = startDelay - (startDelay - cruiseDelay) * i / rampSteps;
    } 
    // Deceleration
    else if (i > totalSteps - rampSteps) {
      stepDelay = startDelay - (startDelay - cruiseDelay) * (totalSteps - i) / rampSteps;
    }

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(stepDelay);
    
    // Feed the SinricPro watchdog every 100 steps to prevent disconnects
    if (i % 100 == 0) SinricPro.handle();
  }
  
  digitalWrite(ENABLE_PIN, HIGH); // Disable motor driver to save power/heat
  isOpen = !isOpen;
}