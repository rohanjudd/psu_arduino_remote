// PSU_Remote v0.3
// Rohan Judd - Jan 2018
// RS232 Remote Interface for TTi Bench PSU
// Tested with CPX400SP

// Softserial RX limited to 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

#include <SoftwareSerial.h>

const String READ_ID = "*IDN?";
const String READ_VOLTAGE = "V1O?";
const String READ_CURRENT = "I1O?";
const String SET_VOLTAGE = "V1 ";
const String SET_CURRENT_LIMIT = "I1 ";
const String IS_ON = "OP1?";
const String TURN_ON = "OP1 1";
const String TURN_OFF = "OP1 0";
const String EOL = "\r\n";

const int NUM_LEDS = 6;
const int LED[NUM_LEDS] = {13, 14, 15, 16, 17, 18};
const int NUM_BUTTONS = 6;
const int BUTTON[NUM_BUTTONS] = {2, 3, 4, 5, 6, 7};
const int NUM_PRESETS = 4;
const float PRESET[NUM_PRESETS] = {0, 6.8, 13, 18.2};

const int RESET_DELAY = 800;
const int LOOP_DELAY = 100;

const int lf = 10; // line feed character
const bool verbose = true;
bool button_clear = false;
SoftwareSerial psuSerial(8, 9); // RX, TX
char serialdata[80]; // buffer for incoming serial data

void setup()
{
	for (int x = 0; x < NUM_LEDS; x++) 
		pinMode(LED[x], OUTPUT);
		
	for (int x = 0; x < NUM_BUTTONS; x++) 
		pinMode(BUTTON[x], INPUT_PULLUP);
  
  Serial.begin(57600);
  Serial.println("PSU_Remote v0.3\nRohan Judd Jan 2017\n\nConnecting to PSU");
  all_leds(true);
  delay(200);
  all_leds(false);
  psuSerial.begin(9600);
  delay(100);
  Serial.println(read_id());
  Serial.println("Starting");
  turn_off();
  set_voltage(PRESET[0]);
}

void processSerialInput()
{
  char c = Serial.read();
  switch (c)
  {
    case 'q':
      {
		Serial.println("ouch");
        break;
      }
    default:
      break;
  }
}

void check_buttons()
{
  byte state = get_button_states();
  if (state == 0)
    button_clear = true;
  else if (button_clear)
  {
    button_clear = false;
    switch (state) 
    {
      case 1: //Button 1 pressed
        all_leds(false);
        turn_on();
        break;
      case 2: //Button 2 pressed
       all_leds(false);
        turn_off();
        break;
      case 4: //Button 3 Pressed
        all_leds(false);
        digitalWrite(LED[2], HIGH);
       ramp(2,4,0.02);
        break;
      case 8: //Button 4 Pressed
        all_leds(false);
        digitalWrite(LED[3], HIGH);
       reset_at_voltage(PRESET[1]);
        break;
      case 16: //Button 5 Pressed
        all_leds(false);
        digitalWrite(LED[4], HIGH);
       reset_at_voltage(PRESET[2]);
        break;
      case 32: //Button 6 Pressed
        all_leds(false);
        digitalWrite(LED[5], HIGH);
       reset_at_voltage(PRESET[3]);
        break;
      default:
      // do nothing
       break;
    }
  }
}

byte get_button_states()
{
  // creates byte from button states, with button 0 as LSB.
  byte b = 0;
  for (int x = 0; x < NUM_BUTTONS; x++)  
	  bitWrite(b, x, !digitalRead(BUTTON[x]));
  return b;
}

void all_leds(bool b)
{
  //writes state to all leds
	for (int x = 0; x < NUM_LEDS; x++)
	digitalWrite(LED[x], b);
}

void psu_send(String s)
{
	if (verbose)
	{
		Serial.print("Sending Command to PSU: ");
		Serial.println(s);
	}
	psuSerial.print(s);
	psuSerial.print(EOL);
}
void turn_on()
{
  psu_send(TURN_ON);
  digitalWrite(LED[0], HIGH);
}

void ramp(float a, float b, float i)
{
  if (a < b)
  {
    float v = a;
    while (v < b)
    {
      set_voltage(v);
      delay(25);
      v += i;
    }
    delay(1000);
    while (v >= a)
    {
      set_voltage(v);
      delay(25);
      v -= i;
    }
    
  }
  else
    Serial.println("Ramp Error");
}
void turn_off()
{
  psu_send(TURN_OFF);
  digitalWrite(LED[0], LOW);
}

void set_voltage(float v)
{
	String command = SET_VOLTAGE;
  psu_send(command + v);
}

String read_id()
{
  psu_send(READ_ID);
  delay(200);
  psuSerial.readBytesUntil(lf, serialdata, 80);
  return serialdata;
}

void reset_at_voltage(float v)
{
  turn_off();
  set_voltage(v);
  delay(RESET_DELAY);
  turn_on();
}

void loop() 
{
  if (psuSerial.available()) 
    Serial.write(psuSerial.read());
  if (Serial.available()) 
    psuSerial.write(Serial.read());
  check_buttons();
  delay(LOOP_DELAY);
}
