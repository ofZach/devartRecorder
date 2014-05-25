

#pragma once

#include "ofMain.h"



class mp3StreamDownloader {

public:
    
    void setup();
    void downloadUrl(string url);
    ~mp3StreamDownloader();
    
    int getGotDataCount();
    void shutDown();
    
    
};