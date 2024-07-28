/*
control stepper from another arduino. Connect output pin from mega to input pin on uno/nano/esp32
Must have common ground
9600 steps is a full 360 deg rotation - 
*/
#include <Wire.h>
#include <U8x8lib.h>
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);

#include <ESP32Encoder.h> // https://github.com/madhephaestus/ESP32Encoder.git 
ESP32Encoder encoder;

#include "OneButton.h"
OneButton RotateButton(4, true);

#include <AccelStepper.h>
AccelStepper stepper(AccelStepper::DRIVER, 13, 14); // step pin, direction pin

#include <Preferences.h> // permanent data storage
Preferences prefs;

// Define the stepper motor enable pin
const int enablePin = 5;

#define RotateLimitSwitch 18
#define AutoReverse 19

String TrackPosition[8] = {"A1", "A2", "A3", "A4", "B1", "B2", "B3", "B4"};
int RetrieveSteps[8];
int TrackSteps[8] = {0, 250, 550, 850, 4800, 5050, 5350, 5650}; // default values if none are saved to eeprom

int RetrievedHomeSteps;
int PreSetSteps;
int NewSteps;
int OldSteps;
int Increment;

// Turntable position variables
  long NewPosition = 0;
  long OldPosition = 0;
  long SetPosition = 0;

// Turntable edit menu variables
  bool EditMenu = false;
  bool MoveMenu = true;
  bool EncoderReset = false;

  long NewMenuPosition = 0;
  long OldMenuPosition = 0;
  long SetMenuPosition = 0;

void MovingMessage(String Message){
  u8x8.clear();
  u8x8.setFont(u8x8_font_courB18_2x3_r);
  u8x8.setCursor(0, 0);
  u8x8.print(Message);
  u8x8.setFont(u8x8_font_victoriabold8_r);
  u8x8.setCursor(2, 4);
  u8x8.print("Please Wait");
}

void ShowTurntablePosition(int SetPosition, int NewPosition){
  u8x8.clear();
  u8x8.setFont(u8x8_font_victoriabold8_r);
  u8x8.setCursor(0, 0);
  u8x8.print("CURR");
  u8x8.setCursor(0, 1);
  u8x8.print("POS");
  u8x8.setCursor(0, 4);
  u8x8.print("MOVE");
  u8x8.setCursor(0, 5);
  u8x8.print("TO");
  
  u8x8.setCursor(10, 1);
  u8x8.print(TrackSteps[SetPosition]);
  u8x8.setCursor(10, 5);
  u8x8.print(TrackSteps[NewPosition]);
  
  u8x8.setFont(u8x8_font_courB18_2x3_r);
  u8x8.setCursor(5, 0);
  u8x8.print(TrackPosition[SetPosition]);
  u8x8.setCursor(5, 4);
  u8x8.print(TrackPosition[NewPosition]);
}

void ShowAdjustScreen(int TheOption){
  switch (TheOption){
    case 0:
      u8x8.clear();
      u8x8.setFont(u8x8_font_victoriabold8_r);
      u8x8.setCursor(0, 0);
      u8x8.print("EDIT");
      u8x8.setCursor(0, 1);
      u8x8.print("POS");
  
      u8x8.setCursor(10, 1);
      u8x8.print(TrackSteps[SetPosition]);

      u8x8.setCursor(0, 4);
      u8x8.print("Rotate:   ADJUST");
      u8x8.setCursor(0, 5);
      u8x8.print("Click:      QUIT");
      u8x8.setCursor(0, 6);
      u8x8.print("Dble click: SAVE");

      u8x8.setFont(u8x8_font_courB18_2x3_r);
      u8x8.setCursor(5, 0);
      u8x8.print(TrackPosition[SetPosition]);
     break;
    case 1:
      u8x8.clearLine(0);
      u8x8.clearLine(1);
      u8x8.setFont(u8x8_font_victoriabold8_r);
      u8x8.setCursor(0, 0);
      u8x8.print("EDIT");
      u8x8.setCursor(0, 1);
      u8x8.print("POS");
  
      u8x8.setCursor(10, 1);
      u8x8.print(NewSteps);

      u8x8.setFont(u8x8_font_courB18_2x3_r);
      u8x8.setCursor(5, 0);
      u8x8.print(TrackPosition[SetPosition]);
      break;
    default:
      break;
    }
}

void MoveStepper(int i){
  if(stepper.currentPosition() != i){
    digitalWrite(enablePin, LOW); // enable the stepper motor
    stepper.setAcceleration(50); // Set acceleration value for the stepper
    stepper.moveTo(i);
    while (stepper.currentPosition() != i) {
      stepper.run();
      }
    digitalWrite(enablePin, HIGH); // disable the stepper motor
    if(i >=3000){ // if turntable moves past approx 90 deg then auto reverse
    digitalWrite(AutoReverse, HIGH);
    }
    else{
      digitalWrite(AutoReverse, LOW);
      }
    }
}

