#include <Joystick.h>

#define genesisRigth    A3
#define genesisLeft     A2
#define genesisDown     A1
#define genesisUp       A0
#define genesisB        16
#define genesisSelect   14
#define genesisC        15

//define DEBUG

enum controllerType {
  _6Button=0,
  _3Button,
  _unKnown =0xff
};

enum buttonType{
  _A=0, 
  _B, 
  _C, 
  _X, 
  _Y, 
  _Z
};

uint16_t combinedButtons = 0;
byte buttonShooting[] = {0, 0, 0, 0, 0, 0};
bool buttonState[] = {false, false, false, false, false, false};
byte hotkeys[] = {0, 0};

Joystick_ Joystick( JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  12, 0,
  true, true, false,
  false, false, false,
  false, false,
  false, false, false);
  
void buttonPress(byte type, uint16_t btBit, uint16_t pressId){  
  if(buttonShooting[type]<4){
    if (combinedButtons & btBit){
      if (!buttonState[type] && hotkeys[0] && hotkeys[1]){
        if(buttonShooting[type] == 0){
          buttonShooting[type] = 1;
        }else{
          buttonShooting[type] = 0;
        }
      }      
      buttonState[type] = true;  
    }else{
      buttonState[type] = false;
    }
    Joystick.setButton(pressId, buttonState[type]);
  }else{
    Joystick.setButton(pressId, 0);
  }  
}

void setup() {
  pinMode(genesisUp    ,INPUT_PULLUP);
  pinMode(genesisDown  ,INPUT_PULLUP);
  pinMode(genesisLeft  ,INPUT_PULLUP);
  pinMode(genesisRigth ,INPUT_PULLUP);
  pinMode(genesisB     ,INPUT_PULLUP);
  pinMode(genesisC     ,INPUT_PULLUP);
  pinMode(genesisSelect,OUTPUT);
  digitalWrite(genesisSelect,LOW);

  Joystick.begin();
  Joystick.setXAxisRange(-1, 1);
  Joystick.setYAxisRange(-1, 1);
  
#if defined (DEBUG)  
  Serial.begin(9600);
  Serial.println("starting...");
#endif
}

void loop() {
  int16_t axis;
  
  SEGAscan();
  axis=0;
  if (combinedButtons & (1<<0)){
    axis = -1;
    hotkeys[0] = 1;
  }else{
    hotkeys[0] = 0;
  }
  if (combinedButtons & (1<<1)) {
    axis = 1;    
  }
  Joystick.setYAxis(axis);

  axis=0;
  if (combinedButtons & (1<<2)) axis = -1;
  if (combinedButtons & (1<<3)) axis = 1;
  Joystick.setXAxis(axis);
  
  if (combinedButtons & (1<<7)){
    Joystick.setButton(10, 1);
    hotkeys[1] = 1;
  }else{
    Joystick.setButton(10, 0);
    hotkeys[1] = 0;
  }
  if (combinedButtons & (1<<11)){
    Joystick.setButton(11, 1);
  }else{
    Joystick.setButton(11, 0);
  }

  for(int i=0; i< 6; i++){
    if(buttonShooting[i]>0){
      buttonShooting[i]++;
      if(buttonShooting[i]>5)
        buttonShooting[i] = 1;
    }
  }

  buttonPress( _A, (1<<6), 0);
  buttonPress( _B, (1<<4), 1);
  buttonPress( _C, (1<<5), 4);
  buttonPress( _X, (1<<10), 3);
  buttonPress( _Y, (1<<9), 6);
  buttonPress( _Z, (1<<8), 7);
      
#if defined (DEBUG) 
    Serial.print("Dpad: ");
    if (combinedButtons & (1<<0)) Serial.print ("U"); else Serial.print("-");
    if (combinedButtons & (1<<1)) Serial.print ("D"); else Serial.print("-");
    if (combinedButtons & (1<<2)) Serial.print ("L"); else Serial.print("-");
    if (combinedButtons & (1<<3)) Serial.print ("R"); else Serial.print("-");
        
    Serial.print(" Buttons: ");
    
    if (combinedButtons & (1<<6)) Serial.print ("A"); else Serial.print("-");
    if (combinedButtons & (1<<4)) Serial.print ("B"); else Serial.print("-");
    if (combinedButtons & (1<<5)) Serial.print ("C"); else Serial.print("-");
        
    if (combinedButtons & (1<<10)) Serial.print (" X"); else Serial.print(" -");
    if (combinedButtons & (1<<9)) Serial.print ("Y"); else Serial.print("-");
    if (combinedButtons & (1<<8)) Serial.print ("Z"); else Serial.print("-");
    
    if (combinedButtons & (1<<7)) Serial.print (" S"); else Serial.print(" -");
    if (combinedButtons & (1<<11)) Serial.print ("M"); else Serial.print("-");
    Serial.println();
#endif
  delay(10);
}

uint8_t SEGAscan(void) {
  uint8_t sample[7]; 
  uint8_t type;  
  combinedButtons = 0;
  
  sample[0] = readController();
  
  digitalWrite(genesisSelect,HIGH);
  delayMicroseconds(10); 
  sample[1] = readController();

  digitalWrite(genesisSelect,LOW);
  delayMicroseconds(10);
  sample[2] = readController();
 
  digitalWrite(genesisSelect,HIGH);
  delayMicroseconds(10);
  sample[3] = readController();

  digitalWrite(genesisSelect,LOW);
  delayMicroseconds(10);
  sample[4] = readController();

  digitalWrite(genesisSelect,HIGH);
  delayMicroseconds(10);
  sample[5] =  readController();

  digitalWrite(genesisSelect,LOW);
  delayMicroseconds(10);
  sample[6] = readController();
    
  if ( ((sample[4] & 0x03) == 0) && ((sample[6] & 0x0f)==0x0f) ) {
  type = _6Button;  
  } else if  ( (sample[6] & 0x0c) == 0)  {
  type = _3Button;
  } else
    type = _unKnown;

#if defined (DEBUG) 
   Serial.print("Type: ");
   switch (type) {
    case _6Button:
    Serial.println("6 button");
    break;
    case _3Button:
    Serial.println("3 button");
    break;    
    default:
    Serial.println("Unknown/Master System");
    break;      
    }
#endif    

  combinedButtons = (uint16_t)sample[1];
  combinedButtons |= ((uint16_t)(sample[0]<<2)) & 0xc0;
  combinedButtons |= ((uint16_t)(sample[5]<<8)) & 0xf00;

  combinedButtons = ~combinedButtons;
  switch (type) {
    case _6Button:
      combinedButtons &= 0x0fff;
      break;
    case _3Button:
      combinedButtons &= 0x00ff;
      break;      
    default:
      combinedButtons &= 0x003f;    
    }
  return type;
}

uint8_t readController() {

  uint8_t buttons=0;

  if( digitalRead(genesisUp)   ) buttons |= (1<<0);
  if( digitalRead(genesisDown) ) buttons |= (1<<1);
  if( digitalRead(genesisLeft) ) buttons |= (1<<2);
  if( digitalRead(genesisRigth)) buttons |= (1<<3);
  if( digitalRead(genesisB)    ) buttons |= (1<<4);
  if( digitalRead(genesisC)    ) buttons |= (1<<5);  
#if defined (DEBUG) 
   Serial.println(buttons | 128,BIN);
#endif  
  return buttons;
}
