// Include Wire Library for I2C
#include <Wire.h>

//Load Cell Library
#include <HX711_ADC.h>
 
//Adafruit Graphics & OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Reset pin not used but needed for library
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//Load pins
const int HX711_dout_1 = 4; //mcu > HX711 no 1 dout pin
const int HX711_sck_1 = 5; //mcu > HX711 no 1 sck pin
const int HX711_dout_2 = 6; //mcu > HX711 no 2 dout pin
const int HX711_sck_2 = 7; //mcu > HX711 no 2 sck pin


//HX711 constructor
HX711_ADC LoadCell_1(HX711_dout_1, HX711_sck_1);
HX711_ADC LoadCell_2(HX711_dout_2, HX711_sck_2); //HX711 2

const float calibrationValue_1 = 42069; // calibration value load cell 1
const float calibrationValue_2 = 277680; // calibration value load cell 2

unsigned long t = 0;

//initialize motor pin and PWM value
int mpin = 3;
float mval =  0;

float dutyCycle = 0; 

//initalize potentiometer pin and value
int ppin = A3;
int pval = 0;

//initialize hall sensor pin
int hall = 9;

unsigned long initial = 0;
unsigned long final = 0;

float dt;
float rpm;
bool read_rpm;

void displayRPMandDC(float rpm, float DC){

  //Serial.print(String(rpm) +" ");
  //Serial.print(String(DC));
  
  // Clear the display
  display.clearDisplay();
  //Set the color - always use white despite actual display color
  display.setTextColor(WHITE);
  //Set the font size
  display.setTextSize(1);
  //Set the cursor coordinates
  display.setCursor(0,0); 
  display.print("RPM:         "); 
  display.print(rpm);
  display.setCursor(0,15);
  display.print("Duty Cycle:  "); 
  display.print(DC);
  display.print("%");
}

void setup() {
  // put your setup code here, to run once:

  // Start Wire library for I2C
  Wire.begin();

  // initialize OLED with I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  Serial.begin(115200); //initialize serial by setting serial baud (communication rate)
  
  pinMode(hall, INPUT_PULLUP); //set hall sensor pin as an input with integrated pullup resistor
  initial = millis(); //set initial to time since program start in milliseconds

  //load cell setup
  LoadCell_1.begin();
  LoadCell_2.begin();

  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  byte loadcell_1_rdy = 0;
  byte loadcell_2_rdy = 0;
  while ((loadcell_1_rdy + loadcell_2_rdy) < 2) { //run startup, stabilization and tare, both modules simultaniously
    if (!loadcell_1_rdy) loadcell_1_rdy = LoadCell_1.startMultiple(stabilizingtime, _tare);
    if (!loadcell_2_rdy) loadcell_2_rdy = LoadCell_2.startMultiple(stabilizingtime, _tare);
  }
  if (LoadCell_1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.1 wiring and pin designations");
  }
  if (LoadCell_2.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 no.2 wiring and pin designations");
  }
  LoadCell_1.setCalFactor(calibrationValue_1); // user set calibration value (float)
  LoadCell_2.setCalFactor(calibrationValue_2); // user set calibration value (float)
}

void loop() {
  // put your main code here, to run repeatedly:

  static boolean newDataReady = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell_1.update() || LoadCell_2.update()) newDataReady = true;
  

  //get smoothed value from data set
  if ((newDataReady)) {
    if (millis() > t + serialPrintInterval) {
      float a = LoadCell_1.getData();
      float b = LoadCell_2.getData();
      Serial.print(a);
      Serial.print(",");
      Serial.print(b);
      Serial.println(",");
      
      newDataReady = 0;
      t = millis();
    }
  }

  pval = analogRead(ppin); //read potentiometer value
  mval = map(pval, 0, 1023, 0, 255); //map the potentiometer position to PWM value
  analogWrite(mpin, mval); //write PWM value to speed control mosfet

  dutyCycle = mval/255*100; //calculate duty cycle percentage

  while(!digitalRead(hall)){ // If this loop is entered, we encountered a magnet.
    
    //keep mapping
    pval = analogRead(ppin);
    mval = map(pval, 0, 1023, 0, 255);
    analogWrite(mpin, mval);

    dutyCycle = mval/255*100; //calculate duty cycle percentage
    
    //print to serial monitor for debugging
    //Serial.print("RPM = " + String(rpm)); 
    //Serial.print("potentiometer = " + String(pval) + " ");
    //Serial.println("motor = " + String(mval)+ " ");
     
    //display to OLED
    displayRPMandDC(rpm, dutyCycle);
    display.display();
    
    read_rpm = true;
    final = millis();
  }
  
  if(read_rpm){
     dt = final-initial; //change in time in milliseconds
     initial = final; 
     read_rpm = false;
  }
  
  rpm = 60000/dt; //calculate RPM

  //print to serial monitor for debugging
  //Serial.print("RPM = " + String(rpm) + " "); 
  //Serial.print("potentiometer = " + String(pval) + " ");
  //Serial.println("motor = " + String(mval));

  //display to OLED
  displayRPMandDC(rpm, dutyCycle);
  display.display();
}
