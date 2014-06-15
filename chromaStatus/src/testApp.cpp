#include "PluginInputDomainAdapter.h"
_VampHost::Vamp::HostExt::PluginInputDomainAdapter * pluginAdapter;


//#define VISUAL_OUTPUT
//#define AUDIO_OUTPUT


#include "testApp.h"
#include "curl/curl.h"
#include "mpg123.h"
#include "testApp.h"
#include "vamp-sdk.h"
#include "mpg123.h"
#include "vampUtils.h"
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>

// TODO:
// MAKE OUTUT FOLDERS if don't exist
// GITIGNORE for mp3 :)
//


float restartRate;

using Vamp::Plugin;
using Vamp::RealTime;

string radioLine;
string uniqueName;

//--------------------------------------------------------------
void testApp::setup(){
    
//    ofSeedRandom(ofGetMinutes() + ofGetSeconds());
//    int random = ofRandom(0,100000);
//    //cout << random << endl;
//    //ofSeedRandom(77);
    
    
    Poco::UUID uuid;
    uuid = Poco::UUIDGenerator::defaultGenerator().createRandom();
    vector < string > uuidSplit = ofSplitString(uuid.toString(), "-");
    uniqueName = uuidSplit[0];
    
    
    
    
    
    
    
    restartRate = ofRandom(5,10);
    
    
//    //----------------------------------------------------------------------
//    for (int i = 0; i < 12; i++){
//        
//        string dirName = "output/" + ofToString(i);
//        ofDirectory dir(dirName);
//        if (!dir.exists()){
//            dir.create();
//        }
//        
//        for (int j = 0; j < 10; j++){
//            
//            string dirName = "output/" + ofToString(i) + "/" + ofToString(j);
//            ofDirectory dir(dirName);
//            if (!dir.exists()){
//                dir.create();
//                string commandGit = "touch " + ofToDataPath(dirName) + "/.gitkeep &";
//                system(commandGit.c_str());
//            }
//            
//        }
//    }
//    
//    std::exit(0);
    
    
    ofBuffer radioStationsBuffer = ofBufferFromFile("newRadio2.txt");
    
    vector < string > radios;
    
    while (true){
        string line = radioStationsBuffer.getNextLine();
        radios.push_back(line);
        if (radioStationsBuffer.isLastLine()) break;
    }
    
    string url = "";
    
    while (true){
        radioLine = radios[ (int)ofRandom(0,radios.size()) % radios.size()];
        cout << radioLine << endl;
        vector < string >  split = ofSplitString(radioLine, "///", true);
        
        if (split[1].find("||") != string::npos){
            vector < string > urls = ofSplitString(split[1], "||");
            url = urls[ (int)ofRandom(0,urls.size() + 100) % urls.size() ];
        } else {
            url = split[1];
        }
        string fileName = ofToDataPath("curl/" + uniqueName + "_curlTest.mp3");
        string command = "curl -sS -o " + fileName + " -m 1 " + url;
        
        //cout << " hello " << endl;
        //cout << fileName << endl;
        system(command.c_str());
        //cout << " world " << endl;

        ofFile file (fileName);
        
        if (file.exists()){
            cout << "file size " << file.getSize()  << endl;
            if (file.getSize() > 1000){
                file.remove();
                break;
            } else {
                file.remove();
                cout << fileName << " too small " << endl;
                
                //std::exit(0);
            }
        } else {
            cout << fileName << " doesn't exist " << endl;
            //std::exit(0);
        }
        ofSleepMillis(500);
        
    }
    
    ofLogToFile("logs/" + uniqueName + ".log.txt");
    ofLog(OF_LOG_NOTICE) << radioLine;
    
    
    ofSetVerticalSync(false);
    
    plugin = new NNLSChroma(44100);
    plugin->setParameter("useNNLS", 1.0);
    plugin->setParameter("rollon", 0.0);
    plugin->setParameter("tuningmode", 0.0);
    plugin->setParameter("chromanormalize", 2.0);
    plugin->setParameter("whitening", 0.0);
    int blockSize = plugin->getPreferredBlockSize();
    int stepSize = plugin->getPreferredStepSize();
    pluginAdapter = new _VampHost::Vamp::HostExt::PluginInputDomainAdapter((Plugin*)plugin);
    pluginAdapter->initialise(1, stepSize, blockSize);
    Plugin::OutputList outputs = plugin->getOutputDescriptors();
    Plugin::OutputDescriptor od;
    int returnValue = 1;
    int progress = 0;
    int outputNo = 3;
    RealTime adjustment = RealTime::zeroTime;
    plugbuf = new float*[1];
    for (int c = 0; c < 1; ++c) plugbuf[c] = new float[blockSize + 2];
    
    
    #ifdef AUDIO_OUTPUT
        soundStream.setup(this, 2, 0, 44100, 512, 4);
    #endif
    
    #ifndef VISUAL_OUTPUT
        ofSetWindowShape(10,10);
    #endif
    
    
    MP3Stream.setup();
    MP3Stream.downloadUrl(url);
}


