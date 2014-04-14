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


#define CONTROL_RATE 256


int tableChange = 0;

Oscil <SAW256_NUM_CELLS, AUDIO_RATE> ncoOne(SAW256_DATA),
ncoTwo(SAW256_DATA), ncoThree(SAW256_DATA);
Oscil <SIN256_NUM_CELLS, CONTROL_RATE> lfo(SIN256_DATA);
ADSR <CONTROL_RATE> adsr;
int noteOnn = 1;
float lastFreq = 0;
int noteOnBuffer[4];
int bufferIndex = 0;
float pbAmount = 0.0;
float lastMidiNote = 0.0;
LowPassFilter lpf;

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
        noteOnBuffer[i] = noteOnBuffer[i - 1];
    }

    noteOnBuffer[0] = note;
    jouerNote((float)note);
}

void jouerNote(float note) {
    float totalNote = note + pbAmount;
    lastFreq = Q16n16_to_float(Q16n16_mtof(float_to_Q16n16(totalNote)));
    ncoOne.setFreq(lastFreq);
    lastMidiNote = note;
    adsr.noteOn();
}

void handleNoteOff(byte channel, byte note, byte velocity) {
    int i = 0;

    while(i < 3 && noteOnBuffer[i] != note) {
        i++;
    }
    while(i < 3) {
        noteOnBuffer[i] = noteOnBuffer[i + 1];
        i += 1;
    }
    if(i < 4) {
        noteOnBuffer[3] = -1;
    }
    if(noteOnBuffer[0] != -1) {
        jouerNote((float)noteOnBuffer[0]);
    } else {
        noteOnn = 0;
        adsr.noteOff();
    }
}

void setup() {
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.setHandlePitchBend(handlePitchBend);
    ncoOne.setFreq(400);
    startMozzi(CONTROL_RATE);
    MIDI.begin();
    lfo.setFreq(1);
    for(int i = 0; i < 3; i++) {
        noteOnBuffer[i] = -1;
    }
    adsr.setADLevels(255, 210);
    adsr.setTimes(188, 345, 65000, 345);
    adsr.setSustainLevel(255);
    lpf.setResonance(156);
    
}

int lfoNext = 0;
void updateControl() {
    lfoNext = lfo.next();
    ncoTwo.setFreq(lastFreq + (lfoNext * 0.02f));
    //version tarrée à virgule fixe.
    //ncoThree.setFreq((float)(((Q15n0_to_Q15n16((mozziAnalogRead(5)-512))*(float_to_Q15n16(0.004))>>23)*float_to_Q15n16(lastFreq))>>23));
    MIDI.read();
    adsr.update();
    lpf.setCutoffFreq(255 * adsr.next() >> 8);
}

int updateAudio() {
    //adsr.next();
    return (int) (adsr.next()*(lpf.next((ncoOne.next() + ncoTwo.next() + ncoThree.next()) >> 2))) >> 2;
}

void loop() {
    audioHook();
}

