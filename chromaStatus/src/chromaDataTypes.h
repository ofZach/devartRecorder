//
//  chromaDataTypes.h
//  chromaStatus
//
//  Created by zach on 5/20/14.
//
//

#ifndef chromaStatus_chromaDataTypes_h
#define chromaStatus_chromaDataTypes_h


typedef struct {
    int frame;
    float data[12];
} chormaData;


typedef struct {
    float myStrength;
    float theirStrength;
    float duration;
} chromaRecordingStats;




#endif