void TurntableHome() {
  digitalWrite(enablePin, LOW); // enable the stepper motor
  MovingMessage(" Homing");
  stepper.setMaxSpeed(1000); // Set maximum speed value for the stepper
  stepper.setAcceleration(50); // Set acceleration value for the stepper
  stepper.setCurrentPosition(0); // Set the current position to 0 steps

// Move motor at normal speed until limit switch is triggered
  stepper.moveTo(-6500);
  while (digitalRead(RotateLimitSwitch) == 1) {stepper.run();}
  stepper.stop();
  stepper.setCurrentPosition(0);

// reverse stepper for 100 steps
  stepper.moveTo(100);
  while (stepper.currentPosition() != 100) {stepper.run();}
  stepper.setCurrentPosition(0);

// move slowly until limit switch is triggered
  stepper.setMaxSpeed(50); // Set maximum speed value for the stepper
  stepper.setAcceleration(0); // Set acceleration value for the stepper
  stepper.moveTo(-300);
  while (digitalRead(RotateLimitSwitch) == 1) {stepper.run();}
  stepper.stop();
  stepper.setCurrentPosition(0);

// back away from microswitch to set home position
  stepper.moveTo(PreSetSteps);
  while (stepper.currentPosition() != PreSetSteps) {stepper.run();}

  stepper.setMaxSpeed(1000); // Set maximum speed value for the stepper
  stepper.setAcceleration(50); // Set acceleration value for the stepper
  stepper.setCurrentPosition(0); // Set the current position to 0 steps
  digitalWrite(enablePin, HIGH); // disable the stepper motor
  NewPosition = 0;
  OldPosition = 0;
  SetPosition = 0;
  encoder.clearCount();
  ShowTurntablePosition(SetPosition, NewPosition);
}

void SaveSteps(){
  if(SetPosition == 0){
    prefs.putInt("HomeSteps", PreSetSteps + NewSteps);  // save the home steps to eeprom
    MovingMessage(" Saving");
    delay(2000);
    ESP.restart();  // restart and initiate the homing sequence
    }
  else{
    MovingMessage(" Saving");
    TrackSteps[SetPosition] = NewSteps; // update the tracksteps array
    prefs.putBytes("SavedArray", (byte*)(&TrackSteps), sizeof(TrackSteps)); // save to eeprom

    encoder.setCount(SetPosition*2);  // reset the encoder
    EditMenu = false;
    MoveMenu = true;
    NewSteps = 0;
    OldSteps = 0;
    EncoderReset = false;
    delay(2000);
    ShowTurntablePosition(SetPosition, NewPosition);
    }
  }

void TurntableMove(){
    NewPosition = encoder.getCount() / 2;
    if(NewPosition != OldPosition){
      if(NewPosition == 8 || NewPosition == -1){
        encoder.clearCount();
        OldPosition = 0;
        NewPosition = 0;
        }
      else{
        OldPosition = NewPosition;
        }
       ShowTurntablePosition(SetPosition, NewPosition);
      }   
  }

void TurntableEdit(){
  if(EncoderReset == false){
    NewSteps = TrackSteps[SetPosition];
    OldSteps = TrackSteps[SetPosition];
    encoder.setCount(0);
    EncoderReset = true;
    }
  Increment = (encoder.getCount() / 2) * 4; // multiply the encoder steps by 4 to give greater movement
  if(NewSteps != OldSteps + Increment){
    NewSteps = OldSteps + Increment;
    ShowAdjustScreen(1);
    MoveStepper(NewSteps);   
    }
  }

void MoveToNewPosition(){
  if(NewPosition != SetPosition){
    MovingMessage(" Moving");
    encoder.pauseCount();
    MoveStepper(TrackSteps[NewPosition]);
    SetPosition = NewPosition;
    ShowTurntablePosition(SetPosition, NewPosition);
    encoder.resumeCount();
    }
  }

// ----- RotateButton callback functions

  void RotateButtonclick(){
    if(MoveMenu == true){MoveToNewPosition();} // in normal operation single click will move the turntable from the set position to the new position

    if(EditMenu == true){ // in edit mode single click to abort editing
      EditMenu = false;
      MoveMenu = true;
      EncoderReset = false;
      NewSteps = 0;
      OldSteps = 0;
      encoder.setCount(SetPosition*2);
      MoveStepper(TrackSteps[SetPosition]);
      ShowTurntablePosition(SetPosition, NewPosition);
      }  
    delay(500);
    }

  void RotateButtondoubleclick(){
    if(MoveMenu == true){TurntableHome();} // in normal operation double click will initiate a homing sequence      

    if(EditMenu == true){SaveSteps();}
    delay(500);
    }

  void RotateButtonLongPressStop(){
    if(NewPosition == SetPosition && MoveMenu == true){ // only enter the edit menu if the set and new positions are equal
      EditMenu = true;
      MoveMenu = false;
      ShowAdjustScreen(0);
      }
    delay(500);
    }

void setup() {
  prefs.begin("PositionArray"); //namespace

// Retrieve the stored HomeSteps
  RetrievedHomeSteps = prefs.getInt("HomeSteps", 300); // The second argument is the default value if the preference is not found
  PreSetSteps = RetrievedHomeSteps;

// retrieve the stored track positions and create the TrackSteps array
  prefs.getBytes("SavedArray", &RetrieveSteps, sizeof(RetrieveSteps));
  for(int i=0; i<8; i++){
    if(RetrieveSteps[i] != 0){
      TrackSteps[i] = RetrieveSteps[i];
      }
    }

// link the button functions.
  RotateButton.attachClick(RotateButtonclick);
  RotateButton.attachDoubleClick(RotateButtondoubleclick);
  RotateButton.attachLongPressStop(RotateButtonLongPressStop);

  stepper.setEnablePin(enablePin);
  stepper.setPinsInverted(false, false, true);
  stepper.enableOutputs();
  pinMode(RotateLimitSwitch, INPUT_PULLUP);
  
  pinMode(AutoReverse, OUTPUT);

  stepper.setCurrentPosition(0); // Set the current position to 0 steps

  u8x8.begin();

  encoder.attachHalfQuad ( 26, 27 );
  encoder.setCount ( 0 );
  
// Home the turntable
  TurntableHome();
}

void loop() {
// check button
  RotateButton.tick();

// check encoder
  if(EditMenu == true){TurntableEdit();}
  if(MoveMenu == true){TurntableMove();}
}
