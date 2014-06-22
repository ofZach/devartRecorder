#pragma once

#include "ofMain.h"
#include "NNLSChroma.h"
#include "mp3StreamDownloader.h"
#include "chromaNoteSegmenter.h"
#include "chromaDataTypes.h"
#include "lame.h"
#include "ofxXmlSettings.h"


class radioStation{

public:
    
    float longitude;    // EW       -180-180
    float latitude;     // NS       -90-90
    string radioStationString;
    string url;
};


class testApp : public ofBaseApp{

	public:
    
    
        ofImage img;
    
    
		void setup();
    
        void parseRadioStations();
        vector < radioStation > stations;
    
        void initSystem();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    
    
		
        void getAudioData(float * audioData, int nSamples);
        void exportAudio(int startFrame, int endFrame, int note, chromaRecordingStats stats);
        void saveFloatBuffer(vector < float > & audioData, int sampleRate, int note, string fileName, chromaRecordingStats stats);

        // todo: sort me up
        mp3StreamDownloader MP3Stream;
        vector < float > values;
        ofSoundStream soundStream;
        NNLSChroma * plugin;
        float blockFloatArray[ 16384 ];
        float **plugbuf;
        vector < float > samples;
        vector < chormaData > chromaDatas;
        chromaNoteSegmenter segmenter;
        int getMedianSpectralCentroid( string fileName );
        long long startRecordTime;
    
        // settings related:
        void loadSettings();
        float recordingThreshold;
        float avgNoteStrengthThreshold;
        float recordingLengthTreshold;

    

};
