// ¡¡¡ VERY INTERESETING DOC !!!
// https://github.com/FastLED/FastLED/wiki/Multiple-Controller-Examples


// ~ · HERE BELOW THERE ARE SOME LINES TO CHANGE BRIGTHNES LED BY LED · ~
// CRGB leds[NUM_STRIPS][LEDS_PER_STRIP];
// void setup() {
//   FastLED.addLeds<NEOPIXEL, 2>(leds[0], LEDS_PER_STRIP);
//   FastLED.addLeds<NEOPIXEL, 3>(leds[1], LEDS_PER_STRIP);
//   FastLED.addLeds<NEOPIXEL, 4>(leds[2], LEDS_PER_STRIP);
// }
// void loop() {
//   // Set brightness for the first strip to 50
//   setStripBrightness(0, 50);
//   // Set brightness for the second strip to 100
//   setStripBrightness(1, 100);
//   // Set brightness for the third strip to 255 (full brightness)
//   setStripBrightness(2, 255);
//   // Your animation code goes here
//   // ...
//   // Show the changes
//   FastLED.show();
// }
// void setStripBrightness(int stripIndex, uint8_t brightness) {
//   for (int i = 0; i < LEDS_PER_STRIP; i++) {
//     leds[stripIndex][i].fadeLightBy(brightness);
//   }
// }



#include <FastLED.h>
#include <arduino-timer.h>
#include <BluetoothSerial.h>

// Initiliaze main variables.
bool flag = true; // Boolean for the switch from manual to BT mode, and vice versa.
bool last_flag = flag; // Auxiliar boolean for the switch from manual to BT mode.
int traffic_light[3][3] = {{255, 0, 0}, {255, 255, 0}, {0, 255, 0}};

// Constant variables.
#define pb_pin      18  // Push button pin.
#define flag_pin    15  // Switch (manual - BT) pin.
#define PIN0        16
#define PIN1         4
#define STRIP_LEN0   5
#define STRIP_LEN1   4
#define N            2
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds0[STRIP_LEN0];
CRGB leds1[STRIP_LEN1];

// Variables related to the Serial/BT communication.
#define COMMAND_LENGTH 100
char command[COMMAND_LENGTH];
char messageTemp[COMMAND_LENGTH];
const char delimiter[2] = ";";
char* token;
int sIt;
char c;
int numel;
int ref;
int tokens_counter = 0;
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial BT;
uint8_t data[15] = "Big:5;Small:4#";
uint8_t ok[4] = "OK#";

// Variables related to the timer interruptions.
volatile unsigned int t2_counter = 0;
bool state = false;
float desired_time_s = 1;
float overflow_time = 255/7812.5; // Resolution (8 bit => 255) / Frequence (set by TCCR2B register)
float threshold = desired_time_s / overflow_time;

// Variables to control the duration of the user's clicks.
uint32_t tLocalTimeMs = 0;  // Timestamp for each iteration [ms].
uint32_t tLastPush = 0; // Timestamp when the user starts clicking the pushbutton [ms].
uint32_t tFinalPressed = 0; // Timestamp when the user stops clicking the pushbutton [ms].
uint32_t tPressed = 0; // Time the pushbutton has been pressed [ms].
uint8_t clicks_num = 0; // Clicks' counter.
uint8_t pb_Pushed = false; // Flag that indicates the pushbutton has been pressed.
int mode = 0;
int num_mode = 0;
int brght_mode = 0;
int freq_mode = 0;

// Initiliaze relative variables to control all the strips.
const int lengths[N] = {STRIP_LEN0, STRIP_LEN1};
const int max_len = max(STRIP_LEN0, STRIP_LEN1);
int num = max_len;
int last_num = num;
int freq = 100; // It should be greater than 10, because there is a delay in the main loop that interferes with the evaluation of the pushbutton clicks duration. [0, 100] ~ %
float minFreq = 15;
float maxFreq = 2000;
int brght = 50; // If it is lower than 10, the colors resolution is reduced. [0, 100] ~ %
int R = 200;
int G = 200;
int B = 200;

// Initiliaze specific variables for each strip.
int nums[N];
int og_nums[N];
int last_nums[N];
int brghts[N]; // If it is lower than 10, the colors resolution is reduced.
int palettes[N];
int Rs[N];
int Gs[N];
int Bs[N];

// Initiliaze variables to manage the palettes.
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

// This function fills the palette with totally random colors.
CRGBPalette16 SetupTotallyRandomPalette() {
  for(int i = 0; i < 16; ++i) {
      currentPalette[i] = CRGB(random(50, 250), random(50, 250), random(50, 250));
  }
  return currentPalette;
}

const CRGBPalette16 MyPartyColors_p = { CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Blue,
                                        CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Blue,
                                        CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Blue,
                                        CRGB::Yellow, CRGB::Green, CRGB::Red, CRGB::Blue };

