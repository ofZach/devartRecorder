//
//  chromaNoteSegmenter.cpp
//  chromaStatus
//
//  Created by zach on 5/20/14.
//
//

#include "chromaNoteSegmenter.h"
#include <stdlib.h>     /* qsort */
#include "testApp.h"


//--------------------------------------------------------------
int compareFloats(const void * elem1, const void * elem2){
    float f1 = *(float *) elem1;
    float f2 = *(float *) elem2;
    if(f1 == f2) return 0;
    if(f1 < f2) return -1;
    else return 1;
}

//--------------------------------------------------------------
inline float medianFilter ( vector < float > & samples ){
    
    if (samples.size() % 2 == 0) return 0;
    unsigned int medianFilterLength = samples.size();
    int middle = (medianFilterLength-1)/2; // check this is right?   9 012345678 should be pos 5 (4), right?  4+1
    int half = (medianFilterLength-1)/2;
    float samplesToSort[medianFilterLength];
    
    for (int i = 0; i < samples.size(); i++){
        samplesToSort[i] = samples[i];
    }
    qsort(samplesToSort, medianFilterLength, sizeof(float), compareFloats);
    return samplesToSort[middle];
}



void chromaNoteSegmenter::process (chormaData incomingVals){

    
    
    
    
    for (int j = 0; j < 12; j++){
        if (valsUnfiltered[j].size() > 0){
            valsUnfiltered[j].erase(valsUnfiltered[j].begin());
            
        }
        
        if (valsUnfilteredBig[j].size() > 0){
            valsUnfilteredBig[j].erase(valsUnfilteredBig[j].begin());
        }
        
        while (valsUnfiltered[j].size() < 3){
            valsUnfiltered[j].push_back(incomingVals.data[j]);
        }
        while (valsUnfilteredBig[j].size() < 4000){
            valsUnfilteredBig[j].push_back(incomingVals.data[j]);
        }
    }
    
    float maxVal = 0.001;
    for (int i = 0; i < 12; i++){
        for (int j = 0; j < valsUnfilteredBig[i].size(); j++){
            maxVal  = std::max(maxVal,valsUnfilteredBig[i][j]);
        }
    }
    
    //cout << valsUnfiltered[0].size();
    
    for (int i = 0; i < 12; i++){
        valsFiltered[i] = medianFilter(valsUnfiltered[i]);
    }
    
    for (int j = 0; j < 12; j++){
        vals.vals[j] = valsFiltered[j];
    }

    
    bool prevRecordingVals[12];
    
    for (int i = 0; i < 12; i++){
        
        prevRecordingVals[i] = bRecording[i];
        
        float val = vals.vals[i];
        float volume = powf(val / maxVal, 2.0);
        if (val <  0.4){
            bRecording[i] = false;
            } else{
            //cout << "recording i"  << " " << i << endl;
            bRecording[i] = true;
        }
        
        if (bRecording[i] == true && prevRecordingVals[i] == false){
            startRecordingFrame[i] = incomingVals.frame;
        } else if ( bRecording[i] == false && prevRecordingVals[i] == true){
            
            int endFrame = incomingVals.frame;
            int startFrame = startRecordingFrame[i];
            
            if (endFrame - startFrame > 5){
                recording r;
                r.startFrame = startFrame;
                r.endFrame = endFrame;
                r.note = i;
                recordings.push_back( r );
            }
            
            //cout << endFrame - startFrame << endl;
            
        }
        
    }
    
    if (bAnyRecording()){
        pastData.push_back(incomingVals);
    }

}

void chromaNoteSegmenter::processRecordingLogic(){
    
    testApp * app = (testApp*) ofGetAppPtr();
    
    
    
    for (int i = 0; i < recordings.size(); i++){
    
        chromaRecordingStats stats;
        
        
        int nFrames = 0;
        float myVolume = 0;
        float theirVolume = 0;
        for (int j = 0; j < pastData.size(); j++){
            if (pastData[j].frame >= recordings[i].startFrame && pastData[j].frame <= recordings[i].endFrame){
                
                
                for (int k=0; k<12; k++){
                    if (k == recordings[i].note){
                        myVolume += pastData[j].data[k];
                    } else {
                        theirVolume += pastData[j].data[k];
                    }
                }
                
                nFrames++;
                
            }
        }
        
        if (nFrames > 0){
            myVolume /= (float)nFrames;
            theirVolume /= (float)(11*nFrames);
            
        }
        
        ofLog(OF_LOG_NOTICE) << "recording strength " << myVolume;
        
        if (myVolume > 0.65){
        stats.myStrength = myVolume;
        stats.theirStrength = theirVolume;
        stats.duration = (recordings[i].endFrame -recordings[i].startFrame) * 2048.0 / 44100.0;
        
        ofLog(OF_LOG_NOTICE) << "outputting recording " << myVolume;
            
        //cout << myVolume << " " << theirVolume << endl;
        
        app->exportAudio(recordings[i].startFrame, recordings[i].endFrame, recordings[i].note, stats);
        }
        
    }
    
    recordings.clear();
    
    if (bAnyRecording() == false){
        pastData.clear();
    }
    
}


bool chromaNoteSegmenter::bAnyRecording(){
    bool bAny = false;
    for (int i = 0; i < 12; i++){
        if (bRecording[i] == true) bAny = true;
    }
    return bAny;
}

