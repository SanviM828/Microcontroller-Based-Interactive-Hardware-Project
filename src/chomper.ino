 #include <Servo.h>
// ---------------------- PIN DEFINITIONS ----------------------
#define BUTTON_PIN 2
#define RED_LED_PIN 3
#define GREEN_LED_PIN 4
#define SERVO_PIN 9
#define PIEZO_PIN 5


// ---------------------- GLOBAL VARIABLES ---------------------
Servo jawServo;

bool chaosStarted = false;
bool isJawOpen = false;

unsigned long lastButtonPress = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastServoTime = 0;
unsigned long lastSoundTime = 0;
unsigned long chaosStartTime = 0;

const unsigned long debounceDelay = 50;
const unsigned long chaosDuration = 20000;
const unsigned long postChaosPause = 5000;

// --- ACCELERATION VARIABLES ---
// We need variables that can change over time
unsigned long currentBlinkInterval;
unsigned long currentServoInterval;
unsigned long currentSoundInterval;

// --- STARTING SPEEDS (Slow & Heavy) ---
const unsigned long startBlinkInterval = 500;
const unsigned long startServoInterval = 1200; // 1.2 seconds (Very slow start)
const unsigned long startSoundInterval = 1200; // Matches servo start

// --- MAX SPEEDS (The limit of physics) ---
const unsigned long minServoInterval = 200;
const unsigned long minSoundInterval = 100;

// ----------------------------------------------------------------

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(PIEZO_PIN, OUTPUT);

  // Initialize random generator
  randomSeed(analogRead(0));

  jawServo.attach(SERVO_PIN);
  jawServo.write(0);   // closed initially

  digitalWrite(GREEN_LED_PIN, HIGH);  
  Serial.begin(9600);
}

void loop() {
  checkButton();


  if (chaosStarted) {
    handleChaos();
  }
}

// ---------------------- BUTTON HANDLING ------------------------

void checkButton() {
  static bool lastState = HIGH;
  bool readState = digitalRead(BUTTON_PIN);


  if (readState != lastState) {
    lastButtonPress = millis();
  }


  if (millis() - lastButtonPress > debounceDelay) {
    if (readState == LOW && !chaosStarted) {
      Serial.println("Ring removed! CHAOS STARTING!");
      startChaos(); // Call helper function to reset variables
    }
    else if (readState == LOW && chaosStarted) {
      stopChaos();
    }
  }
  lastState = readState;
}

// ---------------------- START CHAOS HELPER ---------------------
void startChaos() {
  chaosStarted = true;
  chaosStartTime = millis();
  digitalWrite(GREEN_LED_PIN, LOW);
 
  // Reset speeds to "Slow Mode" at the start
  currentBlinkInterval = startBlinkInterval;
  currentServoInterval = startServoInterval;
  currentSoundInterval = startSoundInterval;
}

// ---------------------- CHAOS MODE -----------------------------

void handleChaos() {
  unsigned long now = millis();

  // If chaos duration finished
  if (now - chaosStartTime >= chaosDuration) {
    endChaos();
    return;
  }

  // ----------------- RED LED BLINK ---------------------
  if (now - lastBlinkTime >= currentBlinkInterval) {
    lastBlinkTime = now;
    static bool redOn = false;
    redOn = !redOn;
    digitalWrite(RED_LED_PIN, redOn);

    if (redOn) {
      // Accelerate blinking
      currentBlinkInterval = max(50, currentBlinkInterval - 20);
    }
  }

  // ----------------- SERVO MOVEMENT (ACCELERATING) -----
  if (now - lastServoTime >= currentServoInterval) {
    lastServoTime = now;
   
    // Toggles between 0 and 45 degrees
    if (isJawOpen) {
      jawServo.write(0);
    } else {
      jawServo.write(45);
    }
    isJawOpen = !isJawOpen;


    // ACCELERATE SERVO: Decrease interval by 50ms every move
    // Stops accelerating when it hits minServoInterval (200ms)
    if (currentServoInterval > minServoInterval) {
      currentServoInterval -= 50;
    }
  }


  // ----------------- PIEZO "CHOMP" (ACCELERATING) ------
  if (now - lastSoundTime >= currentSoundInterval) {
    lastSoundTime = now;
   
    // "CHOMP" SOUND EFFECT
    // Low frequency (150Hz) + Short Duration (40ms) = Mechanical "Clank"
    // We add a tiny randomizer to make it sound "rusty"
    tone(PIEZO_PIN, random(100, 200), 40);

    // ACCELERATE SOUND: Keep pace with the servo
    if (currentSoundInterval > minSoundInterval) {
      currentSoundInterval -= 50;
    }
  }
}

// ---------------------- END CHAOS (EXPLOSION) ------------------

void endChaos() {
  Serial.println("CHAOS COMPLETE!");
  digitalWrite(RED_LED_PIN, LOW);

  // --- EXPLOSION EFFECT ---
  unsigned long explosionStart = millis();
  while (millis() - explosionStart < 1500) {
    int rumblePitch = random(50, 300);
    tone(PIEZO_PIN, rumblePitch);
    delay(3);
  }
  noTone(PIEZO_PIN);
  // ------------------------

  jawServo.write(0);
  delay(postChaosPause);

  digitalWrite(GREEN_LED_PIN, HIGH);
  chaosStarted = false;
}

// ---------------------- STOP CHAOS -----------------------------

void stopChaos() {
  Serial.println("CHAOS STOPPED!");
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, HIGH);
  noTone(PIEZO_PIN);
  jawServo.write(0);
  chaosStarted = false;
}

