/*
 * multiFilter.h
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
#ifndef MultiFilter_H
#    define MultiFilter_H
#    include <StateVariable.h>
#    include <LowPassFilter.h>

class Multifilter {
  private:
    //Current selected filter
    int filterSelection;
    //Current filter frequency
    int freq;
    //Current filter resonance.
    int res;
    //Mozzi's lowpass filter.
    LowPassFilter lpf;
    //Mozzi's multimode filter in notch mode
    StateVariable <NOTCH> nf;
    //Mozzi's multimode filter in highpass mode
    StateVariable <HIGHPASS> hpf;
    //Mozzi's multimode filter in bandpass mode
    StateVariable <BANDPASS> bpf;

  public:
    //Constructor. The type parameter is the type of the filter used at first.
    Multifilter(int type){
        filterSelection = type;
    }
    
    /**
    *Changes the filter type and sets the new filter to the right value.
    * 0: Lowpass 1: Highpass, 2 : Bandpass, 3: Notch, 4 : Allpass.
    */
    int changeFilter(int selection) {
        if (filterSelection != selection){
            filterSelection = selection;
            setResonance(res);
            setCutoffFreq(freq);
        }
    }
    /**
    *Sets the resonance of the filter. Takes care of the conversion if they are needed. 
    */
    int setResonance(int resonance) {
        res = resonance;
        switch (filterSelection) {
        case 0:
            lpf.setResonance(resonance);
            break;
        case 1:
           
            // maps values from 0-255 to 147-20
            hpf.setResonance(180 - (resonance>>1) -33);
            break;
        case 2:
            bpf.setResonance(180 - (resonance>>1) -33);
            break;
        case 3:
            nf.setResonance(180 - (resonance>>1) -33);
            break;
        }
    }
    /**
    *Sets the cutoff of the filter. Takes care of the conversion if they are needed. 
    */
    int setCutoffFreq(int cutoff) {
        freq = cutoff;
        switch (filterSelection) {
        case 0:
            lpf.setCutoffFreq(cutoff);
            break;
        case 1:
            // maps values from 0-255 to 20-4100
            hpf.setCentreFreq((cutoff<<4)+20);
            break;
        case 2:
            
            bpf.setCentreFreq((cutoff<<4)+20);
            break;
        case 3:
            
            nf.setCentreFreq((cutoff<<4)+20);
            break;
        }
    }
    /**
    *returns the signal affected by the selected filter.
    */
    int next(int signal) {
        switch (filterSelection) {
        case 0:
            return lpf.next(signal);

        case 1:
            return hpf.next(signal);

        case 2:
            return bpf.next(signal);

        case 3:
            return nf.next(signal);

        case 4:
            return signal;

        }
    }

};
#endif

