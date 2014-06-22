#include "PluginInputDomainAdapter.h"
_VampHost::Vamp::HostExt::PluginInputDomainAdapter * pluginAdapter;



#include "testApp.h"
#include "curl/curl.h"
#include "mpg123.h"
#include "testApp.h"
#include "vamp-sdk.h"
#include "mpg123.h"
#include "vampUtils.h"
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>



float restartRate;
using Vamp::Plugin;
using Vamp::RealTime;
string radioLine;
string uniqueName;

void testApp::loadSettings(){
    
    ofxXmlSettings settings;
    settings.load("settings/settings.xml");
    
    segmenter.recordingThreshold                  = settings.getValue("noteThreshold:recordingThreshold", 0.31);
    segmenter.avgNoteStrengthThreshold            = settings.getValue("noteThreshold:avgNoteStrengthThreshold", 0.79);
    segmenter.recordingLengthTreshold             = settings.getValue("noteThreshold:recordingLengthTreshold", 0.51);
    
    
    //cout << recordingThreshold << " " << avgNoteStrengthThreshold << " " << " " << recordingLengthTreshold << endl;
    
    
    //    <noteThreshold>
    //	<recordingThreshold>0.3</recordingThreshold>
    //	<avgNoteStrengthThreshold>0.8</avgNoteStrengthThreshold>
    //	<recordingLengthTreshold>0.5</recordingLengthTreshold>
    //    </noteThreshold>
    
    

}



float earthDistance ( float lon1, float lat1, float lon2, float lat2 ){
    float R = 6371; // km
    float lat1rad = lat1 * DEG_TO_RAD;
    float lat2rad = lat2 * DEG_TO_RAD;
    float latDiffRad = (lat2-lat1) * DEG_TO_RAD;
    float lonDiffRad = (lon2-lon1) * DEG_TO_RAD;
    float a = sin(latDiffRad/2) * sin(latDiffRad/2) +
    cos(lat1rad) * cos(lat2rad) *
    sin(lonDiffRad/2) * sin(lonDiffRad/2);
    float c = 2 * atan2(sqrt(a), sqrt(1-a));
    float d = R * c;
    return d;
}


void testApp::parseRadioStations(){
    
    ofBuffer radioStationsBuffer = ofBufferFromFile("newRadio2.txt");
    
    ofLogToFile("logs/" + uniqueName + ".log.txt");
    
    vector < string > radios;
    
    while (true){
        string line = radioStationsBuffer.getNextLine();
        
        
        vector < string >  split = ofSplitString(line, "///", true);
        
        if (split.size() == 6){
            //cout << split[4] << " " << split[5] << endl;
            
//            class radioStation{
//                float longitude;    // EW       -180-180
//                float latitude;     // NS       -90-90
//                string radioStationString;
//                string radioUrl;
//            };
//
            radioStation RS;
            RS.longitude    = ofToFloat(split[4]);
            RS.latitude     = ofToFloat(split[5]);
            
            if (split[1].find("||") != string::npos){
                vector < string > urls = ofSplitString(split[1], "||");
                RS.url = urls[ (int)ofRandom(0,urls.size() + 100) % urls.size() ];
            } else {
                RS.url = split[1];
            }
            RS.radioStationString = line;
            stations.push_back(RS);
            
        }

        
        if (radioStationsBuffer.isLastLine()) break;
    }
    //vector < radioStation > stations;

}

ofPoint projection( float latitude, float longitude, ofRectangle mapTo){
    
    float mapWidth = mapTo.width;
    float mapHeight = mapTo.height;
    
    // get x value
    float x =  (mapWidth * (180+longitude) / 360.0);
    float latRad = latitude*PI/180;
    
    // get y value
    float mercN = log(tan((PI/4.0)+(latRad/2)));
    float y     = (mapHeight/2.0)-(mapWidth*mercN/(2*PI));
    ofPoint pt(x,y);
    return pt;
    
}

//--------------------------------------------------------------
void testApp::setup(){
    
    parseRadioStations();
    
    loadSettings();
    
    img.setUseTexture(false);
    img.loadImage("map/map.png");
    
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
    
    
    initSystem();
    MP3Stream.setup();
    
    
}


