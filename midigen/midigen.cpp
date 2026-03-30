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

#include "midigen.hpp"
#include <sstream>
#include <stdlib.h>
#include <stdexcept> // exec
#include <algorithm> // rtrim
#include <random>
#include <format>

std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

// Trim from the end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void Midigen::setFilename(const string &f)
{
    _filename = f;
}

void Midigen::setBPM(int b)
{
    _bpm = b;
}

void Midigen::setBeatsPerChord(int l)
{
    _beatsPerChord = l;
}

void Midigen::setQuantize(int q)
{
    _quantize = q;
}

void Midigen::setSteps(string s)
{
    for(char& c : s) {
        if(c=='1') {
            _steps.push_back(1);
        } else {
            _steps.push_back(0);
        }
    }
}

void Midigen::setSoundfont(string p)
{
    _soundfont = p;
}

void Midigen::setInstrumentPreset(int p)
{
    _instrumentPreset = p;
}

void Midigen::setInstrumentBank(int b)
{
    _instrumentBank = b;
}

void Midigen::addChord(string c) {
    _chords.push_back(c);
}

Midigen::Midigen() :
_tpq {48},  // default value in MIDI file is 48
_bpm {140},
_sampleRate {48000},
_beatsPerChord {4},
_soundfont {"/home/apolloqa/sf2/Touhou.sf2"}
{
}

void Midigen::newMidiFile() {

    MidiEvent tempoEvent;

    vector<uchar> midievent;    // temporary storage for MIDI events
    midievent.resize(3);        // set the size of the array to 3 bytes

    midiOut.clear();
    midiOut.absoluteTicks();    // time information stored as absolute time
    // (will be coverted to delta time when written)
    midiOut.addTrack(2);        // Add another two tracks to the MIDI file
    midiOut.setTicksPerQuarterNote(_tpq);

    tempoEvent.setTempo(_bpm);
    midiOut.addEvent( 0, 0, tempoEvent );

    createChordsTrack();

    // End Of Track
    int endtick=_chords.size()*_beatsPerChord*_tpq/_quantize;
    cout << "endtick " << endtick << endl;
    midiOut.addMetaEvent( 0, endtick, 0x2F, "" );
    midiOut.addCopyright( 0, 0, "(c) 2026 by apolloqa.net / Claudio Zopfi" );
    midiOut.sortTracks();
}

void Midigen::createChordsTrack() {
    // Random Generator
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist127(0,127); // distribution in range [1, 6]
    std::uniform_int_distribution<std::mt19937::result_type> dist32k(0,32767); // distribution in range [1, 6]

    //string command = sprintf("echo prog 0 %d > %s.conf",randomInstrument, _filename);
    //string command = std::format("echo prog 0 {} > {}.conf",_instrumentPreset ,_filename);
    string command = std::format("echo select 0 0 {} {} > {}.conf", _instrumentBank, _instrumentPreset ,_filename);
    system( command.c_str() );
    cout << "executed " << command << endl;

    /*
    MidiEvent bankMsb( 176, _instrumentBank/128 );
    midiOut.addEvent( 0, 0, bankMsb );

    MidiEvent bankLsb( 208, _instrumentBank%128 );
    midiOut.addEvent( 0, 0, bankLsb );
    */

    MidiEvent pc( 192, _instrumentPreset );
    midiOut.addEvent( 0, 0, pc );

    std::vector<int> chordNoteSet;

    int chordId=0;

    // number of 16th notes to generate
    int ticksPerNote=_tpq*4/_quantize;

    for(auto chord:_chords) {
        //int startTick=chordId*n16th*_tp16th;
        int startTick=chordId*_beatsPerChord*_tpq;

        // calculate available notes in current chord
        chordNoteSet.clear();
        int chordBaseNote=str2midinote(chord);

        char lastChar=chord.back();

        for(int octave=2;octave<7;octave++) {
            if(lastChar=='m') {
                // minor chord
                chordNoteSet.push_back(chordBaseNote+12*octave);
                chordNoteSet.push_back(chordBaseNote+3+12*octave);
                chordNoteSet.push_back(chordBaseNote+7+12*octave);
            } else if(lastChar=='7') {
                    // minor chord
                    chordNoteSet.push_back(chordBaseNote+12*octave);
                    chordNoteSet.push_back(chordBaseNote+4+12*octave);
                    chordNoteSet.push_back(chordBaseNote+7+12*octave);
                    chordNoteSet.push_back(chordBaseNote+10+12*octave);
            } else {
                // major chord
                chordNoteSet.push_back(chordBaseNote+12*octave);
                chordNoteSet.push_back(chordBaseNote+4+12*octave);
                chordNoteSet.push_back(chordBaseNote+7+12*octave);
            }
        }
        int noteSetSize=chordNoteSet.size();

        // add calculated notes to midifile
        MidiEvent midievent;
        int randomNote;
        bool noteOn=false;

        for(int j=0;j<_beatsPerChord;j++) {
            int tick;
            for(int i=0;i<_quantize;i++) {
                if(_steps[i%8]) {
                    tick=startTick+((j*_beatsPerChord+i)*ticksPerNote);

                    // previous note Off
                    if(noteOn) {
                        midievent.setCommand(0x80,randomNote,0x00);
                        midiOut.addEvent( 0, tick-1, midievent );
                        noteOn=false;
                    }

                    // calculate new note
                    randomNote = chordNoteSet.at(dist32k(rng)%noteSetSize);
                    int randomVelocity = 64+(dist127(rng)%2)*63;

                    // new note On
                    midievent.setCommand(0x90,randomNote,randomVelocity);
                    midiOut.addEvent( 0, tick, midievent );
                    noteOn=true;
                }
            }

            // final note Off
            if(noteOn) {
                midievent.setCommand(0x80,randomNote,0x00);
                midiOut.addEvent( 0, tick+ticksPerNote-1, midievent );
                noteOn=false;
            }
        }

        chordId++;
    }
}

