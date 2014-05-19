/*
 * llammas.ino
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
#include <tables/saw512_int8.h>
#include <tables/sin512_int8.h>
#include <tables/triangle512_int8.h>
#include <tables/square_no_alias512_int8.h>
#include <ADSR.h>
#include <LowPassFilter.h>
#include "pitchbendArray.h"
#include "multiFilter.h"
#include "lfoFreqArray.h"

#define CONTROL_RATE 256



//Array of sound generating units. (numerically controlled oscillators)
Oscil < SAW512_NUM_CELLS, AUDIO_RATE > nco[] = {
(SAW512_DATA), (SAW512_DATA), (SAW512_DATA)};

//Array of lwo frequency oscillator (acting as modulators)
Oscil < SIN512_NUM_CELLS, CONTROL_RATE > lfo_one(SIN512_DATA),
lfo_two(SIN512_DATA);
//Amplitude envelope
ADSR < CONTROL_RATE > adsr_envelope;
//Filter Envelope
ADSR < CONTROL_RATE > adsr_filter;


//Buffer of last received midi notes
int note_buffer[4];

//Amount of pitch bend in fractional midi notes.
float pb_amount = 0.0;

//Last midi note played
float last_midi_note = 0.0;

//Detune values of the oscilator 2 and 3 in fractional midi notes.
float osc_det[2];
//Detune amount of osc2 and osc3.
int osc_det_amount[2] = {0,0};

//Last values of the lfos.
int last_lfo_values[2];

//Multimode filter.
Multifilter mf(0);

//Destination of the lfo modulations
bool lfo_effect[2][7];          //0:oscil1, 1:oscil2, 2:oscil3, 3:envelope, 4:cutoff, 5: det osc2, 6: det osc3

//Last filter type in use
int last_filter = 0;

//Last read values of knobs.
int last_reading[4];

//Potentiometer readings.
int readings[4][2];

//Array for precalculated osc1,2,3 lfo1 amounts.
int lfo_amount[] = {0,0,0}; 

//Midi notes to add to osc1,2,3 pitch
int pitch[] = {0,0,0};
//Levels of the oscillators.
int nco_levels[] = {255,255,255};
//amount of glitch
int glitch = 0;
//global lfo2 amount
int global_lfo = 0;
/*Handler for the pitchbend midi message.
*Reads a float array from ROM at the index indicated by the most significant byte
*Of the midi message and set the value as the current pitch bend amount.
*/
void handle_pitch_bend(byte channel, byte lsb, byte msb) {

    pb_amount = pgm_read_float_near(PB_ARRAY + msb);
    play_note(last_midi_note);
}

/*
*Handles the midi note on messages.
*
*/
void handle_note_on(byte channel, byte note, byte velocity) {
    //If velocity is 0, the noteOn message is equivalent to a noteOff message.
    if(velocity == 0) {
        handle_note_off(channel, note, velocity);

        return;
    }

    for(int i = 3; i > 0; i--) {
        note_buffer[i] = note_buffer[i - 1];
    }

    note_buffer[0] = note;
    play_note((float) note);
    adsr_filter.noteOn();
    adsr_envelope.noteOn();
}

/*
*Sets the frequency of each oscillator.
*
*/
void play_note(float note) {
    // Note joué sur le clavier additionnée du pitch bend.
    float totalNote = note + pb_amount + pitch[0];

    nco[0].setFreq(Q16n16_to_float(Q16n16_mtof(float_to_Q16n16(totalNote))));
    for(int i = 0; i < 2; i++) {
        // Note globale plus le désacordage de l'oscillateur.
        float noteOscil = osc_det[i] + pitch[i+1] + totalNote;

        nco[i+1].setFreq(Q16n16_to_float(Q16n16_mtof(float_to_Q16n16(noteOscil))));
    }
    last_midi_note = note;
}

/*
*Handles the midi note off message
*/
void handle_note_off(byte channel, byte note, byte velocity) {
    int i = 0;

    while(i < 3 && note_buffer[i] != note) {
        i++;
    }
    while(i < 3) {
        note_buffer[i] = note_buffer[i + 1];
        i += 1;
    }
    if(i < 4) {
        note_buffer[3] = -1;
    }
    if(note_buffer[0] != -1) {
        play_note((float) note_buffer[0]);
    } else {
        adsr_filter.noteOff();
        adsr_envelope.noteOff();
    }
}

