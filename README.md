Llammas
=======

*Llammas is Literally Another Monophonic Mozzi Arduino Synth*.
This MIDI synthesizer has been developped with the [Mozzi library](https://sensorium.github.io/Mozzi/), a sound synthesis library for Arduino.

## Features

Llammas is a Monophonic synthesizer, with 3 oscillators (supporting different waveforms). It includes 2 LFO with flexible routing, 2 enveloppes (filter & amplitude), 3 operator FM synthesis and a multimode resonnant filter.

### Supported filters

- Highpass
- Lowpass
- Bendpass
- Notch

### Supported waveforms

- Sine
- Saw
- Square
- Triangle

### Protocole

The synthesizer supports the MIDI standard and works with any MIDI controller. Theoretically. In fact, too much MIDI signals at once is badly handled, because of the Arduino's capacity.

### Usage

The first potentiometer selects what the four others modify.

#### Mode 0

1. Amplitude modulation of osc 1 by lfo 1;
2. Note played by osc 1;
3. Osc 1 level;
4. Waveform of osc 1;

#### Mode 1

1. Amplitude modulation of osc 2 by lfo 1;
2. Note played by osc 2;
3. Osc 2 detune;
4. Osc 2 level;


#### Mode 2

1. Sinewave of osc 2;
2. Modulation of osc 3 amplitude;
3. Note played by osc 3;
4. Osc 3 level;

#### Mode 3

1. Waveform of osc 3;
2. Osc 3 detune;
3. Attack of amplitude enveloppe
4. Decay of amplitude enveloppe

#### Mode 4

1. Sustain of amplitude enveloppe;
2. Release of amplitude enveloppe;
3. Attack of filter enveloppe;
4. Decay of filter enveloppe;

#### Mode 5

1. Sustain of filter enveloppe;
2. Release of amplitude enveloppe;
3. LFO 1 frequency;
4. Waveform of LFO 1;

#### Mode 6

1. LFO 2 Frequency;
2. Waveform of LFO 2;
3. Filter type;
4. Filter cutoff;

#### Mode 7

1. Modulation of filter cutoff by LFO 1;
2. Modulation of filter cutoff by LFO 2;
3. Amount of glitch;
4. Modulation of global amplitude by LFO 2;


## Licence

This synthesizer has been conceived by Guillaume Riou, Aude Forcione-Lambert & Nicolas Hurtubise and is distributed under the terms of the GNU General Public License v3. See the LICENSE file for more details.
