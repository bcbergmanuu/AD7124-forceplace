#include <Arduino.h>
/*
  AD7124 Multiple sensor type example

  Demonstrate how to read multiple sensors of different types using different 
  channel configurations. This example configures the AD7124 for 3 differential
  and 2 single ended channels. It also reads the internal temperature sensor
  and uses it for the cold junction reference temperature. Readings are done
  in single conversion mode.

  Uses the switched 2.5V excitation provided on the NHB AD7124 board, which must
  be turned on before reading the sensor. On NHB boards the on chip low side switch
  is tied to the enable pin of a 2.5V regulator to provide excitation voltage to
  sensors. This allows the regulator to be shut down between readings to save power
  when doing long term, low speed logging.

  
  The channels are configured as follows: 

  Channel 0   Differential    Load Cell (or other full bridge)
  Channel 1   Differential    Thermocouple
  Channel 2   Differential    Thermocouple   
  Channel 3   Single Ended    Potentiometer
  Channel 4   Single Ended    Potentiometer
  Channel 5   Internal        IC Temperature  


  For more on AD7124, see
  http://www.analog.com/media/en/technical-documentation/data-sheets/AD7124-4.pdf



  This file is part of the NHB_AD7124 library.

  MIT License - A copy of the full text should be included with the library

  Copyright (C) 2021  Jaimy Juliano

*/

#include <NHB_AD7124.h>


//#define CH_COUNT 1 // 3 differential channels + 2 single ended + internal temperature sensor

const uint8_t csPin = 10;

Ad7124 adc(csPin, 4000000);


 
// The filter select bits determine the filtering and ouput data rate
//     1 = Minimum filter, Maximum sample rate
//  2047 = Maximum filter, Minumum sample rate
//
// For this example I'll use a setting of 45, which will give us about 20 sps
// with this channel configuration.
//
// Of course you can always take your readings at a SLOWER rate than
// the output data rate. (i.e. logging a reading every  30 seconds)
//
// NOTE: Actual output data rates in single conversion mode will be slower
// than calculated using the formula in the datasheet. This is because of
// the settling time plus the time it takes to enable or change the channel.
//int filterSelBits = 45; //13 ~= 50 sps, 18 ~= 40 sps

void calibrate() {
  adc.setAdcControl(AD7124_OpMode_Idle, AD7124_FullPower, false);
  for(int i =0; i< 7; i++) {

    adc.enableChannel(i, true);
    adc.setAdcControl(AD7124_OpMode_SystemOffsetCalibration, AD7124_FullPower, false);
    delay(100);
    adc.enableChannel(i, false);
  }
  adc.setAdcControl (AD7124_OpMode_Continuous, AD7124_FullPower, false);
}
void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin (500000);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println ("AD7124 Multiple sensor type example");

  // Initializes the AD7124 device
  adc.begin();
  

  
  for(int i =0; i< 7; i++) {
    adc.setup[i].setConfig(AD7124_Ref_Avdd, AD7124_Gain_128, true);
    
    adc.setup[i].setFilter(AD7124_Filter_SINC4, 14);
  }
  
  for(int i =0; i< 7; i++) {
     adc.setChannel(i, i, static_cast<AD7124_InputSel>(i*2), static_cast<AD7124_InputSel>(i*2+1), true);
  }
  
  
  adc.setAdcControl (AD7124_OpMode_Continuous, AD7124_FullPower, false);

}

const double kgfactor = 0.000076399109;

void loop() {

  int readings[7];
  for(int i =0; i < 7; i++) {
    readings[i] = adc.readRaw(i);
  }  
  
  //dt = micros() - dt;
  for(int i =0; i< 7; i++) {
    Serial.print((readings[i]-0x800000)*kgfactor,2);
    if(i < 6) Serial.print('\t');    
  }
  Serial.println();
  if (Serial.available()) {
    int inByte = Serial.read();
    if(inByte == 'c'){
      calibrate();
    }

  }
 
  delay(15);

}