std::string exec(const char* cmd) {
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
}

int freq2midi(float freq) {
	return (int)( 69 + 12.0*log(freq/440.0)/log((float)2) );
}

int testApp::getMedianSpectralCentroid( string fileName ){
    //string command = "/usr/local/bin/sox '" + ofToDataPath(fileName) + "' -n stat 2>&1 | sed -n 's#^Rough [^0-9]*\\([0-9.]*\\)$#\\1#p' 2>&1 ";
#ifdef TARGET_OSX
    
    string command = "/usr/local/bin/sox '" + ofToDataPath(fileName) + "' -n stat 2>&1 | sed -n 's#^Rough [^0-9]*\\([0-9.]*\\)$#\\1#p' 2>&1 ";
#else 
    string command = "/usr/bin/sox '" + ofToDataPath(fileName) + "' -n stat 2>&1 | sed -n 's#^Rough [^0-9]*\\([0-9.]*\\)$#\\1#p' 2>&1 ";

#endif    

string result = exec(command.c_str());
    return freq2midi( ofToInt(result)/2) / 12;
}

void testApp::saveFloatBuffer(vector < float > & audioData, int sampleRate, int note, string fileName, chromaRecordingStats stats){
    
    
    Poco::UUID uuid;
    uuid = Poco::UUIDGenerator::defaultGenerator().createRandom();
    vector < string > uuidSplit = ofSplitString(uuid.toString(), "-");
    string uniqueFileName = uuidSplit[0];

    string tempFileName = "temp/" + uniqueName + "-" + uniqueFileName + ".mp3";
    
    
    //cout << "start of save float" << endl;
    
    
    
    int read, write;
    
    
    vector < string >  split = ofSplitString(radioLine, "///", true);
    
//    string url = "";
//    if (split[1].find("||") != string::npos){
//        vector < string > urls = ofSplitString(split[1], "||");
//        url = urls[ (int)ofRandom(0,urls.size() + 100) % urls.size() ];
//    } else {
//        url = split[1];
//    }
    
    
    FILE *mp3 = fopen( ofToDataPath(tempFileName).c_str(), "wb");
    
    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 8192;
    
    float pcm_buffer[PCM_SIZE];
    unsigned char mp3_buffer[MP3_SIZE];
    
    lame_t lame = lame_init();
    
    
    lame_set_in_samplerate(lame, sampleRate);
    lame_set_VBR(lame, vbr_default);
    
    string albumbName = ofToString(stats.myStrength) + "," + ofToString(stats.theirStrength) + "," + ofToString(stats.duration);
    
    
    id3tag_add_v2(lame);
    
    
    
    
    
	//id3tag_set_title(lame, "TITLE: any TItle");
	//id3tag_set_artist(lame, "who is the author?");
    
    
    string title =split[4] + "||" + split[5];
    
    id3tag_set_title(lame, title.c_str());
    id3tag_set_artist(lame, radioLine.c_str());
    id3tag_set_album(lame, albumbName.c_str());
	id3tag_set_comment(lame, ofGetTimestampString().c_str());
    
    
    
    
    lame_init_params(lame);

    int nChuncks = ceil(audioData.size() / PCM_SIZE) + 1;
    int pos = 0;
    
    
    for (int i = 0; i < nChuncks; i++){
        
        read = 0;
        
        for (int j = pos; j < pos + PCM_SIZE; j++){
            
            if (j >= audioData.size()){
                pcm_buffer[j-pos] = 0;
                read = 0;
            } else {
                pcm_buffer[j-pos] = audioData[j];
                read++;
            }
            
        }
        
        
        if (read == 0)
            write = lame_encode_flush_nogap(lame, mp3_buffer, MP3_SIZE);
        else{
            write = lame_encode_buffer_ieee_float(lame, pcm_buffer, pcm_buffer, read, mp3_buffer, MP3_SIZE);
            
            //cout << write << endl;
        }
        fwrite(mp3_buffer, write, 1, mp3);
        
        pos+=PCM_SIZE;
    }
    
    //lame_mp3_tags_fid(lame, mp3);
    lame_close(lame);
    fclose(mp3);
    
    
    cout << "mid of save float" << endl;
    
    // TODO:
    
    // SOX for octave splits...
    // SOX normalize  (MP3)  or use RMS and raw data...
    
    
    int octave = getMedianSpectralCentroid(tempFileName);
    
    
    //if (octave < 0) return; // ?
    
    
    // add note and octave to filename
    
    fileName = "output/" + ofToString(note) + "/" + ofToString(octave) + "/" + fileName;
    
    
    char soxMessage[512];
    

#ifdef TARGET_OSX
    sprintf(soxMessage, "/usr/local/bin/sox \"%s\" \"%s\" compand 0,0.1 -80.1,-inf,-80,-160,-48,-24,-24,-6,-12,-1.5,-6,-0.3 -1 -6 loudness",
            ofToDataPath(tempFileName).c_str(), ofToDataPath(fileName).c_str());
#else
    sprintf(soxMessage, "/usr/bin/sox \"%s\" \"%s\" compand 0,0.1 -80.1,-inf,-80,-160,-48,-24,-24,-6,-12,-1.5,-6,-0.3 -1 -6 loudness",
            ofToDataPath(tempFileName).c_str(), ofToDataPath(fileName).c_str());
#endif
    
    system(soxMessage);
    
    char id3cpMessage[512];
    
#ifdef TARGET_OSX
    sprintf(id3cpMessage, "/usr/local/bin/id3cp \"%s\" \"%s\"",
            ofToDataPath(tempFileName).c_str(), ofToDataPath(fileName).c_str());
#else
    sprintf(id3cpMessage, "/usr/bin/id3cp \"%s\" \"%s\"",
            ofToDataPath(tempFileName).c_str(), ofToDataPath(fileName).c_str());
#endif    
    system(id3cpMessage);


    string deleteCommand = "rm " + ofToDataPath(tempFileName);
    //system(deleteCommand.c_str());
    
    ofLog(OF_LOG_NOTICE) << "saved " << fileName <<  " " << tempFileName;
    
    
    //sox "$original_file" "$temp_file" compand 0,0.1 -80.1,-inf,-80,-160,-48,-24,-24,-6,-12,-1.5,-6,-0.3 -1 -6 loudness
    //id3cp "$original_file" "$temp_file"
    //mv "$temp_file" "$original_file"
    
    
    
    
    
    //string mvCommand = "mv " + ofToDataPath(tempFileName) + " " + ofToDataPath(fileName);
    //system(mvCommand.c_str());
    
    //cout << mvCommand << endl;
    
    
    //cout << "save " << fileName << endl;
    
    return;
    
    
    
    
    
    
}





