/* Frequency Probe

   David Johnson-Davies - www.technoblogy.com - 7th December 2020
   ATtiny84 @ 12 MHz (external crystal; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <Wire.h>
#include <avr/sleep.h>

// OLED I2C 128 x 32 monochrome display **********************************************

const int OLEDAddress = 0x3C;

// Initialisation sequence for OLED module
int const InitLen = 15;
const unsigned char Init[InitLen] PROGMEM = {
  0xA8, // Set multiplex
  0x1F, // for 32 rows
  0x8D, // Charge pump
  0x14, 
  0x20, // Memory mode
  0x01, // Vertical addressing
  0xA1, // 0xA0/0xA1 flip horizontally
  0xC8, // 0xC0/0xC8 flip vertically
  0xDA, // Set comp ins
  0x02,
  0xD9, // Set pre charge
  0xF1,
  0xDB, // Set vcom deselect
  0x40,
  0xAF  // Display on
};

const int data = 0x40;
const int single = 0x80;
const int command = 0x00;

void InitDisplay () {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(command);
  for (uint8_t c=0; c<InitLen; c++) Wire.write(pgm_read_byte(&Init[c]));
  Wire.endTransmission();
}

void DisplayOnOff (int On) {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(command);
  Wire.write(0xAE + On);
  Wire.endTransmission();
}

// Graphics **********************************************

int Scale = 1;
boolean Smooth = true;

// Character set for text - stored in program memory
const uint8_t CharMap[96][6] PROGMEM = {
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 
{ 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00 }, 
{ 0x00, 0x07, 0x00, 0x07, 0x00, 0x00 }, 
{ 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00 }, 
{ 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00 }, 
{ 0x23, 0x13, 0x08, 0x64, 0x62, 0x00 }, 
{ 0x36, 0x49, 0x56, 0x20, 0x50, 0x00 }, 
{ 0x00, 0x08, 0x07, 0x03, 0x00, 0x00 }, 
{ 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00 }, 
{ 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00 }, 
{ 0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x00 }, 
{ 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00 }, 
{ 0x00, 0x80, 0x70, 0x30, 0x00, 0x00 }, 
{ 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 }, 
{ 0x00, 0x00, 0x60, 0x60, 0x00, 0x00 }, 
{ 0x20, 0x10, 0x08, 0x04, 0x02, 0x00 }, 
{ 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00 }, 
{ 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00 }, 
{ 0x72, 0x49, 0x49, 0x49, 0x46, 0x00 }, 
{ 0x21, 0x41, 0x49, 0x4D, 0x33, 0x00 }, 
{ 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00 }, 
{ 0x27, 0x45, 0x45, 0x45, 0x39, 0x00 }, 
{ 0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00 }, 
{ 0x41, 0x21, 0x11, 0x09, 0x07, 0x00 }, 
{ 0x36, 0x49, 0x49, 0x49, 0x36, 0x00 }, 
{ 0x46, 0x49, 0x49, 0x29, 0x1E, 0x00 }, 
{ 0x00, 0x00, 0x14, 0x00, 0x00, 0x00 }, 
{ 0x00, 0x40, 0x34, 0x00, 0x00, 0x00 }, 
{ 0x00, 0x08, 0x14, 0x22, 0x41, 0x00 }, 
{ 0x14, 0x14, 0x14, 0x14, 0x14, 0x00 }, 
{ 0x00, 0x41, 0x22, 0x14, 0x08, 0x00 }, 
{ 0x02, 0x01, 0x59, 0x09, 0x06, 0x00 }, 
{ 0x3E, 0x41, 0x5D, 0x59, 0x4E, 0x00 }, 
{ 0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00 }, 
{ 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00 }, 
{ 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00 }, 
{ 0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00 }, 
{ 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00 }, 
{ 0x7F, 0x09, 0x09, 0x09, 0x01, 0x00 }, 
{ 0x3E, 0x41, 0x41, 0x51, 0x73, 0x00 }, 
{ 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00 }, 
{ 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00 }, 
{ 0x20, 0x40, 0x41, 0x3F, 0x01, 0x00 }, 
{ 0x7F, 0x08, 0x14, 0x22, 0x41, 0x00 }, 
{ 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00 }, 
{ 0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x00 }, 
{ 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00 }, 
{ 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00 }, 
{ 0x7F, 0x09, 0x09, 0x09, 0x06, 0x00 }, 
{ 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00 }, 
{ 0x7F, 0x09, 0x19, 0x29, 0x46, 0x00 }, 
{ 0x26, 0x49, 0x49, 0x49, 0x32, 0x00 }, 
{ 0x03, 0x01, 0x7F, 0x01, 0x03, 0x00 }, 
{ 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00 }, 
{ 0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00 }, 
{ 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00 }, 
{ 0x63, 0x14, 0x08, 0x14, 0x63, 0x00 }, 
{ 0x03, 0x04, 0x78, 0x04, 0x03, 0x00 }, 
{ 0x61, 0x59, 0x49, 0x4D, 0x43, 0x00 }, 
{ 0x00, 0x7F, 0x41, 0x41, 0x41, 0x00 }, 
{ 0x02, 0x04, 0x08, 0x10, 0x20, 0x00 }, 
{ 0x00, 0x41, 0x41, 0x41, 0x7F, 0x00 }, 
{ 0x04, 0x02, 0x01, 0x02, 0x04, 0x00 }, 
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x00 }, 
{ 0x00, 0x03, 0x07, 0x08, 0x00, 0x00 }, 
{ 0x20, 0x54, 0x54, 0x78, 0x40, 0x00 }, 
{ 0x7F, 0x28, 0x44, 0x44, 0x38, 0x00 }, 
{ 0x38, 0x44, 0x44, 0x44, 0x28, 0x00 }, 
{ 0x38, 0x44, 0x44, 0x28, 0x7F, 0x00 }, 
{ 0x38, 0x54, 0x54, 0x54, 0x18, 0x00 }, 
{ 0x00, 0x08, 0x7E, 0x09, 0x02, 0x00 }, 
{ 0x18, 0xA4, 0xA4, 0x9C, 0x78, 0x00 }, 
{ 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00 }, 
{ 0x00, 0x44, 0x7D, 0x40, 0x00, 0x00 }, 
{ 0x20, 0x40, 0x40, 0x3D, 0x00, 0x00 }, 
{ 0x7F, 0x10, 0x28, 0x44, 0x00, 0x00 }, 
{ 0x00, 0x41, 0x7F, 0x40, 0x00, 0x00 }, 
{ 0x7C, 0x04, 0x78, 0x04, 0x78, 0x00 }, 
{ 0x7C, 0x08, 0x04, 0x04, 0x78, 0x00 }, 
{ 0x38, 0x44, 0x44, 0x44, 0x38, 0x00 }, 
{ 0xFC, 0x18, 0x24, 0x24, 0x18, 0x00 }, 
{ 0x18, 0x24, 0x24, 0x18, 0xFC, 0x00 }, 
{ 0x7C, 0x08, 0x04, 0x04, 0x08, 0x00 }, 
{ 0x48, 0x54, 0x54, 0x54, 0x24, 0x00 }, 
{ 0x04, 0x04, 0x3F, 0x44, 0x24, 0x00 }, 
{ 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00 }, 
{ 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00 }, 
{ 0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00 }, 
{ 0x44, 0x28, 0x10, 0x28, 0x44, 0x00 }, 
{ 0x4C, 0x90, 0x90, 0x90, 0x7C, 0x00 }, 
{ 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00 }, 
{ 0x00, 0x08, 0x36, 0x41, 0x00, 0x00 }, 
{ 0x00, 0x00, 0x77, 0x00, 0x00, 0x00 }, 
{ 0x00, 0x41, 0x36, 0x08, 0x00, 0x00 }, 
{ 0x02, 0x01, 0x02, 0x04, 0x02, 0x00 }, 
{ 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0 }

};

void ClearDisplay () {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(command);
  // Set column address range
  Wire.write(0x21); Wire.write(0); Wire.write(127);
  // Set page address range
  Wire.write(0x22); Wire.write(0); Wire.write(3);
  Wire.endTransmission();
  // Write the data in 26 20-byte transmissions
  for (int i = 0 ; i < 26; i++) {
    Wire.beginTransmission(OLEDAddress);
    Wire.write(data);
    for (int i = 0 ; i < 20; i++) Wire.write(0);
    Wire.endTransmission();
  }
}

// Converts bit pattern abcdefgh into aabbccddeeffgghh
int Stretch (int x) {
  x = (x & 0xF0)<<4 | (x & 0x0F);
  x = (x<<2 | x) & 0x3333;
  x = (x<<1 | x) & 0x5555;
  return x | x<<1;
}

// Plots a character; line = 0 to 3; column = 0 to 131
void PlotChar(int c, int line, int column) {
  Wire.beginTransmission(OLEDAddress);
  Wire.write(command);
  // Set column address range
  Wire.write(0x21); Wire.write(column); Wire.write(column + Scale*6 - 1);
  // Set page address range
  Wire.write(0x22); Wire.write(line); Wire.write(line + Scale - 1);
  Wire.endTransmission();
  Wire.beginTransmission(OLEDAddress);
  Wire.write(data);
  uint8_t col0 = pgm_read_byte(&CharMap[c-32][0]);
  int col0L, col0R, col1L, col1R;
  col0L = Stretch(col0);
  col0R = col0L;
  for (uint8_t col = 1 ; col < 5; col++) {
    uint8_t col1 = pgm_read_byte(&CharMap[c-32][col]);
    col1L = Stretch(col1);
    col1R = col1L;
    if (Scale == 1) Wire.write(col0);
    // Smoothing
    else {
      if (Smooth) {
        for (int i=6; i>=0; i--) {
          for (int j=1; j<3; j++) {
            if (((col0>>i & 0b11) == (3-j)) && ((col1>>i & 0b11) == j)) {
              col0R = col0R | 1<<((i*2)+j);
              col1L = col1L | 1<<((i*2)+3-j);
            }
          }
        }
      }
      Wire.write(col0L); Wire.write(col0L>>8); 
      Wire.write(col0R); Wire.write(col0R>>8);
      col0L = col1L; col0R = col1R;
    }
    col0 = col1;
  }
  if (Scale == 1) Wire.write(col0);
  else {
    Wire.write(col0L); Wire.write(col0L>>8); 
    Wire.write(col0R); Wire.write(col0R>>8); 
  } 
  Wire.endTransmission();
}

// Plot text starting at the current plot position
void PlotText(PGM_P s, int line, int column) {
  int p = (int)s;
  while (1) {
    char c = pgm_read_byte(p++);
    if (c == 0) return;
    PlotChar(c, line, column);
    column = column + Scale*6;
  }
}

// Display a fixed-point value starting at line, column
void PlotValue (unsigned long freq, int line, int column, int places) {
  bool suppress = true;
  int divisor = 10000;
  for (unsigned int d=5; d>0; d--) {
    if (d == places) { PlotChar('.', line, column); column = column + Scale*6; }
    char c = (freq/divisor) % 10 +'0';
    if (c == '0' && suppress && d>places+1) c = ' '; else suppress = false;
    PlotChar(c, line, column);
    column = column + Scale*6;
    divisor = divisor / 10;
  }
}

// Display frequencies **********************************************

const int Col = 0;

void PlotFrequency (unsigned long c, int line) {
  unsigned long f = (c * 46875)/512;
  if (f >= 1000000) {
    PlotValue(f/100, line, Col, 4);
    PlotText(PSTR(" MHz"), line, Col+6*6*Scale);
  } else if (f >= 100000) {
    PlotValue(f/10, line, Col, 2);
    PlotText(PSTR(" kHz"), line, Col+6*6*Scale);
  } else if (f >= 10000) {
    PlotValue(f, line, Col, 3);
    PlotText(PSTR(" kHz"), line, Col+6*6*Scale);
  } else PlotText(PSTR("----------"), line, Col);
}

void PlotInterval (unsigned long c, int line) {
  if (c > 0 && c < 120) {
    PlotValue((unsigned long)1200000 / c, line, Col, 2);
    PlotText(PSTR(" kHz"), line, Col+6*6*Scale);
  } else if (c < 1200) {
    PlotValue((unsigned long)12000000 / c, line, Col, 3);
    PlotText(PSTR(" kHz"), line, Col+6*6*Scale);
  } else if (c < 12000) {
    PlotValue((unsigned long)120000000 / c, line, Col, 4);
    PlotText(PSTR(" kHz"), line, Col+6*6*Scale);
  } else if (c < 120000) {
    PlotValue((unsigned long)1200000000 / c, line, Col, 2);
    PlotText(PSTR(" Hz "), line, Col+6*6*Scale);
  } else if (c < 1200000) {
    PlotValue((unsigned long)1200000000 / (c/10), line, Col, 3);
    PlotText(PSTR(" Hz "), line, Col+6*6*Scale);
  } else {
    PlotValue((unsigned long)1200000000 / (c/100), line, Col, 4);
    PlotText(PSTR(" Hz "), line, Col+6*6*Scale);
  }
}

void PlotVoltage (unsigned long c, int line) {
  unsigned long v = c * 125 / 256;
  PlotValue(v, line, Col, 2);
  PlotText(PSTR(" V  "), line, Col+6*6*Scale);
}


// Interrupt routines **********************************************

volatile unsigned int FreqHigh, IntHigh, Timer;
volatile unsigned long Count0, Count1, Count;
volatile boolean Capture = false;
volatile uint8_t Captures = 0;

// Timer/Counter0 is used in frequency mode
ISR(TIM0_COMPA_vect) {
  FreqHigh++;
}

// Timer/Counter1 compare interrupt is used in frequency mode
ISR(TIM1_COMPA_vect) {
  uint8_t temp = TCNT0;
  TIMSK1 = 0;                             // Turn off interrupt
  TIMSK0 = 0;                             // Turn off interrupt
  Count = (unsigned long)FreqHigh<<8 | temp;
  Capture = true;
}

// Timer/Counter1 overflow interrupt is used in interval mode
ISR(TIM1_OVF_vect) {
  IntHigh++;
}

// Timer/Counter1 input capture interrupt is used in interval mode
ISR(TIM1_CAPT_vect) {
  unsigned int temp = ICR1;
  Count0 = Count1;
  Count1 = (unsigned long)IntHigh<<16 | temp;
  Captures++;
  if (Captures == 2) {
    Count = Count1 - Count0;
    TIMSK1 = 0;
  }
}

// Watchdog interrupt gives a 1-second delay
ISR(WDT_vect) {
  WDTCSR = WDTCSR | 1<<WDIE;
  Timer--;
}

// Delay for one second using watchdog timer
void DelaySecond () {
  WDTCSR = WDTCSR | 1<<WDIE;
  Timer = 8;
  while (Timer > 0);
  WDTCSR = WDTCSR & ~(0<<WDIE);
}

// Main measurement routines

unsigned long GetFrequency () {
  Capture = false;
  FreqHigh = 0;
  TCNT0 = 0;
  TCNT1 = 0;
  OCR1A = 16383;                          // Divide by 16384
  TIFR1 = 1<<OCF1A;                       // Clear compare flag
  TIMSK0 = 1<<OCIE0A;                     // Timer/Counter0 COMPA interrupt
  TIMSK1 = 1<<OCIE1A;                     // Timer/Counter1 COMPA interrupt
  GTCCR = 1;                              // Clear prescaler
  TCCR1B = TCCR1B | 2<<CS10;              // Start clock, /8
  TCCR0B = TCCR0B | 7<<CS00;              // Start external clock on T0 rising edge
  while (!Capture);                       // Wait for Timer/Counter1 timeout
  TCCR1B = TCCR1B & ~(2<<CS10);           // Stop clock
  TCCR0B = TCCR0B & ~(7<<CS00);           // Stop clock
  return Count;
}

unsigned long GetInterval () {
  Captures = 0;
  Count = 0;
  IntHigh = 0;
  TCNT1 = 0;
  OCR1A = 65535;                          // Divide by 65536
  TCCR1B = TCCR1B | 1<<CS10;              // Start clock, /1
  TIFR1 = 1<<ICF1 | 1<<TOV1;              // Clear capture and OVF flags
  TIMSK1 = 1<<TOIE1 | 1<<ICIE1;           // Timer/Counter1 OVF + capture interrupts
  DelaySecond();                          // Allow 1 second for captures
  TCCR1B = TCCR1B & ~(1<<CS10);           // Stop clock
  TIMSK1 = 0;                             // Turn off interrupts
  return Count;
}

unsigned long GetVoltage () { 
  ADCSRA = ADCSRA | 1<<ADSC;              // Start conversion
  while(ADCSRA & 1<<ADSC);                // Wait until complete
  int low, high;
  low = ADCL;
  high = ADCH;
  return high<<8 | low;
}

// Setup **********************************************

enum modeType { FREQUENCY, INTERVAL, VOLTAGE};

void setup() {
  Wire.begin();
  InitDisplay();
  ClearDisplay();
  //
  // Set up 8-bit Timer/Counter0
  TCCR0A = 0<<WGM00;                      // Normal mode, no waveform output
  TCCR0B = 0<<WGM02 | 0<<CS00;            // Clock stopped
  OCR0A = 255;                            // Compare match at top
  TIMSK0 = 0;                             // Interrupts initially off
  //
  // Set up 16-bit Timer/Counter1
  TCCR1A = 0<<WGM10;                      // No waveform output
  TCCR1B = 1<<WGM12 | 1<<ICES1 | 0<<CS10; // CTC mode, rising edge, clock stopped
  TIMSK1 = 0;                             // Interrupts initially off
  //
  // Set up Watchdog timer for 1 Hz interrupt for display update
  WDTCSR = 0<<WDIE | 3<<WDP0;             // 1/8 second interrupt
  //
  // Set up ADC to use ADC1 (PA1)
  ADMUX = 2<<REFS0 | 1<<MUX0;             // ADC1, 1.1V ref
  ADCSRA = 1<<ADEN | 0<<ADIE | 7<<ADPS0;  // enable ADC, 93.75kHz ADC clock
}

const int Timeout = 30; // Seconds

void loop() {
  unsigned long c;
  modeType mode = FREQUENCY;
  Scale = 2;
  PlotText(PSTR("Freq Probe"), 1, 0);
  int Sleep = Timeout;
  do {
    if (mode == FREQUENCY) {
      c = GetFrequency();
      if (c > 320) {
        PlotFrequency(c, 1);
        Sleep = Timeout;
        DelaySecond();
      } else if (c == 0) mode = VOLTAGE;
      else mode = INTERVAL;
    } else if (mode == INTERVAL) {
      c = GetInterval();
      if (c > 390) {
        PlotInterval(c, 1);
        Sleep = Timeout;
      } else if (c == 0) mode = VOLTAGE;
      else mode = FREQUENCY;
    } else if (mode == VOLTAGE) {
      c = GetVoltage();
      PlotVoltage(c, 1);
      if (c < 40) Sleep--;
      DelaySecond();
      mode = FREQUENCY;
    }
  } while (Sleep > 0);
  // Go to sleep
  TIMSK0 = 0;
  TIMSK1 = 0;
  WDTCSR = 0;                             // Make sure there are no interrupts
  DisplayOnOff(0);                        // Display off
  ADCSRA = ADCSRA & ~(1<<ADEN);           // Disable ADC to save power
  DDRA = 0;                               // All I/O lines as inputs
  DDRB = 0;
  PORTA = 0x25;                           // Pullups on PA0, PA2, and PA5
  PORTB = 0x07;                           // Pullups on PB0, PB1, and PB2
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();                            // Go to sleep until reset
}
