/**
* ApolloqA
*
* Piano music composition generator
*
* (c) 2025 by claudio zopfi
*
* Licence: GNU/GPL
*
* https://github.com/misuco/apolloqa
*
*/

#include <stdlib.h>
#include <sstream>
#include <unistd.h>
//#include <json/value.h>
#include <jsoncpp/json/json.h>
#include "midigen.hpp"

Midigen mg;
map<int, int> quarterHistogram;

inline bool file_exists (const string& name) {
    ifstream f(name.c_str());
    return f.good();
}

int main(int argc, char *argv[])
{
    string filename="midigen";
    string sf2path=".";
    string datapath=".";
    int tempo=140;
    int beatsPerChord=4;
    int quantize=16;
    string steps="00000000";

    int c;
    while ((c = getopt (argc, argv, "d:f:s:")) != -1) {
        switch (c)
        {
            case 'd':
                datapath = optarg;
                break;
            case 'f':
                filename = optarg;
                break;
            case 's':
                sf2path = optarg;
                break;
            default:
                abort ();
        }
    }

    /*
    parse generator config json:
    {
        "id":"0_120_1767436140840",
        "tempo":"120",
        "chords":"E_A_G_C",
        "sf2file":"Among Us.sf2",
        "presetNr":"7",
        "presetName":"EjectSplash",
        "presetBank":"0",
        "beatsPerChord":"8",
        "quantize":"16",
        "steps":"00000000",
        "sessionId":"3fbcfd97-d2a4-4ee4-be24-8fe9664d2c9d"
    }
    */

    string gen_config_filename=filename+".json";
    cout << "generator config file: " << gen_config_filename << endl;
    std::ifstream gen_config_file(gen_config_filename, std::ifstream::binary);
    Json::Value gen_config;
    gen_config_file >> gen_config;
    cout<< "generator config" << endl << gen_config << endl;

    // set direct parameters
    tempo = stoi(gen_config["tempo"].asString());
    beatsPerChord = stoi(gen_config["beatsPerChord"].asString());
    quantize = stoi(gen_config["quantize"].asString());
    //density = stoi(gen_config["density"].asString());
    steps = gen_config["steps"].asString();

    // lookup chords
    string chords = gen_config["chords"].asString();

    // Init generator from config
    mg.setBPM( tempo );
    mg.setBeatsPerChord( beatsPerChord );
    mg.setQuantize( quantize );
    //mg.setDensity( density );
    mg.setSteps( steps );
    mg.setFilename( filename );

    // - instrument
    mg.setSoundfont( sf2path + "/" + gen_config["sf2file"].asString() );
    mg.setInstrumentPreset( stoi(gen_config["presetNr"].asString()) );
    mg.setInstrumentBank( stoi(gen_config["presetBank"].asString()) );

    // - chord
    std::stringstream chords_stream(chords);
    std::string chords_segment;
    while(std::getline(chords_stream, chords_segment, '_'))
    {
        mg.addChord(chords_segment);
        cout << "chord " << chords_segment << endl;
    }

    // Generate composition
    mg.newMidiFile();
    cout << "create_file: " << filename << endl << "tempo: " << tempo << endl;

    // Save composition as files (Midi/Audio)
    mg.saveNewMidiFile();

    return 0;
}