int counter = 0;


//-------------------------------------------
typedef struct {
    int frameNum;
    float data[2048];
} frameAudio;
vector < frameAudio > audioRecording;


void testApp::exportAudio(int startFrame, int endFrame, int note, chromaRecordingStats stats ){
    
    
    vector < float > samplesForAudioRecording;
    
    
    
    
    for (int i = 0; i < audioRecording.size(); i++){
        
        if (audioRecording[i].frameNum >=startFrame &&
            audioRecording[i].frameNum <= endFrame){
            samplesForAudioRecording.insert(samplesForAudioRecording.end(), audioRecording[i].data, audioRecording[i].data + 2048);
            
            
            
        }
    }
    
    ofLog(OF_LOG_NOTICE) << "audio recoding size " << audioRecording.size();
    if (audioRecording.size() > 0){
        ofLog(OF_LOG_NOTICE) << "audio recording a " << audioRecording[0].frameNum << " b " << audioRecording[audioRecording.size()-1].frameNum;
        
    }
    ofLog(OF_LOG_NOTICE) << "start frame end frame duration " << startFrame << " " << endFrame << " " << samplesForAudioRecording.size() <<endl;
    
    //cout << samplesForAudioRecording.size() << endl;
    //cout << samplesForAudioRecording.size() << endl;
    if (samplesForAudioRecording.size() > 2048 * 8) saveFloatBuffer(samplesForAudioRecording, 44100, note, uniqueName + "-" + ofToString(startFrame) + ".mp3", stats);
    
    samplesForAudioRecording.clear();
    //audioRecording.clear();
    
    
}



