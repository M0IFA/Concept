// My_VFO_KB is a keyboard input/screen display VFO
// Starter kit VFO using Si5351 module, freq in cHz
// Si5351 I2C bus 
// SDA = A4 
// SCL = A5

// I2C and Si5351 Libraries
#include "Wire.h"
#include "si5351.h"

// create dds object
Si5351 dds;

// start frequency (cHz)
uint32_t freq = 700000000; // 7MHz
uint32_t prevFreq = freq;

// setup runs once on upload
void setup(){
  // start serial (monitor "NEWLINE" & 9600 baud)
  Serial.begin(9600);
  
  // init dds si5351 module, "0" = default 25MHz XTAL
  dds.init(SI5351_CRYSTAL_LOAD_8PF, 0);

  // set 8mA output drive
  dds.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);

  // enable VFO output CLK0, disable CLK1 & 2
  dds.output_enable(SI5351_CLK0, 1);
  dds.output_enable(SI5351_CLK1, 0);
  dds.output_enable(SI5351_CLK2, 0);
  
  freqOut(freq); // output freq
  dispFreq(freq); // display freq in Hz
}

void loop(){ 
   freq = getIn(); // get input freq cHz
   
  // new freq?
  if(freq != prevFreq) 
  {
    freqOut(freq); // output freq
    dispFreq(freq); // display in Hz
    prevFreq = freq; // remember as previous freq
  } 
}

// freq output in cHz on CLK0
void freqOut(uint32_t freq){
    dds.set_freq(freq, 0, SI5351_CLK0); // cHz
}

// get input Hz, return cHz
uint32_t getIn(){
  uint32_t in;
  
  while(Serial.available() > 0) // flush buffer
    Serial.read();
    
  while(Serial.available() == 0) // wait for input
    in = Serial.parseInt(); // read input in Hz and parse to integer
   
  return in * 100UL; // return in cHz
}

// display frequency on monitor
void dispFreq(uint32_t f){
  // display freq
  Serial.print("My_VFO = ");
  Serial.print((float)f / 100, 0); // convert to float & display in Hz
  Serial.println(" Hz");
}
