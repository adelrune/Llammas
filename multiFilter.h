#ifndef MultiFilter_H
#    define MultiFilter_H
#    include <StateVariable.h>
#    include <LowPassFilter.h>

class Multifilter {
  private:
    int filterSelection;
    int freq;
    int res;
    LowPassFilter lpf;
    StateVariable <NOTCH> nf;
    StateVariable <HIGHPASS> hpf;
    StateVariable <BANDPASS> bpf;

  public:
    
    Multifilter(int type){
        filterSelection = type;
    }
    
    /**
    * 0: Lowpass 1: Highpass, 2 : Bandpass, 3: Notch, 4 : Allpass.
    */
    int changeFilter(int selection) {
        if (filterSelection != selection){
            filterSelection = selection;
            setResonance(res);
            setCutoffFreq(freq);
        }
    }

    int setResonance(int resonance) {
        res = resonance;
        switch (filterSelection) {
        case 0:
            lpf.setResonance(resonance);
            break;
        case 1:
            // Oh non, c'est trop louuurd.
            //hpf.setResonance(map(resonance, 180, 1, 0, 255));
            // Approximation d'un mappage de valeurs de 0 à 255 vers 20 à 147.
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

    int setCutoffFreq(int cutoff) {
        freq = cutoff;
        switch (filterSelection) {
        case 0:
            lpf.setCutoffFreq(cutoff);
            break;
        case 1:
            // Approximation de mappage de valeurs de 0 à 255 vers 20 à 4100.
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

