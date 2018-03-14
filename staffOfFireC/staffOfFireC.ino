#include <Adafruit_NeoPixel.h>
#define NUMPIXELS 7 // Number of LEDs in strip
//#include <avr/power.h> // Comment out this line for non-AVR boards (Arduino Due, etc.)

#define PIN 6
const int button=3; //pin for button

// Trigger Parameters
double long blist=0; //tap blacklist.
const double long blistdur=300; //tap blacklist duration, debounces.
double long wlist=0; //tap whitelist
const double long wlistdur=1000; //tap whitelist duration, for 2nd tap detection
int presses=0; //how many taps detected
double long nowtime=0; //millis at start of cycle, to maintain sync
bool trigger=0; //trigger flag

//flame Parameters
int flameOR = 0; //flame temperature override value
const int flamestep=3; //size of a step
const double long updelay=5; //delay between flame updates
double long nextud=0; //last update tracker
const int dthreshL=0; //dim lower threshold
const int dthreshH=699; //dim upper threshold
const int flashthreshL=700; //flash lower threshold
const int flashthreshH=1000; //flash upper threshold
int curCol[]={0,255,0}; //return array for HSV2RGB
int fdir[]= {1,1,1,1,1,1,1}; //flame travel direction.
int basecol[]= {255,255,255,255,255,255,255};//individual flame base colour
int globalcol=255; //global colour selector
int indheat[]={0,0,0,0,0,0,0}; //individual flame temperature

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(button, INPUT_PULLUP);
  strip.begin();
  strip.setPixelColor(0,0,255,0);
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(0));
//  Serial.begin(9600);
}


void loop() {
  nowtime=millis();
  buttonDetection();
  flames();
}

void flames() {
  if (trigger) {
    flameOR=1000; //override flame heat values
    trigger=0; //reset trigger to off
    globalcol=random(0,768); //choose new flame base colour
    for (int i=0;i<7;i++) {
      basecol[i]=globalcol;
      indheat[i]=random(25,500);
      if (random(0,100)<50) {
        fdir[i]=1;
      } else {
        fdir[i]=-1;
      }
    }
  }

  if (nowtime>nextud) {
    bool keepresult = 1; //whether to keep the following calcs
    int finalheat=0; //heat value to actually use, calculated later
    nextud=nextud+updelay; //generate next update time

    for (int i=0;i<7;i++) {
      int h1=basecol[i]; //hsv holders
      int s1=0;
      int b1=255;
      if (flameOR>indheat[i]) { //check for flame heat override
        keepresult=0; //don't keep these values in indheat[]
        finalheat=flameOR;
      } else {
        finalheat=indheat[i];
      }
      if (finalheat<=dthreshH) {//dimming detection
        b1=map(finalheat, dthreshL, dthreshH, 0,255);
        
      }
      if (finalheat>=flashthreshL){
        s1=map(finalheat, flashthreshL, flashthreshH, 0,255);
      }
      h1=h1+map(indheat[i], 0, flashthreshH,0,128);
      if (h1>767) {
        h1=h1-767;
      }
      if (h1<0) {
        h1=h1+767;
      }
      if (s1>255) {
        s1=255;
      }
      if (s1<0) {
        s1=0;
      }
      if (b1>255) {
        b1=255;
      }
      if (b1<0) {
        b1=0;
      }
      //convert HSV 2 RGB and send to LEDs
      
      hsb2rgb(h1, 255-s1, b1);
      strip.setPixelColor(i, curCol[0], curCol[1],curCol[2]);      
      finalheat=finalheat+(fdir[i]*flamestep);
      if (random(0,200)==70) {
        fdir[i]=fdir[i]*(-1);
      }
      if (finalheat<0) { //check heat isn't below 0
        finalheat=0;     //and clamp & reverse if so
        fdir[i]=1;
      }
      if (finalheat>flashthreshH) { //check heat isn't above max
        finalheat=flashthreshH;     // and clamp & reverse if so
        fdir[i]=-1;
      }
      if (keepresult) {
        indheat[i]=finalheat; //if not overridden heat, save calc result
      }
      if (indheat[i]>flashthreshL) {
        if (random(0,100)<10) {
          fdir[i]=-1;
        }
      }
      

      }
      flameOR=flameOR-3;
      if (flameOR<0) {
        flameOR=0;
    //end of per flame loop
    }


    strip.show();
    //end of timed update loop
  }

  



// demo stuff,testing button  
//    int r=random(0,256);
//    int g=random(0,256);
//    int b=random(0,256);
//    for (int i=0;i<NUMPIXELS;i++) {
//      strip.setPixelColor(i,r,g,b);
//    }
//    strip.show();
    
}

void buttonDetection() {
  bool bpress=0;
  if (digitalRead(button)==LOW) {
    bpress=1;
  }
  if ((nowtime>blist) && bpress) {
    
    if (nowtime<wlist) {
      presses=2;
      wlist=nowtime;
    } else {
      presses=1;
      wlist=nowtime+wlistdur;
    }
    blist=nowtime+blistdur;
    
    //Serial.print("tap");
    
  }
  
  if ((presses==2)) { //double tap detected
    trigger=1;
    //Serial.println(presses);
    presses=0;
  }
  
  if (nowtime>wlist) {
    presses=0; //if no second tap occurs in time, set presses to 0
  }
}
/******************************************************************************
 * accepts hue, saturation and brightness values and outputs three 8-bit color
 * values in an array (color[])
 *
 * saturation (sat) and brightness (bright) are 8-bit values.
 *
 * hue (index) is a value between 0 and 767. hue values out of range are
 * rendered as 0.
 *
 *****************************************************************************/
void hsb2rgb(uint16_t index, uint8_t sat, uint8_t bright)
{
  uint16_t r_temp, g_temp, b_temp;
  uint8_t index_mod;
  uint8_t inverse_sat = (sat ^ 255);

  index = index % 768;
  index_mod = index % 256;

  if (index < 256)
  {
    r_temp = index_mod ^ 255;
    g_temp = index_mod;
    b_temp = 0;
  }

  else if (index < 512)
  {
    r_temp = 0;
    g_temp = index_mod ^ 255;
    b_temp = index_mod;
  }

  else if ( index < 768)
  {
    r_temp = index_mod;
    g_temp = 0;
    b_temp = index_mod ^ 255;
  }

  else
  {
    r_temp = 0;
    g_temp = 0;
    b_temp = 0;
  }

  r_temp = ((r_temp * sat) / 255) + inverse_sat;
  g_temp = ((g_temp * sat) / 255) + inverse_sat;
  b_temp = ((b_temp * sat) / 255) + inverse_sat;

  r_temp = (r_temp * bright) / 255;
  g_temp = (g_temp * bright) / 255;
  b_temp = (b_temp * bright) / 255;

  curCol[0]  = (uint8_t)r_temp;
  curCol[1]  = (uint8_t)g_temp;
  curCol[2] = (uint8_t)b_temp;
  //return(color);
}
