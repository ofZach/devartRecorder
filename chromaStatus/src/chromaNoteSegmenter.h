



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
    
    
};



/*
 
 
 float endTime = vals[j].pos;
 float duration = endTime - goodStartTime;
 
 float otherAvgVolumes;
 float myAvgVolume;
 
 int nFrames = j - goodStartFrame;
 
 for (int k = goodStartFrame; k < j; k++){
 for (int l = 0; l < 12; l++){
 
 if (l == i){
 myAvgVolume += vals[k].vals[l];
 } else {
 otherAvgVolumes += vals[k].vals[l];
 }
 }
 }
 
 myAvgVolume /= (float)nFrames;
 otherAvgVolumes /= (float)(nFrames * 11);

 
 */




