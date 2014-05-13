#include <MIDI.h>
#include <MozziGuts.h>
#include <Oscil.h>
#include <mozzi_midi.h>
#include <mozzi_fixmath.h>
#include <tables/saw256_int8.h>
#include <tables/sin256_int8.h>
#include <ADSR.h>
#include <LowPassFilter.h>
#include "pitchbendArray.h"
#include "multiFilter.h"
#include "lfoFreqArray.h"

#define CONTROL_RATE 256


int tableChange = 0;

Oscil <SAW256_NUM_CELLS, AUDIO_RATE> nco[] =
{(SAW256_DATA), (SAW256_DATA), (SAW256_DATA)};

Oscil <SIN256_NUM_CELLS, CONTROL_RATE> lfoOne(SIN256_DATA),
lfoTwo(SIN256_DATA);

ADSR <CONTROL_RATE> adsr_envelope;
ADSR <CONTROL_RATE> adsr_filter;



int noteBuffer[4];
int bufferIndex = 0;
float pbAmount = 0.0;
float lastMidiNote = 0.0;
float oscDet[2];
LowPassFilter lpf;
int lastLfoValues[2];
Multifilter mf(0);
bool lfoEffect[2][7]; //0:oscil1, 1:oscil2, 2:oscil3, 3:envelope, 4:cutoff, 5: det osc2, 6: det osc3
int lastFilter = 0;

void handlePitchBend(byte channel, byte lsb, byte msb) {
    
    pbAmount = pgm_read_float_near(PB_ARRAY+msb);
    jouerNote(lastMidiNote);
}

void handleNoteOn(byte channel, byte note, byte velocity) {
    //If velocity is 0, the noteOn message is equivalent to a noteOff message.
    if(velocity == 0) {
        handleNoteOff(channel, note, velocity);

        return;
    }

    for(int i = 3; i > 0; i--) {
        noteBuffer[i] = noteBuffer[i - 1];
    }

    noteBuffer[0] = note;
    jouerNote((float)note);
    adsr_filter.noteOn();
    adsr_envelope.noteOn();
}

void jouerNote(float note) {
    // Note joué sur le clavier additionnée du pitch bend.
    float totalNote = note + pbAmount;
    nco[0].setFreq(Q16n16_to_float(Q16n16_mtof(float_to_Q16n16(totalNote))));
    for(int i = 0; i < 2; i++) {
        // Note globale plus le désacordage de l'oscillateur.
        float noteOscil = oscDet[i] + totalNote;
        nco[i+1].setFreq(Q16n16_to_float(Q16n16_mtof(float_to_Q16n16(noteOscil))));
    }
    //lastFreq = Q16n16_to_float(Q16n16_mtof(float_to_Q16n16(totalNote)));
    //nco[0].setFreq(lastFreq);
   // nco[1].setFreq(Q16n16_to_float(Q16n16_mtof(float_to_Q16n16(totalNote+12))));
   // nco[2].setFreq(lastFreq);
    lastMidiNote = note;

}

void handleNoteOff(byte channel, byte note, byte velocity) {
    int i = 0;

    while(i < 3 && noteBuffer[i] != note) {
        i++;
    }
    while(i < 3) {
        noteBuffer[i] = noteBuffer[i + 1];
        i += 1;
    }
    if(i < 4) {
        noteBuffer[3] = -1;
    }
    if(noteBuffer[0] != -1) {
        jouerNote((float)noteBuffer[0]);
    } else {
    
        adsr_filter.noteOff();
        adsr_envelope.noteOff();
    }
}

void setup() {
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    nco[0].setFreq(400);
    nco[1].setFreq(333);
    //nco[2].setFreq(300);
    startMozzi(CONTROL_RATE);
    MIDI.begin();
    lfoOne.setFreq((float)0.3);
    lfoTwo.setFreq(5);
    for(int i = 0; i < 3; i++) {
        noteBuffer[i] = -1;
    }
    adsr_envelope.setADLevels(255, 210);
    adsr_envelope.setTimes(188, 345, 65000, 345);
    adsr_envelope.setSustainLevel(255);
    mf.setResonance(156);
    lastLfoValues[0] = 0;
    lastLfoValues[1] = 0;
}


void updateControl() {
    lastLfoValues[0] = lfoOne.next();
    lastLfoValues[1] = lfoTwo.next();
    // Oscillator detune.
    // 
    int filterReading = mozziAnalogRead(0)>>8;
    if (lastFilter != filterReading ) {
        mf.changeFilter(filterReading);
        lastFilter = filterReading;
    }
    
    for (int i = 0; i < 2; i ++){
        if(lfoEffect[i][5] || true){
            oscDet[0] = pgm_read_float_near(PB_ARRAY+((lastLfoValues[i]+127)>>1));
        }
        if(lfoEffect[i][6]){
            oscDet[1] = pgm_read_float_near(PB_ARRAY+((lastLfoValues[i]+127)>>1));
        }
    }
    
    // TODO : Lire la valeur sur des knobs.
    int cutoff = 255;
    for (int i = 0 ; i <2 ; i++){
        if(lfoEffect[i][4]){
            //cutoff = (cutoff * (lastLfoValues[i]+127)) >>7;
        }
    }
    
    mf.setCutoffFreq(cutoff * adsr_envelope.next() >> 8);
    // Lis et traite les valeurs midi.
    MIDI.read();
    jouerNote(lastMidiNote);
    // Update l'adsr.
    adsr_envelope.update();
    
}

int updateAudio() {
    //adsr.next();
    
    
    int osc1 = nco[0].next();
    int osc2 = nco[1].next();
    int osc3 = /*nco[2].next()*/0;
    
    for(int i=0;i<2;i++) {
        if(lfoEffect[i][0] || true) {
            //osc1 = (osc1*lastLfoValues[i])>>7;
        }
        if(lfoEffect[i][1] || true) {
            //osc2 = (osc2*lastLfoValues[i])>>7;
        }
        if(lfoEffect[i][2] || true) {
            //osc3 = (osc3*lastLfoValues[i])>>7;
        }
        /**if(lfoEffect[i][4]) {
            total = ((osc1+osc2+osc3)*lastLfoValues[i])>>9;
        }
        else {
            total = (osc1+osc2+osc3)>>2;
        }*/
    } 
    int total = (osc1+osc2+osc3)>>2;
    
    for(int i=0; i<2; i++){
        if(lfoEffect[i][3] || true) {
            //total = (total*lastLfoValues[i])>>7;
        }
    }
    
    
    return (int) (adsr_envelope.next()*(mf.next(total)))>>2;
}

void loop() {
    audioHook();
}

