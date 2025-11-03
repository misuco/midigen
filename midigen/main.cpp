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
    int len=4;
    int quantize=16;
    int density=16;

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

    // prepare chords lookup from json
    string chords_config_filename=datapath+"/chords.json";
    cout << "chords config file: " << chords_config_filename << endl;
    std::ifstream chords_config_file(chords_config_filename, std::ifstream::binary);
    Json::Value chords_config;
    chords_config_file >> chords_config;
    cout<< "chords config" << endl << chords_config << endl;

    // prepare instruments lookup from json
    string instruments_config_filename=datapath+"/instruments.json";
    cout << "instrument config file: " << instruments_config_filename << endl;
    std::ifstream instruments_config_file(instruments_config_filename, std::ifstream::binary);
    Json::Value instruments_config;
    instruments_config_file >> instruments_config;
    cout<< "instrument config" << endl << instruments_config << endl;

    /*
    parse generator config json:
    {
        "id":"0_140_1761813755890",
        "tempo":"140",
        "chords":"3",
        "instrument":"1",
        "quantize":"3",
        "density":"3",
        "sessionId":"de9514cf-efaa-4cc1-ba37-8170bbcc51ea"
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
    len = stoi(gen_config["len"].asString());
    quantize = stoi(gen_config["quantize"].asString());
    density = stoi(gen_config["density"].asString());

    // lookup instrument
    int instrumentId = stoi(gen_config["instrument"].asString());
    string instrument_sf2_file=instruments_config[instrumentId]["soundfont"].asString();
    Json::Value instrument_sf2_prog=instruments_config[instrumentId]["program"];
    cout << "intrument sf2 " << instrument_sf2_file << endl;

    // lookup chords
    int chordsId = stoi(gen_config["chords"].asString());
    string chords=chords_config[chordsId]["chords"].asString();
    cout << "intrument sf2 " << instrument_sf2_file << endl;

    // Init generator from config
    mg.setBPM( tempo );
    mg.setLen( len );
    mg.setQuantize( quantize );
    mg.setDensity( density );
    mg.setFilename( filename );

    // - instrument
    mg.setSoundfont( sf2path + "/" + instrument_sf2_file );
    for ( int index = 0; index < instrument_sf2_prog.size(); ++index ) {
        mg.addInstrument(instrument_sf2_prog[index].asInt());
    }

    // - chord
    std::stringstream chords_stream(chords);
    std::string chords_segment;
    while(std::getline(chords_stream, chords_segment, ' '))
    {
        mg.addChord(chords_segment);
        cout << "intrument sf2 " << instrument_sf2_file << endl;
    }

    // Generate composition
    mg.newMidiFile();
    cout << "create_file: " << filename << endl << "tempo: " << tempo << endl;

    // Save composition as files (Midi/Audio)
    mg.saveNewMidiFile();

    return 0;
}