const CRGBPalette16 FlamingoColors_p = { CRGB(170, 0, 170), CRGB(170, 0, 170), CRGB(200, 70, 100), CRGB(200, 70, 100),
                                         CRGB(200, 70, 100), CRGB(200, 70, 100), CRGB(170, 0, 170), CRGB(170, 0, 170),
                                         CRGB(200, 50, 155), CRGB(200, 50, 155), CRGB(200, 50, 155), CRGB(200, 50, 155),
                                         CRGB(200, 50, 50), CRGB(200, 50, 50), CRGB(200, 70, 100), CRGB(200, 70, 100) };

const CRGBPalette16 PoliceColors_p = { CRGB::Black, CRGB::Blue, CRGB::Gray, CRGB::Red,
                                       CRGB::Black, CRGB::Blue, CRGB::Gray, CRGB::Red,
                                       CRGB::Black, CRGB::Blue, CRGB::Gray, CRGB::Red,
                                       CRGB::Black, CRGB::Blue, CRGB::Gray, CRGB::Red };

#define N_palettes 7
CRGBPalette16 currentPalettes[N_palettes] = {RainbowColors_p, MyPartyColors_p, OceanColors_p, ForestColors_p, LavaColors_p, FlamingoColors_p, PoliceColors_p};

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    Serial.print(".");
  }
  Serial.println("Serial COM initialized.");
  BT.begin("JS testing testing");
  while (!BT) {
    Serial.print(".");
  }
  Serial.println("Bluetooth initialized.");

  // Initialize hardware pins.
  pinMode(pb_pin, INPUT_PULLUP);
  pinMode(flag_pin, INPUT_PULLUP);
  
  // Define the leds stripe.
  FastLED.addLeds<LED_TYPE, PIN0, COLOR_ORDER>(leds0, STRIP_LEN0).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN1, COLOR_ORDER>(leds1, STRIP_LEN1).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brght);
  currentPalette = RainbowColors_p;
  currentBlending = NOBLEND;

  // Define default values for each strip.
  for (int i=0; i<N; i++) {
    nums[i] = max_len;
    last_nums[i] = nums[i];
    brghts[i] = brght;
    Rs[i] = R;
    Gs[i] = G;
    Bs[i] = B;
    palettes[i] = 0;
  }
  
  // Finish setup feedback.
  all_off();
  delay(100);
  for (int i=0; i<3; i++) {
    leds0[i] = CRGB(traffic_light[i][0], traffic_light[i][1], traffic_light[i][2]);
    leds1[i] = CRGB(traffic_light[i][0], traffic_light[i][1], traffic_light[i][2]);
    FastLED.show();
    delay(333);
  }
  Serial.println("Setup end.");

  // // Initialize the palettes after 'all_off' method, since it change it purposely.
  // for (int i=0; i<N; i++) {
  //   palettes[i] = 0;
  // }
}

