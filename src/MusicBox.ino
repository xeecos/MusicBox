
#include "MeLEDMatrix.h"
#include "synth.h"
#include "song.h"
#include <EEPROM.h>
#include <avr/pgmspace.h>
MeLEDMatrix ledMx1(A0,A1);
MeLEDMatrix ledMx2(A2,A3);

synth edgar; //declare a synth

uint8_t faceA[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t faceB[16] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

byte version = 2; //change this to 1 if you're using MintySynth rev. 1.0-1.3 (without a photocell)

const byte LED1 =  2;      // the LED pins
const byte LED2 =  4;
int thisStep = 0; //step counter
int POT[5] = {5000,5000,5000,5000,5000};//holds readings for each pot
long stepLength = 250; //the length of each step, in miliseconds
byte userWave = 0; //the waveform
byte scalePitch = 62; //MIDI pitch, 0-127
byte userPitch = 62; //MIDI pitch once the selected scale is applied.
byte userEnv = 2; //envelope, 0-3
unsigned int userLength = 70;  //the length (duration) of each note, in milliseconds, 0-127. Not to be confused with step-- can be longer or shorter than the step.
byte userMod = 64;  //modulation, the pitch shift up or down over the duration of the note, 0-127
byte Swing = 0; //swing, the percentage of the step length that is added to even notes and subtracted from odd notes (the note numbers start with 0, which is even)
unsigned int swingLength; //length of the step once swing is taken into consideration
unsigned long triggerTime; //the time at the beginning of the step
boolean livePots[5] = {0,0,0,0,0}; //true if pot has been turned (unlocked), false if it has been locked and not yet turned
static long readTime=100; //the last time the pots were read
unsigned int batteryLevel; //the smoothed battery level, 0-1023
boolean batteryLow=0;
static long batteryTime=60000; //the last time the battery was checked. Set high so we check once at startup.
boolean mixerMode = 0; //mixerMode let us set volume levels for each voice. MixerMode can be on during Program Mode or Live Mode.
byte Current = 2; //the song that is currently being played. 0 and 1 are the two halves of the demo song
byte programMode = 0; //0-3 are Program Modes for voices 0-3. 4 means Program Mode is off (we're in Live mode).
boolean sequMode = 0; //Sequencer Mode, which is a subset of Program Mode. When we enter Sequencer Mode from Program Mode for voice i, we're in Sequencer Mode for voice i.
byte modeChange = 0; //This is a flag indicating that we've just changed modes. We use it to indicate whether to unflag justreleased if we were just using the buttons for the mode change. The value indicates the number of buttons that have simultaneously been pressed to change modes.
byte appendMode = 0; //0 if we're only playing song two (or the demo song), 2 if we're cycling through songs 2 and 3, 3 if we're cycling through 2-4; and 4 if we're cycling through 2-5
boolean LED1state = 0; //keep track of the LED states so we know when to turn them off.
boolean LED2state = 0;
byte scaleNumber = 0; //the current scale mode, 0-7 (see tables.h for scale names) 0 is chromatic
boolean pause = 0; //pausees the trigger timer, as opposed to suspend(), which pauses the audio engine. We use pause when we need to have the audio engine available for another use
boolean ascending = 1; //tells us if we're ascending or descending when playing a sample scale
byte scaleNote = 0; //the current note in a sample scale
boolean scaleChange; //tells us if we've changed the scale selection
byte prevScaleNumber; //the previous scale selection, so we can track whether it's changed
boolean envChange = 0; //tells us if we've just changed the envelope, so we know whether to show a binary LED indicator
byte prevEnv = 2; //the previous envelope selection
unsigned long envTimer; //used to turn off the envelope binary LED counter when the envelope pot hasn't been turned in the set amount of time
boolean envMode; //tells us if we've changed the envelope within the set amount of time
byte switchWave[5] = {11,1,6,7,0}; //the programmed waveform for each button.
boolean wavProgMode = 0; //tells us if we're in WavProgram Mode
boolean potLockFlag = 0; //set if we just locked pot0, so we can monitor when it becomes live. Used in WavProgMode.
int songShift = 0; //the number of semitones (positive or negative) by which to shift the entire currently playing song
int scaleShift = 0; //the number of semitones (positive or negative) by which to shift the current scale
boolean shiftMode = 0; //used to tell when we've just shifted the song or scale, for displaying the binary indicator.
unsigned long shiftTimer; //used to turn off the songShift/scaleSift binary indicator after some time.
unsigned long waveTimer; //used to tell us when to pause the music if we're programming waveforms.
byte voice0Volume = 8; //we remember the current volume setting of voice 0 so we can turn it up to play sample scakles and waveforms and then turn it back down if necessary.
unsigned int LDRread; //reading from the LDR (photocell)
boolean LDRMode = 0; //if we're in LDR mode (and in Live Mode) we read pitch from the LDR instead of Pot 1.
unsigned int LDRMax; //used to calibrate the LDR.
unsigned int LDRMin = 1023; //used to calibrate the LDR.
boolean tripwireMode = 0; //A simple tripwire alarm function, just for fun.
unsigned long LDRTimer; //used while calibrating the LDR for tripwireMode

//variables for MIDI (there are more on the synth.h tab):
byte MIDIswitchWave[5] = {1,57,73,17,1}; //the programmed MIDI instrument for each button.
byte MIDIswitchChannel[5] = {10,1,1,1,1}; //the programmed MIDI channel for each button.
byte currentInstrument = 0; //the instrument to which the voice that we're controlling is set (analagous to userWave), used to set MIDI voice when we trigger the MIDI note.
byte currentChannel = 1; //the instrument to which the voice that we're controlling is set, used to set MIDI voice when we trigger the MIDI note.
boolean instrumentProgMode = 0; //tells us if we're in instrumentProgram Mode
boolean potLockFlag1 = 0; //set if we just locked pot1, so we can monitor when it becomes live. Used in instrumentProgMode.
unsigned long instrumentTimer; //used to tell us when to pause the music if we're programming waveforms.
byte MIDIstate = 0; //0 is MIDI off, 1 is baud rate 115200, and 2 is baud rate 31250

#define DEBOUNCE 10  // button debouncer, how many ms to debounce, 5+ ms is usually plenty
byte buttons[] = {// here is where we define the buttons that we'll use. button "0" is the first, button "5" is the 6th, etc
  5, 7, 11, 8, 10}; // button pins
#define NUMBUTTONS sizeof(buttons)// This handy macro lets us determine how big the array up above is, by checking the size
byte pressed[NUMBUTTONS], justpressed[NUMBUTTONS], justreleased[NUMBUTTONS], longpress[NUMBUTTONS], extralongpress[NUMBUTTONS]; // we will track if a button is just pressed, just released, or 'currently pressed'

void setup(){
    pinMode(9,INPUT);
    pinMode(10,INPUT);
    
    ledMx1.setBrightness(2);
    ledMx1.setColorIndex(1);
    ledMx2.setBrightness(2);
    ledMx2.setColorIndex(1);

    // DIDR0 = 0x3F;   //disable the digital input buffers on the analog pins to save a bit of power and reduce noise.
    edgar.begin(CHA); 
    Serial.begin(9600);
}
float l = 0;
int prevR = 0;
float prevT = 0;
int index = 0;
int bufferIndex = 0;
int tt[16] = {0};
float dir = 1;
float lastNote = 1;
long delayTime = 0;
long lastAutoPlay = 0;
bool isAutoPlay = false;
int trackIndex = 0;
int musicLength[2] = {981,2699};
int musicIndex = 0;
int songIndex = 1;
long musicTime = 0;
float speed = 3;
int instrumentId = 10;
void loop(){
    l = (digitalRead(9)<<1)+digitalRead(10);
    int currentR = l*100;
    Serial.println(currentR);
    if(prevR!=currentR){
        speed-=4.5;
        speed = fmax(speed,1.8);
    }else{
        speed*=1.15;
        speed = fmin(speed,12);
    }
    if(speed<10){
        autoPlay();
    }
    prevR = currentR;
    renderLED();
}

void setPianoNote(int track, int n,float t){
      edgar.setupVoice(track%4, instrumentId, n, 0, fminf(95,fmaxf(40,900*t/80+40)), 64);
      edgar.trigger(track%4);
}

void parseNote(int track,int note,int during){
  int n = (int)((fmaxf(0,(note-48))/35.0)*15.0);
  tt[n]=0xfff;
  for(int i=-4;i<0;i++){
    if(n+i<=15&&n+i>=0){
      tt[n+i]=0xfff>>(abs(i)*4);
    }
  }
  setPianoNote(track,note,during);
}
int reverse(int t){
  int o = 0;
  for(int i=0;i<8;i++){
    o +=(t>>i&1)<<(7-i);
  }
  return o;
}
void autoPlay(){
  float ttt = fmaxf(0,pgm_read_byte((songIndex==1?musicTicks2:musicTicks1)+(musicIndex)));
  if(millis()-musicTime>=ttt*speed){
    musicTime = millis();
    if(musicIndex<=musicLength[songIndex]){
      int d = pgm_read_byte((songIndex==1?musicNotes2:musicNotes1)+musicIndex);
      int track = d>>6;
      int note = (d&B111111)+(songIndex==1?25:20);
      parseNote(track%4,note,pgm_read_byte((songIndex==1?musicDuring2:musicDuring1)+musicIndex));
      musicIndex++;
    }else{
      musicIndex = 0;
      songIndex++;
      songIndex = songIndex%2;
      lastAutoPlay = millis();
    }
  }
}
void renderLED(){
    if(millis()-delayTime>20){
        delayTime = millis();
        for(int i=0;i<16;i++){
            faceA[i] = 0;
            faceB[i] = 0;
        }
        for(int i=0;i<16;i++){
            faceB[i] = (tt[i]&0xff);
            faceA[i] = ((tt[i]>>7)&0xff);
            
            if(tt[i]>0){
                if(index%2==0){
                    tt[i]=tt[i]>>1;
                }
            }else{
                tt[i]=0;
            }
        }
        index++;
        if(index>14){
            index = 1;
        }
        ledMx1.drawBitmap(0,0,16,faceA);
        ledMx2.drawBitmap(0,0,16,faceB);
    }
}