void testApp::initSystem(){
    
    
    loadSettings();
    
    /// ? plugin->reset();
    
    samples.clear();
    chromaDatas.clear();
    values.clear();
    
    
    Poco::UUID uuid;
    uuid = Poco::UUIDGenerator::defaultGenerator().createRandom();
    vector < string > uuidSplit = ofSplitString(uuid.toString(), "-");
    uniqueName = uuidSplit[0];
    
    
    restartRate = ofRandom(5,10);
    
    
    ofBuffer radioStationsBuffer = ofBufferFromFile("newRadio2.txt");
    
    ofLogToFile("logs/" + uniqueName + ".log.txt");
    
    vector < string > radios;
    
    while (true){
        string line = radioStationsBuffer.getNextLine();
        radios.push_back(line);
        if (radioStationsBuffer.isLastLine()) break;
    }
    
    string url = "";
    
    while (true){
        
//        
//        class radioStation{
//            
//        public:
//            
//            float longitude;    // EW       -180-180
//            float latitude;     // NS       -90-90
        float randomLon = ofRandom(-180,180);
        float randomLat = ofRandom(-90,90);
        
        while (true){
            
            ofPoint pt = projection(randomLat, randomLon, ofRectangle(0,0, img.getWidth(), img.getHeight()));
            int alpha = img.getColor(pt.x, pt.y).a;
            if (alpha > 127){
                break;
            } else {
                randomLon = ofRandom(-180,180);
                randomLat = ofRandom(-90,90);

            }
            
        }
        
        

        float minDistance = 10000000000;
        int minIndex = -1;
        
        for (int i = 0; i < stations.size(); i++){
            float latR = stations[i].latitude + ofRandom(-0.4, 0.4);
            float lonR = stations[i].longitude + ofRandom(-0.4, 0.4);
            float dist = earthDistance(randomLon, randomLat, lonR, latR);
            
            if (dist < minDistance ){
                minDistance = dist;
                minIndex = i;
            }
        }
        
        if (minIndex != -1){
            
            cout << minIndex << endl;
            cout << stations[minIndex].radioStationString << endl;
            
            radioLine = stations[minIndex].radioStationString;
            
            //radios[ (int)ofRandom(0,radios.size()) % radios.size()];
            
            ofLog(OF_LOG_NOTICE) << radioLine;
            
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
            system(command.c_str());
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

            
            
            
        } else {
            //continue;
        }
        
        
        
        
               //ofSleepMillis(500);
        
    }
    
    
    
    ofSetVerticalSync(false);


    startRecordTime = ofGetElapsedTimeMillis();;
    
    MP3Stream.start(url);
    
    
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




int lastDataCount = 0;
int consecutiveZeroCount = 0;
//--------------------------------------------------------------
void testApp::update(){
    
    int currentDataCount = MP3Stream.getGotDataCount();
    if (currentDataCount == 0) consecutiveZeroCount++;
    
    else consecutiveZeroCount = 0;
    ofLog(OF_LOG_NOTICE) << "update at " << ofGetTimestampString() << " data count " << currentDataCount - lastDataCount;
    lastDataCount = currentDataCount;
    

    
    long long currentTime = ofGetElapsedTimeMillis();;
    long long diffTime =  currentTime - startRecordTime;
    float timeDiffSecs = diffTime / 1000.0;
    
    
    
    if (consecutiveZeroCount > 6){
MP3Stream.close();
        ofSleepMillis(30); // cleanup;
        ofFile file("logs/" + uniqueName + ".log.txt");
        file.remove();
        //ofLogToFile("logs/" + uniqueName + ".log.txt");
        
        initSystem();
    }
 
    // quit out after 1.5 mins or so...
    if (timeDiffSecs > 25){
MP3Stream.close();
        ofFile file("logs/" + uniqueName + ".log.txt");
        file.remove();
        
        ofSleepMillis(30); // cleanup;
        initSystem();
    }
    
    
    
    //ofSetFrameRate(30);
    //cout << ofGetFrameRate() << endl;
    
}

//--------------------------------------------------------------
void testApp::draw(){
    ofSetFrameRate(1);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    

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

