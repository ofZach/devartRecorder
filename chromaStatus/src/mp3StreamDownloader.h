

#pragma once

#include "ofMain.h"


class curlThreaded;

class mp3StreamDownloader {

public:
    
    
    void setup();
    int getGotDataCount();
    void start(string url);
    void close();
    
    
};