/*
*This is run at startup. It sets most variables to defaults values
* and it initializes arrays.
*/
void setup() {
    MIDI.setHandleNoteOn(handle_note_on);
    MIDI.setHandleNoteOff(handle_note_off);
    MIDI.setHandlePitchBend(handle_pitch_bend);
    nco[0].setFreq(440);
    nco[1].setFreq(440);
    nco[2].setFreq(440);
    startMozzi(CONTROL_RATE);
    MIDI.begin();
    lfo_one.setFreq((float) 0.3);
    lfo_two.setFreq(5);
    for(int i = 0; i < 3; i++) {
        note_buffer[i] = -1;
    }
    adsr_envelope.setADLevels(255, 210);
    adsr_envelope.setTimes(188, 345, 65000, 345);
    adsr_envelope.setSustainLevel(255);
    mf.setResonance(156);
    for (int i = 0; i < 4; i ++){
        for (int j = 0; i < 2; i ++){
            readings[i][j] = 0;
        }
    }
    last_lfo_values[0] = 0;
    last_lfo_values[1] = 0;
    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 5; j++) {
            lfo_effect[i][j] = false;
        }
    }
}

// Reads the four potentiometers and compare the result to what was previously
// found. Returns a 2d array in which the first position is which potentiometre was read
// and in the second is the value in the first position and a boolean that is True if the
// value read is different than the last reading.
void read_and_compare(int position, int readings[4][2]) {

    int reading = 0;

    for(int i = 0; i < 4; i++) {
        reading = mozziAnalogRead(i+1);
        readings[i][0] = reading;
        readings[i][1] = ((reading > last_reading[i]+4) || (reading < last_reading[i]-4));
        if(readings[i][1]){
            last_reading[i] = readings[i][0];
        }
    }
}
/**
*Changes the waveform of the oscillator (0 :saw, 1: sin, 3: triangle, 3: square)
*/
void change_waveform(int wave, int osc){
    switch(wave){
        case 0 : 
            nco[osc].setTable(SAW512_DATA);
            break;
        case 1 : 
            nco[osc].setTable(SIN512_DATA);
            break;
        case 2 : 
            nco[osc].setTable(TRIANGLE512_DATA);
            break;
        case 3 : 
            nco[osc].setTable(SQUARE_NO_ALIAS512_DATA);
            break;
    }
}

