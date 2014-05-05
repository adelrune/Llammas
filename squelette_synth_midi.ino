/*
 * squelette_synth_midi.ino
 *
 * Copyright 2014 Guillaume Riou, Aude Forcione-Lambert & Nicolas Hurtubise.
 *
 * This file is part of Llammas.
 *
 * Llammas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Llammas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Llammas.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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

Oscil <SAW256_NUM_CELLS, AUDIO_RATE> ncoOne(SAW256_DATA),
ncoTwo(SAW256_DATA), ncoThree(SAW256_DATA);

Oscil <SIN256_NUM_CELLS, CONTROL_RATE> lfoOne(SIN256_DATA),
lfoTwo(SIN256_DATA);

ADSR <CONTROL_RATE> adsr_envelope;
ADSR <CONTROL_RATE> adsr_filter;


float lastFreq = 0;
int noteBuffer[4];
int bufferIndex = 0;
float pbAmount = 0.0;
float lastMidiNote = 0.0;
LowPassFilter lpf;
int lastLfoValues[2];
Multifilter mf(0);
bool lfoEffect[2][5]; //0:oscil1, 1:oscil2, 2:oscil3, 3:envelope, 4:detune

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
}

void jouerNote(float note) {
    float totalNote = note + pbAmount;
    lastFreq = Q16n16_to_float(Q16n16_mtof(float_to_Q16n16(totalNote)));
    ncoOne.setFreq(lastFreq);
    lastMidiNote = note;
    adsr_filter.noteOn();
    adsr_envelope.noteOn();
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
    ncoOne.setFreq(400);
    startMozzi(CONTROL_RATE);
    MIDI.begin();
    lfoOne.setFreq(1);
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

int lastFilter = 0;
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

    MIDI.read();
    adsr_envelope.update();
    mf.setCutoffFreq(255 * adsr_envelope.next() >> 8);
}

int updateAudio() {
    //adsr.next();

    /**
    int osc1 = ncoOne.next();
    int osc2 = ncoTwo.next();
    int osc3 = ncoThree.next();
    */
   /** for(int i=0;i<2;i++) {
        if(lfoEffect[i][0] {
            osc1 = osc1*lastLfoValues[i];
        }
        if(lfoEffect[i][1] {
            osc2 = osc2*lastLfoValues[i];
        }
        if(lfoEffect[i][2] {
            osc3 = osc3*lastLfoValues[i];
        }
        if(lfoEffect[i][3] {
            //envelope
        }
        if(lfoEffect[i][4] {
            //detune
        }
    } */ 


    return (int) (adsr_envelope.next()*(mf.next((ncoOne.next() + ncoTwo.next() + ncoThree.next()) >> 2))) >> 2;
}

void loop() {
    audioHook();
}

