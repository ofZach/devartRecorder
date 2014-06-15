//
//  mp3StreamDownloader.cpp
//  chromaStatus
//
//  Created by zach on 5/17/14.
//
//


#include "mp3StreamDownloader.h"
#include "curl/curl.h"
#include "mpg123.h"
#include "ofMain.h"
#include "testApp.h"



mpg123_handle *mh = NULL;
bool bAbort = false;
int gotDataCount = 0;

int mp3StreamDownloader::getGotDataCount(){
    return gotDataCount;
}


int currentTime = 0;
int count = 0;
int previousTime = 0;
int nSamples = 0;

void calculateFPS(){
    
    currentTime = ofGetElapsedTimeMillis();
    
    //  Calculate time passed
    int timeInterval = currentTime - previousTime;
    
string timestamp = ofGetTimestampString();
    
    if(timeInterval > 1000){
        //  calculate the number of frames per second
        float sampleFPS = nSamples / (float)(timeInterval / 1000.0);

        ofLog(OF_LOG_NOTICE) << timestamp << " - " << sampleFPS << " samples per second ";
        //  Set time
        previousTime = currentTime;
        nSamples = 0;
    }
}
//-------------------------------------------------------------------------------





//-----------------------------------------------------------------------------------------
size_t play_stream(void *buffer, size_t size, size_t nmemb, void *userp)
{
    
    
    gotDataCount++;
    
//  calculateFps();
  calculateFPS();

    ofLog(OF_LOG_NOTICE) << "received data, size is " << size * nmemb;
//cout << "got data" << ofGetTimestampString() << endl;
    int err;
    off_t frame_offset;
    unsigned char *audio;
    size_t done;
    int channels, encoding;
    long rate;
    
    float * values = NULL;
    
    //cout << "." << endl;
    
    mpg123_feed(mh, (const unsigned char*) buffer, size * nmemb);
    
    do {
        err = mpg123_decode_frame(mh, &frame_offset, &audio, &done);
        switch(err) {
            case MPG123_NEW_FORMAT:
                mpg123_getformat(mh, &rate, &channels, &encoding);
                
//                cout << "got a new format" << endl;
//                cout << "bits " << mpg123_encsize(encoding) << endl;
//                cout << "rate " << rate << endl;
//                cout << "format.channels " << channels << endl;
                
                // TODO: we need a check here...
                // if we are not mono, 44100  and float, we are in trouble & let's abort !!!!!
                
                break;
                
            case MPG123_OK:
                
                // TODO: THIS CAN BE OPTIMIZED  ( no create delete!)
                // only reallocate on new stuff.
                
                //printf("got chunk \n");
                if (values == NULL) values  = new float[done / 4];
                memcpy((char *)values, audio, done);
                ((testApp *) ofGetAppPtr())->getAudioData(values, done/4);
                nSamples += done/4;
                //delete [] values;
                break;
            case MPG123_NEED_MORE:
                break;
            default:
                break;
        }
    } while(done > 0);
    
    // this kills curl ... do we need a delay before cleanup?
    if (bAbort == false){
        return size * nmemb;
    } else {
        bAbort = false;
        return -1;
    }
}



class curlThreaded : public ofThread{
    
public:
    
    CURLM* multi_handle;
    CURL *curl;
    string url;
    int handle_count;
    
    void setUrl(string urlToLoad){
        url = urlToLoad;
    }
    //--------------------------
    curlThreaded(){
        curl = NULL;
        handle_count = 0;
    }
    
    void start(){
        
        startThread(true, false);   // blocking, verbose
    }
    
    void stop(){
        stopThread();
        bAbort = true;
        curl_easy_cleanup(curl);
        
    }
    
    //--------------------------
    void threadedFunction(){
        
        bAbort = false;
        curl = curl_easy_init();
        //multi_handle = curl_multi_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, play_stream);
        curl_easy_setopt(curl, CURLOPT_BUFFERSIZE,1152*30);                 // <----- this is adjustable.  worth experimenting with
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        //curl_multi_add_handle(multi_handle, curl);
        
        while( isThreadRunning() != 0 ){
            curl_easy_perform(curl);
            ofSleepMillis(ofRandom(80,120));                                              // <----- this is adjustable.  worth experimenting with
            calculateFPS();
        }
        
    }
    
    
};

curlThreaded * CT;


void mp3StreamDownloader::shutDown(){
    //CT->stop();
    //delete CT;
}



void mp3StreamDownloader::setup(){
    mpg123_init();
    mh = mpg123_new(NULL, NULL);
    mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_MONO_MIX, 0);         // mono!
    mpg123_param(mh, MPG123_FORCE_RATE, 44100, 0);                  // 44100
    mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT, 0);      // float
    mpg123_open_feed(mh);
    CT = new curlThreaded();
}

void mp3StreamDownloader::downloadUrl(string url){
    CT->setUrl(url);
    CT->start();
}


mp3StreamDownloader::~mp3StreamDownloader(){
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
}





