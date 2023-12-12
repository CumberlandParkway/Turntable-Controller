// Define arduino pin connections
#define TurntableEnablePin 3
#define TurntableStep 4
#define TurntableDir 5
#define RotateLimitSwitch 6
#define RotateButton 7
#define ButtonCW 8
#define ButtonACW 9

int TurntableHomeSteps = 340; // adjust this to get initial start position
int Rot180DegSteps = 4792; // (200*16*3)/2 gives 180 deg motor.
int TurntableHomeSpeed = 8000;
int TurntableNormalSpeed = 2000;

//Common variables
int i = 0;
int x = 0;
int RotDir = 0; // 0=ACW(HIGH) 1=CW (LOW)

void TurntableHome() {
// Enable the stepper motor
digitalWrite(TurntableEnablePin, LOW);

// Move motor at normal speed until rough home position reached
digitalWrite(TurntableDir, HIGH); // towards home position ACW
while (digitalRead(RotateLimitSwitch) == 1) {
digitalWrite(TurntableStep, HIGH);
delayMicroseconds(TurntableHomeSpeed);
digitalWrite(TurntableStep, LOW);
delayMicroseconds(TurntableHomeSpeed);
}
delay(1000);
// reverse stepper for 80 steps
digitalWrite(TurntableDir, LOW); // away from home position CW
for(i = 0; i < 80; i++) {
digitalWrite(TurntableStep,HIGH); 
delayMicroseconds(TurntableHomeSpeed); 
digitalWrite(TurntableStep,LOW); 
delayMicroseconds(TurntableHomeSpeed); 
}
delay(1000);
// move slowing towards home position
digitalWrite(TurntableDir, HIGH);
while (digitalRead(RotateLimitSwitch) == 1) {
digitalWrite(TurntableStep, HIGH);
delayMicroseconds(TurntableHomeSpeed);
digitalWrite(TurntableStep, LOW);
delayMicroseconds(TurntableHomeSpeed);
}
delay(1000);
// back away from microswitch to set home position
digitalWrite(TurntableDir, LOW);
for(i = 0; i < TurntableHomeSteps; i++) {
digitalWrite(TurntableStep,HIGH); 
delayMicroseconds(TurntableHomeSpeed); 
digitalWrite(TurntableStep,LOW); 
delayMicroseconds(TurntableHomeSpeed); 
}
delay(1000);
// Disable the stepper motor
digitalWrite(TurntableEnablePin, HIGH);
RotDir = 1;
}

void TurntableRotate(){
// Enable the stepper motor
digitalWrite(TurntableEnablePin, LOW);

if(RotDir == 1){digitalWrite(TurntableDir, LOW); RotDir = 0;}
else{digitalWrite(TurntableDir, HIGH); RotDir = 1;}

for(i = 0; i < Rot180DegSteps; i++) {
digitalWrite(TurntableStep,HIGH); 
delayMicroseconds(TurntableNormalSpeed); 
digitalWrite(TurntableStep,LOW); 
}
// Disable the stepper motor
digitalWrite(TurntableEnablePin, HIGH);
}

void RotateCw(){
digitalWrite(TurntableEnablePin, LOW);
digitalWrite(TurntableDir, LOW);
digitalWrite(TurntableStep,HIGH); 
delayMicroseconds(TurntableHomeSpeed); 
digitalWrite(TurntableStep,LOW);
digitalWrite(TurntableEnablePin, HIGH);
}

void RotateAcw(){
digitalWrite(TurntableEnablePin, LOW);
digitalWrite(TurntableDir, HIGH);
digitalWrite(TurntableStep,HIGH); 
delayMicroseconds(TurntableHomeSpeed); 
digitalWrite(TurntableStep,LOW);
digitalWrite(TurntableEnablePin, HIGH);
}

void setup() {
// set pin modes for buttons, relays and stepper controls
pinMode(RotateButton, INPUT_PULLUP);
pinMode(ButtonCW, INPUT_PULLUP);
pinMode(ButtonACW, INPUT_PULLUP);
pinMode(RotateLimitSwitch, INPUT_PULLUP);
pinMode(TurntableDir, OUTPUT);
pinMode(TurntableStep, OUTPUT);
pinMode(TurntableEnablePin, OUTPUT);

// disable the stepper motors
digitalWrite(TurntableEnablePin, HIGH);

// Home the turntable
TurntableHome();
}

void loop() {
if(digitalRead(RotateButton) == 0){TurntableRotate();}
if(digitalRead(ButtonCW) == 0){RotateCw(); delay(300);}
if(digitalRead(ButtonACW) == 0){RotateAcw(); delay(300);}
}
