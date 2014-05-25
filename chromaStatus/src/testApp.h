#pragma once

#include "ofMain.h"
#include "NNLSChroma.h"
#include "mp3StreamDownloader.h"
#include "chromaNoteSegmenter.h"
#include "chromaDataTypes.h"
#include "lame.h"


class radioInfo {
    string radioStationName;
    string cityName;
    string countryName;
    float longitude;
    float latitude;
};




class testApp : public ofBaseApp{

	public:
    
    
		void setup();
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
        void audioOut(float * output, int bufferSize, int nChannels);
        void exportAudio(int startFrame, int endFrame, int note, chromaRecordingStats stats);
        void saveFloatBuffer(vector < float > & audioData, int sampleRate, string fileName, chromaRecordingStats stats);
    
        mp3StreamDownloader MP3Stream;
        ofMutex lock3;
        ofMutex lock2;
        ofMutex lock;
        vector < float > values;
        ofSoundStream soundStream;
        void exit(){};


        NNLSChroma * plugin;
        float blockFloatArray[ 16384 ];
        float **plugbuf;
        vector < float > samples;
        vector < chormaData > chromaDatas;
        chromaNoteSegmenter segmenter;
    
    
        radioInfo RI;
    
    

};