void loop() {
  // Check the switch status and make feedback when it is toggled.
  flag = digitalRead(flag_pin);
  if (last_flag != flag) {
    last_flag = flag;
    Serial.println("Toggled mode.");
  }

  if (flag) {
    if (BT.available()) { // BT mode.
      serialEvent();
    }
  }
  else { // Manual mode.
    // Push-button checking.
    tLocalTimeMs = millis();
    if (!digitalRead(pb_pin)) { // Pushed.
      // Save the first instant pushing.
      if (!pb_Pushed && tLocalTimeMs > tLastPush + 70) { // Let 70 ms from the last push to avoid the uncontrolled pulses of the pushbutton.
        tLastPush = tLocalTimeMs;
        pb_Pushed = true;
        Serial.println("PUSH");
      }
    }
    else { // Not pushed.
      // Check how many time have elapsed from the first instant pushing.
      if (pb_Pushed && tLocalTimeMs > tLastPush + 70) { // Leaving this time allowed (70 ms) seems that the program avoid more noise errors.
        pb_Pushed = false;
        tPressed = tLocalTimeMs - tLastPush;
        tFinalPressed = tLastPush + tPressed;
        clicks_num += 1;
      }
    
      if (tLocalTimeMs > tFinalPressed + 500 && clicks_num == 1) { // Check if it is a single click waiting for 500 ms.
        clicks_num = 0;
    
        if (tPressed > 70 && tPressed < 800) { // Check if it is a short click.
          mode += 1;
          
          if (mode > 3) {
            mode = 0;
          }
          
          switch (mode) {
            case 0:
              currentPalette = RainbowColors_p;
              currentBlending = NOBLEND;
              break;
            case 1:
              currentPalette = CloudColors_p; 
              currentBlending = NOBLEND;
              break;
            case 2:
              currentBlending = LINEARBLEND;
              break;
            case 3:
              currentPalette = PartyColors_p;
              currentBlending = NOBLEND;
              break;
            default:
              // statements
              break;
          }
          Serial.println("Short click");
        }
        else if (tPressed >= 800 && tPressed <= 3000) {  // Check if it is a long click.
          num_mode += 1;
          
          if (num_mode > 3) {
            num_mode = 0;
          }
          
          switch (num_mode) {
            case 0:
              nums[0] = STRIP_LEN0/(STRIP_LEN0/2);
              break;
            case 1:
              nums[0] = STRIP_LEN1/10;
              break;
            case 2:
              nums[0] = STRIP_LEN1/2;
              break;
            case 3:
              nums[0] = STRIP_LEN1;
              break;
          }

          Serial.println("Long click");
        }
      }
      else if (tPressed > 70 && tPressed < 800 && clicks_num == 2){ // Check if it is a double click of short clicks.
        clicks_num = 0;
        freq_mode += 1;
        
        if (freq_mode > 3) {
          freq_mode = 0;
        }
        
        switch (freq_mode) {
          case 0:
            freq = 10;
            break;
          case 1:
            freq = 100;
            break;
          case 2:
            freq = 500;
            break;
          case 3:
            freq = 2000;
            break;
          default:
            // statements
            break;
        }
        Serial.println("Double short click");
      }
      else if (tPressed > 70 && tPressed >= 800 && clicks_num == 2){ // Check if it is a double click of short click + long click.
        nums[0] = 10;
        clicks_num = 0;
        brght_mode += 1;
        
        if (brght_mode > 3) {
          brght_mode = 0;
        }
        
        switch (brght_mode) {
          case 0:
            brght = 2;
            break;
          case 1:
            brght = 10;
            break;
          case 2:
            brght = 50;
            break;
          case 3:
            brght = 100;
            break;
          default:
            // statements
            break;
        }
        
        Serial.println("Double long click");
      }
      else if (clicks_num > 2) { // Restart the clicks' counter if no of the last cases has happened.
        clicks_num = 0;
      }
    }
  }

  // Copied framework to set the corresponding palettes for each strip (only changing the correpsonding strip index).
  // 0
  static uint8_t startIndex0 = 0;
  startIndex0 += 1; /* motion speed */
  set_palette(palettes[0]); // Update currentPalette.
  if (palettes[0] < N_palettes+1) {
    FillLEDsFromPaletteColors0(startIndex0, brghts[0]);
  }
  if (nums[0] != last_nums[0]) {
    numChanged0();
    last_nums[0] = nums[0];
  }
  FastLED.show();

  // 1
  static uint8_t startIndex1 = 0;
  startIndex1 += 1; /* motion speed */
  set_palette(palettes[1]); // Update currentPalette.
  if (palettes[1] < N_palettes+1) {
    FillLEDsFromPaletteColors1(startIndex1, brghts[1]);
  }
  if (nums[1] != last_nums[1]) {
    numChanged1();
    last_nums[1] = nums[1];
  }
  FastLED.show();

  FastLED.delay(1000 / freq);
}

//ISR(TIMER2_OVF_vect) { // Timer 2 overflow interruption function.
//  t2_counter++;
//  if (t2_counter > threshold) {
//    digitalWrite(8, state);
//    state = !state;
//    t2_counter = 0;
//  }
//}

void set_palette(int i) { // Update currentPalette.
  if (i < N_palettes) {
    currentPalette = currentPalettes[i];
  }
  else {
    currentPalette = SetupTotallyRandomPalette();
  }
}

void numChanged0() { // Function to turn off the left over leds1 when the number is set to a lower one.
  fill_solid(leds0, STRIP_LEN0, CRGB::Black);
//  for (int i=0; i<STRIP_LEN1; i++) {
//      leds1[i] = CRGB(0, 0, 0);
//  }
}

void numChanged1() { // Function to turn off the left over leds1 when the number is set to a lower one.
  fill_solid(leds1, STRIP_LEN1, CRGB::Black);
}

void FillLEDsFromPaletteColors0(uint8_t colorIndex, uint8_t brightness) { // Function to set the proper colors to the leds.
  for(int i = 0; i < nums[0]; ++i) {
      leds0[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
      colorIndex += 5;
  }
}

void FillLEDsFromPaletteColors1(uint8_t colorIndex, uint8_t brightness) { // Function to set the proper colors to the leds.
  for(int i = 0; i < nums[1]; ++i) {
      leds1[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
      colorIndex += 51;
  }
}

void all_off() {  // Turn off all leds.
  for (int i=0; i<max_len; i++) {
    leds0[i] = CRGB(0, 0, 0);
    leds1[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  for (int i=0; i<N; i++) {
    palettes[i] = 99;
  }
}