int nFramesNonRecording = 0;
void testApp::getAudioData(float * audioData, int nSamples){
   
    
    // get processing and audio recoring synchronized
    
    for (int i = 0; i < nSamples; i++){
        samples.push_back(audioData[i]);
    }
  
    
    vector < chormaData > chromaDataTemps;
    
    
    while (samples.size() > 16384){
        for (int j = 0; j < 16384; j++){
            plugbuf[0][j] = samples[j];
        }
        
        
        samples.erase(samples.begin(), samples.begin()+2048);
        RealTime rt;
        rt = RealTime::frame2RealTime(counter * 2048, 44100);
        RealTime::frame2RealTime(counter * 2048, 44100);
        pluginAdapter->process(plugbuf, rt);
        if (counter % 1 == 0){
            Plugin::FeatureSet features = plugin->getRemainingFeatures();
            for (int i = 0; i < features[3].size(); i++){
                chormaData data;
                for (unsigned int j = 0; j < features[3][i].values.size(); ++j) {
                    data.data[j] =features[3][i].values[j] ;
                }
                data.frame = counter;
                segmenter.process(data);
                for (int j = 0; j < 12; j++){
                   data.data[j] = segmenter.vals.vals[j];
                }
                chromaDatas.push_back(data);
                if (chromaDatas.size() > 150){
                    chromaDatas.erase(chromaDatas.begin());
                }
                
                // deal with record or non record now
                
                bool bShouldIRecord = segmenter.bAnyRecording();
                if (bShouldIRecord == true){
                    nFramesNonRecording = 0;
                    int mid = 16384/8;
                    int start = mid - 2048/2;
                    int end = start + 2048;
                    frameAudio FA;
                    FA.frameNum = counter;
                    for (int i = start; i < end; i++){
                        FA.data[i-start] = plugbuf[0][i];
                    }
                    audioRecording.push_back(FA);
                } else {
                    nFramesNonRecording ++;
                    if (nFramesNonRecording > 3){
                        audioRecording.clear();
                    }
                 
                }
                
                segmenter.processRecordingLogic();
            }
            plugin->reset();
        }
        counter++;
        
    }
#ifdef AUDIO_OUTPUT
    // todo: scoped lock here!
    for (int i = 0; i < nSamples; i++){
        values.push_back(audioData[i]);
    }
    
#endif 
    
    chromaDataTemps.clear();
    
    
    
}



