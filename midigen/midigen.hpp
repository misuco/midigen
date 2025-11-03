/**
 * Midigen
 *
 * Midi music generator
 *
 * (c) 2025 by claudio zopfi
 *
 * Licence: GNU/GPL
 *
 * https://github.com/misuco/apolloqa
 *
 */

#ifndef MIDIGEN_HPP
#define MIDIGEN_HPP

#include "MidiFile.h"
#include "Options.h"

#include <map>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;
using namespace smf;

class Midigen {

public:

    Midigen();
    ~Midigen() {}

    void setFilename(const string &f);
    void setBPM(int b);
    void setLen(int l);
    void setQuantize(int q);
    void setDensity(int d);
    void setSoundfont(string p);
    void addInstrument(int i);
    void addChord(string c);
    void newMidiFile();
    void saveNewMidiFile();

private:
    MidiFile    midiOut;
    
    int         _sampleRate;
    int         _tpq;           // ticks per quarter (1/4 note)
    
    string              _soundfont;
    std::vector<int>    _instruments;
    std::vector<string> _chords;
    
    int         _bpm;
    int         _len;        // 1 Beat = 1/4 Note
    int         _quantize;
    int         _density;
    
    string      _filename;
    
    void createChordsTrack();
    int str2midinote(const string &note);
    string midinote2txt(const int &note);
};

#endif // MIDIGEN_HPP