/**
*Executed 256 times a second. Updates the internal controls of 
*the synth and reads analog values.
*/
void updateControl() {
    last_lfo_values[0] = lfo_one.next();
    last_lfo_values[1] = lfo_two.next();
    int entry_mode = mozziAnalogRead(0) >> 7;
    int cutoff = 255;
    int cutoff_lfo[] = {0,0};
    read_and_compare(entry_mode, readings);

    switch (entry_mode) {
        // Osc1 lfo amount, Osc1 pitch, Osc1 Level, Osc1 Waveform
    case 0:
        if(readings[0][1]) {
            lfo_amount[0] = (last_lfo_values[0]*readings[0][0])>>8;
        }
        if(readings[1][1]) {
            pitch[0] = (readings[1][0]>>5)-16;
        }
        if(readings[2][1]) {
            nco_levels[0] = readings[2][0]>>1;
        }
        if(readings[3][1]) {
            change_waveform(readings[3][0]>>8, 0);
        }
        break;
        // Osc2 lfo, Osc2 pitch, Osc2 detune, Osc2 level
    case 1:
        if(readings[0][1]) {
            lfo_amount[1] = (last_lfo_values[0]*readings[0][0])>>8;
        }
        if(readings[1][1]) {
            pitch[1] = (readings[1][0]>>5)-16;
        }
        if(readings[2][1]) {
            osc_det_amount[0] = readings[2][0]>>2;
        }
        if(readings[3][1]) {
            nco_levels[1] = readings[3][0]>>1;
        }
        break;
        // Osc2 Waveform, Osc3 lfo amount, Osc3 pitch, Osc3 Level
    case 2:
        if(readings[0][1]) {
            change_waveform(readings[0][0]>>8, 1);
        }
        if(readings[1][1]) {
            lfo_amount[2] = (last_lfo_values[0]*readings[1][0])>>8;
        }
        if(readings[2][1]) {
            pitch[2] = (readings[2][0]>>5)-16;
        }
        if(readings[3][1]) {
            nco_levels[2] = readings[3][0]>>1;
        }
        break;
        // Osc3 Waveform, Osc 3 Detune, Amp attack, Amp decay
    case 3:
        if(readings[0][1]) {
            change_waveform(readings[0][0]>>8, 2);
        }
        if(readings[1][1]) {
            osc_det_amount[1] = readings[1][0]>>2;
        }
        if(readings[2][1]) {
            adsr_envelope.setAttackTime(readings[2][0]<<1);
        }
        if(readings[3][1]) {
            adsr_envelope.setDecayTime(readings[3][0]<<1);
        }
        break;
        // Amp sustain Amp Release, Filter attack, Filter Decay
    case 4:
        if(readings[0][1]) {
            adsr_envelope.setSustainLevel(readings[0][0]>>2);
        }
        if(readings[1][1]) {
            adsr_envelope.setReleaseTime(readings[1][0]<<1);
        }
        if(readings[2][1]) {
            adsr_filter.setAttackTime(readings[2][0]<<1);
        }
        if(readings[3][1]) {
            adsr_filter.setDecayTime(readings[3][0]<<1);
        }
        break;
        //  Filter Sustain, Filter Release, LFO1 Freq, LFO1 Waveform
    case 5:
        if(readings[0][1]) {
            adsr_filter.setSustainLevel(readings[0][0]>>2);
        }
        if(readings[1][1]) {
            adsr_envelope.setReleaseTime(readings[1][0]<<1);
        }
        if(readings[2][1]) {
            lfo_one.setFreq(pgm_read_float_near(LFO_ARRAY + (readings[2][0]>>2)));
        }
        if(readings[3][1]) {
            switch(readings[3][0]>>8){
                case 0 : 
                    lfo_one.setTable(SAW512_DATA);
                    break;
                case 1 : 
                    lfo_one.setTable(SIN512_DATA);
                    break;
                case 2 : 
                    lfo_one.setTable(TRIANGLE512_DATA);
                    break;
                case 3 : 
                    lfo_one.setTable(SQUARE_NO_ALIAS512_DATA);
                    break;
            }
        }
        break;
        //  LFO2 Freq, LFO2 Waveform, Filter type, Filter Cutoff
    case 6:
        if(readings[0][1]) {
            lfo_two.setFreq(pgm_read_float_near(LFO_ARRAY + (readings[0][0]>>2)));
        }
        if(readings[1][1]) {
            switch(readings[1][0]>>8){
                case 0 : 
                    lfo_two.setTable(SAW512_DATA);
                    break;
                case 1 : 
                    lfo_two.setTable(SIN512_DATA);
                    break;
                case 2 : 
                    lfo_two.setTable(TRIANGLE512_DATA);
                    break;
                case 3 : 
                    lfo_two.setTable(SQUARE_NO_ALIAS512_DATA);
                    break;
            }
        }
        if(readings[2][1]) {
            mf.changeFilter(readings[2][0]>>8);
        }
        if(readings[3][1]) {
            cutoff = readings[3][0];
        }
        break;
        //  Filter LFO1, Filter LFO2, Glitch amount, global LFO.
    case 7:
        if(readings[0][1]) {
            cutoff_lfo[0] = ((last_lfo_values[0]+127)*(readings[0][0]>>3))>>7;
        }
        if(readings[1][1]) {
            cutoff_lfo[1] = ((last_lfo_values[1]+127)*(readings[1][0]>>3))>>7;
        }
        if(readings[2][1]) {
            glitch = readings[2][0];
        }
        if(readings[3][1]) {
            global_lfo = (last_lfo_values[1]*(readings[3][0]>>2))>>8;
        }
        break;
    }
    

    //Detune
    if(osc_det_amount[0]) {
        osc_det[0] = pgm_read_float_near(PB_ARRAY +((((last_lfo_values[0] + 127)*osc_det_amount[0])>>8) >> 1));
    }
    if(osc_det_amount[1]) {
        osc_det[1] = pgm_read_float_near(PB_ARRAY +((((last_lfo_values[0] + 127)*osc_det_amount[1])>>8) >> 1));
    }
    

    // TODO : Lire la valeur sur des knobs.
    

    for(int i = 0; i < 2; i++) {
        if(cutoff_lfo[i]) {
            cutoff = (cutoff * cutoff_lfo[i]) >> 7;
        }
    }

    mf.setCutoffFreq(cutoff * adsr_envelope.next() >> 8);

    // Lis et traite les valeurs midi.

    MIDI.read();
    play_note(last_midi_note);
    // Update l'adsr.
    adsr_envelope.update();

}

/*
*Fills the audio buffer of the synth. This method does 
*all the needed operation to convert the values from the different 
*signal generators and modifiers to aunique signal that can be outputed.
*/
int updateAudio() {
    
    //Useless operation that juste makes the thing lag more and more as the glitch parameter increases.
    for (int i = 0;  i < glitch;  i ++){
        nco[0].setPhaseFractional(nco[0].getPhaseFractional());
    }
    //fetch the next osc samples
    int osc1 = nco[0].next();
    int osc2 = nco[1].next();
    int osc3 = nco[2].next();

    //applies oscillators lfos.
    if(lfo_amount[0]) {
        osc1 = (osc1*(lfo_amount[0]))>>7;
    }
    if(lfo_amount[1]) {
        osc2 = (osc2*(lfo_amount[1]))>>7;
    }
    if(lfo_amount[2]) {
        osc3 = (osc3*(lfo_amount[2]))>>7;
    }
   


    //adds the ncos together.
    int total = ((osc1*nco_levels[0]>>8) + (osc2*nco_levels[0]>>8) + (osc3*nco_levels[0]>>8)) >> 2;

    //apply the global lfo.
    if(global_lfo) {
        total = (total*global_lfo)>>7;
    }

    //applies the filter and the envelope.
    return (int) (adsr_envelope.next() * (mf.next(total))) >> 2;


}

//Continuously executes mozzi's audiHook method wich sends pwmd voltage values out
//in the real world
void loop() {
    audioHook();
}

