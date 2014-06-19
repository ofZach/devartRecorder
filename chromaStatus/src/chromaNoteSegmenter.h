



#pragma once

#include "ofMain.h"
#include "chromaDataTypes.h"

typedef struct {
	float pos;
	float vals[12];
} timeVal;

typedef struct {
    int startFrame;
    int endFrame;
    int note;
} recording;


class chromaNoteSegmenter {
public:
    timeVal vals;
    vector < float > valsUnfiltered[12];
    vector < float > valsUnfilteredBig[12];
    float valsFiltered[12];
    void process (chormaData incomingVals);
    

    bool bRecording[12];
    int startRecordingFrame[12];
    
    chromaNoteSegmenter(){
        for (int i = 0; i < 12; i++){
            bRecording[i] = 0;
            startRecordingFrame[i] = 0;
        }
    }
    
    bool bAnyRecording();
    void processRecordingLogic();
    vector < recording > recordings;
    vector < chormaData > pastData;
    
    float recordingThreshold;
    float avgNoteStrengthThreshold;
    float recordingLengthTreshold;
};






