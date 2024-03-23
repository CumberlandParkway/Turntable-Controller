// control stepper from another arduino. Connect output pin from mega to input pin on uno/nano
// 9600 steps is a full 360 deg rotation

#include <AccelStepper.h>

// Define the stepper motor and the pins that is connected to

const int stepPin = 3;
const int directionPin = 4;
const int enablePin = 5;

AccelStepper stepper(AccelStepper::DRIVER, stepPin, directionPin);

#define GoToZero 6
#define GoToExit_1 7
#define GoToExit_2 8
#define GoToExit_3 9
#define GoToExit_4 11
#define GoToExit_5 12
#define GoToExit_6 0
#define GoToExit_7 1

#define RotateLimitSwitch 10
int TurntableHomeSteps = 396; // adjust this to get initial start position

void MoveStepper(int i){
stepper.setAcceleration(50); // Set acceleration value for the stepper
stepper.moveTo(i);
while (stepper.currentPosition() != i) {stepper.run();}
}

void TurntableHome() {
stepper.setMaxSpeed(1000); // Set maximum speed value for the stepper
stepper.setAcceleration(50); // Set acceleration value for the stepper
stepper.setCurrentPosition(0); // Set the current position to 0 steps

// Move motor at normal speed until limit switch is triggered
stepper.moveTo(-6500);
while (digitalRead(RotateLimitSwitch) == 1) {stepper.run();}
stepper.stop();
stepper.setCurrentPosition(0);

// reverse stepper for 200 steps
stepper.moveTo(200);
while (stepper.currentPosition() != 200) {stepper.run();}
stepper.setCurrentPosition(0);

// move slowly until limit switch is triggered
stepper.setMaxSpeed(500); // Set maximum speed value for the stepper
stepper.moveTo(-300);
while (digitalRead(RotateLimitSwitch) == 1) {stepper.run();}
stepper.stop();
stepper.setCurrentPosition(0);

// back away from microswitch to set home position
stepper.moveTo(TurntableHomeSteps);
while (stepper.currentPosition() != TurntableHomeSteps) {stepper.run();}

stepper.setMaxSpeed(1000); // Set maximum speed value for the stepper
stepper.setAcceleration(50); // Set acceleration value for the stepper
stepper.setCurrentPosition(0); // Set the current position to 0 steps
}

void setup(){
stepper.setEnablePin(enablePin);
stepper.setPinsInverted(false, false, true);
stepper.enableOutputs();

pinMode(GoToZero, INPUT_PULLUP);
pinMode(GoToExit_1, INPUT_PULLUP);
pinMode(GoToExit_2, INPUT_PULLUP);
pinMode(GoToExit_3, INPUT_PULLUP);
pinMode(GoToExit_4, INPUT_PULLUP);
pinMode(GoToExit_5, INPUT_PULLUP);
pinMode(GoToExit_6, INPUT_PULLUP);
pinMode(GoToExit_7, INPUT_PULLUP);
pinMode(RotateLimitSwitch, INPUT_PULLUP);

// Home the turntable
TurntableHome();
}

void loop(){
if(digitalRead(GoToZero) == 0){MoveStepper(0);}
if(digitalRead(GoToExit_1) == 0){MoveStepper(370);}
if(digitalRead(GoToExit_2) == 0){MoveStepper(700);}
if(digitalRead(GoToExit_3) == 0){MoveStepper(1075);}
if(digitalRead(GoToExit_4) == 0){MoveStepper(4810);}
if(digitalRead(GoToExit_5) == 0){MoveStepper(5180);}
if(digitalRead(GoToExit_6) == 0){MoveStepper(5500);}
if(digitalRead(GoToExit_7) == 0){MoveStepper(5880);}
}
