#include <MIDI.h>
#include <MozziGuts.h>
#include <Oscil.h>
#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include <tables/saw256_int8.h>
#include <tables/sin256_int8.h>
#include <ADSR.h>
#include <LowPassFilter.h>

#define CONTROL_RATE 256

int tableChange =0;
Oscil <SAW256_NUM_CELLS, AUDIO_RATE> nco(SAW256_DATA), nco_det(SAW256_DATA);
Oscil <SIN256_NUM_CELLS, CONTROL_RATE> lfo(SIN256_DATA);
ADSR <CONTROL_RATE> adsr;
int noteOnn = 1;
float lastFreq = 0.0;
int noteOnBuffer[4];
int bufferIndex = 0;
LowPassFilter lpf;

void handleNoteOn(byte channel, byte note, byte velocity){
  //If velocity is 0, the noteOn message is equivalent to a noteOff message.
  if(velocity == 0){
    handleNoteOff(channel,note,velocity);
    return;
  }
  
  for(int i = 3; i> 0 ; i --){
    noteOnBuffer[i] = noteOnBuffer[i-1];
  }
  
  noteOnBuffer[0] = note;
  jouerNote(note);
}

void jouerNote(byte note){
  noteOnn = 1;
  lastFreq = mtof(note);
  nco.setFreq(lastFreq);
  
  adsr.noteOn();
}

void handleNoteOff(byte channel, byte note, byte velocity){
 int i =0;
 while(i<3 && noteOnBuffer[i] != note){
   i ++;
 }
 while(i < 3){
   noteOnBuffer[i] = noteOnBuffer[i+1];
   i+=1;
 }
 if(i < 4){
   noteOnBuffer[3] = -1;
 }
 if(noteOnBuffer[0] != -1){
   jouerNote(noteOnBuffer[0]);
 }else{
   noteOnn = 0;
   adsr.noteOff();
 }
}

void setup(){
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  nco.setFreq(400);
  startMozzi(CONTROL_RATE);
  MIDI.begin();
  lfo.setFreq(1);
  for(int i = 0; i<3; i++){
    noteOnBuffer[i] = -1;
  }
  adsr.setADLevels(255,210);
  adsr.setTimes(188,345,65000,345);
  adsr.setSustainLevel(255);
  lpf.setResonance(156);
}

void updateControl(){
  int lfoNext = lfo.next();
  nco_det.setFreq(lastFreq+(lfoNext*0.02f));
  MIDI.read();
  adsr.update();
  lpf.setCutoffFreq(255*adsr.next()>>8);
}

int updateAudio(){
  //adsr.next();
  return (int)(adsr.next()*(lpf.next((nco.next()+nco_det.next())>>2)))>>4;
}

void loop(){
  audioHook();
}