void Midigen::saveNewMidiFile()
{
    midiOut.write(_filename+".mid");
    string command = "fluidsynth \"" + _soundfont + "\" " + _filename + ".mid -f " + _filename + ".conf -F " + _filename + "-uncut.wav -r 48000 -O s24";
    system( command.c_str() );
    cout << "executed " << command << endl;

    int nChords=_chords.size();
    int nSamples=60*_sampleRate*_beatsPerChord*nChords/_bpm;
    string nSamplesIsCommand="soxi -s "+ _filename + "-uncut.wav ";
    string nSamplesIs=exec( nSamplesIsCommand.c_str() );
    rtrim(nSamplesIs);
    int padSamples=nSamples - stoi(nSamplesIs);

    cout << "bpms " << _bpm << " chords " << nChords << " samples: target " << nSamples << " is " << nSamplesIs << " pad " << padSamples << endl;

    if(padSamples<0) {
        int trimSamples=stoi(nSamplesIs) - nSamples;
        string nSamplesTrim=to_string(nSamples);
        //string nSamplesTrimFrom=to_string(nSamples);
        command = "sox " + _filename + "-uncut.wav " + _filename + ".wav " + " trim 0 " + nSamplesTrim + "s";
        //command = "sox " + _filename + "-uncut.wav " + _filename + ".wav " + " trim " + nSamplesTrimFrom + "s " + nSamplesTrim + "s";
        //command = "sox " + _filename + "-uncut.wav " + _filename + ".wav " + " pad 0s@" + nSamplesIs + "s";
        system( command.c_str() );
        cout << "executed " << command << endl;
    } else {
        string nSamplesPad=to_string(padSamples);
        command = "sox " + _filename + "-uncut.wav " + _filename + ".wav " + " pad " + nSamplesPad + "s@" + nSamplesIs + "s";
        system( command.c_str() );
        cout << "executed " << command << endl;
    }


    command = "ffmpeg -y -i " + _filename + ".wav -acodec libvorbis -ab 128k " + _filename + ".ogg";
    system( command.c_str() );
    cout << "executed " << command << endl;

    string nSamplesIsMP3Command="soxi -s "+ _filename + ".ogg";
    string nSamplesIsMP3=exec( nSamplesIsMP3Command.c_str() );
    cout << "executed " << nSamplesIsMP3Command << endl;
    cout << "samples mp3: " << nSamplesIsMP3 << endl;
}

int Midigen::str2midinote(const string &note) {
    string firstChar=note.substr(0,1);
    string secondChar=note.substr(1,1);
    int midinote=-1;
    if (firstChar=="C") {
        if (secondChar=="#") {
            midinote=1;
        } else {
            midinote=0;
        }
    } else if (firstChar=="D") {
        if (secondChar=="#") {
            midinote=3;
        } else {
            midinote=2;
        }
    } else if (firstChar=="E") {
        midinote=4;
    } else if (firstChar=="F") {
        if (secondChar=="#") {
            midinote=6;
        } else {
            midinote=5;
        }
    } else if (firstChar=="G") {
        if (secondChar=="#") {
            midinote=8;
        } else {
            midinote=7;
        }
    } else if (firstChar=="A") {
        if (secondChar=="#") {
            midinote=10;
        } else {
            midinote=9;
        }
    } else if (firstChar=="B") {
        midinote=11;
    } else if (firstChar=="H") {
        midinote=11;
    }
    return midinote;
}

string Midigen::midinote2txt(const int &note) {
    int octave=note/12;
    int v=note%12;
    std::string txt = "?";

    switch (v) {
        case 0:
        txt="C";
        break;
        case 1:
        txt="C#";
        break;
        case 2:
        txt="D";
        break;
        case 3:
        txt="D#";
        break;
        case 4:
        txt="E";
        break;
        case 5:
        txt="F";
        break;
        case 6:
        txt="F#";
        break;
        case 7:
        txt="G";
        break;
        case 8:
        txt="G#";
        break;
        case 9:
        txt="A";
        break;
        case 10:
        txt="A#";
        break;
        case 11:
        txt="B";
        break;
    }
    return txt + std::to_string( octave );
}