//--------------------------------------------------------------
void testApp::audioOut(float * output, int bufferSize, int nChannels){
	
    
#ifdef AUDIO_OUTPUT
    float vals[bufferSize];
    memset(vals, 0, bufferSize*4);
    {
        
        ofMutex::ScopedLock Lock( lock );
        int nSamples = MIN(values.size(),bufferSize);
        for (int i = 0; i < nSamples; i++){
            vals[i] = values[i];
        }

        values.erase(values.begin(), values.begin()+ nSamples );
    }
    
    
	if ( true){
		// ---------------------- noise --------------
		for (int i = 0; i < bufferSize; i++){
            output[i*nChannels    ] = vals[i] * 0.1;
			output[i*nChannels + 1] = vals[i] * 0.1;
		}
	}
    #endif
}



int lastDataCount = 0;
int consecutiveZeroCount = 0;
//--------------------------------------------------------------
void testApp::update(){
    

    
    int currentDataCount = MP3Stream.getGotDataCount();
    
    if (currentDataCount == 0) consecutiveZeroCount++;
    else consecutiveZeroCount = 0;
    
    ofLog(OF_LOG_NOTICE) << "update at " << ofGetTimestampString() << " data count " << currentDataCount - lastDataCount;
    lastDataCount = currentDataCount;
    
    
    
    if (consecutiveZeroCount > 6){
        MP3Stream.shutDown();
        ofSleepMillis(30); // cleanup;
        ofFile file("logs/" + uniqueName + ".log.txt");
        file.remove();
        //ofLogToFile("logs/" + uniqueName + ".log.txt");
               
        std::exit(0);
    }
 
    // quit out after 1.5 mins or so...
    if (ofGetElapsedTimef() > 60){
        MP3Stream.shutDown();
        ofFile file("logs/" + uniqueName + ".log.txt");
        file.remove();
	ofSleepMillis(30); // cleanup;
        std::exit(0);
    }
    
    
    
    //ofSetFrameRate(30);
    //cout << ofGetFrameRate() << endl;
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
    
#ifdef VISUAL_OUTPUT
    vector < chormaData > datas;
    
    {
        ofMutex::ScopedLock Lock( lock3 );
        for (int i = 0; i < chromaDatas.size(); i++){
            datas.push_back(chromaDatas[i]);
        }

    }
    
    //cout << chromaDatas.size() << endl;
    
    if (datas.size() > 0){
        
        //chormaData temp = chromaDatas[chromaDatas.size()-1];
    
        
        for (int i = 0; i < datas.size(); i++){
        for (int j = 0; j < 12; j++){
            
            float w = ofGetWidth() / (float)datas.size();
            
            ofColor tempColor;
            tempColor.setHsb(  ofMap(datas[i].data[j], 0, 1, 100,0), 255, 255);
            
            ofFill();
            ofSetColor(tempColor);
            ofRect(i * w, j * ofGetHeight()/12., w,ofGetHeight()/12. );
            
            //ofNoFill();
            //ofSetColor(0);
            //ofRect(i * w, j * ofGetHeight()/12., w,ofGetHeight()/12. );
            
        }
        
       }
        
        
    }
    
    
    
    
    for (int j = 0; j < 12; j++){
        
        
        if (segmenter.bRecording[j]){
        ofNoFill();
        ofSetColor(255);
        ofRect(0, j * ofGetHeight()/12., ofGetWidth(),ofGetHeight()/12. );
        }
        //ofNoFill();
        //ofSetColor(0);
        //ofRect(i * w, j * ofGetHeight()/12., w,ofGetHeight()/12. );
        
    }
#else
    ofSetFrameRate(1);
    
#endif
    
    
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    if (key == ' ' ){
        ofMutex::ScopedLock Lock( lock );
        values.clear();
    }
//    if (key == ' '){
//        ofSleepMillis(5000);
//    }
    
    //CT->stop();
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
    
}

