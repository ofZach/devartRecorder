
#include "testApp.h"
#include "vamp-sdk.h"


using Vamp::Plugin;
using Vamp::RealTime;


void
printFeatures(int frame, int sr, int output, Plugin::FeatureSet features, ofstream *out, bool useFrames)
{
    for (unsigned int i = 0; i < features[output].size(); ++i) {
        
        if (useFrames) {
            
            int displayFrame = frame;
            
            if (features[output][i].hasTimestamp) {
                displayFrame = RealTime::realTime2Frame
                (features[output][i].timestamp, sr);
            }
            
            (out ? *out : cout) << displayFrame;
            
            if (features[output][i].hasDuration) {
                displayFrame = RealTime::realTime2Frame
                (features[output][i].duration, sr);
                (out ? *out : cout) << "," << displayFrame;
            }
            
            (out ? *out : cout)  << ":";
            
        } else {
            
            RealTime rt = RealTime::frame2RealTime(frame, sr);
            
            if (features[output][i].hasTimestamp) {
                rt = features[output][i].timestamp;
            }
            
            (out ? *out : cout) << rt.toString();
            
            if (features[output][i].hasDuration) {
                rt = features[output][i].duration;
                (out ? *out : cout) << "," << rt.toString();
            }
            
            (out ? *out : cout) << ":";
        }
        
        for (unsigned int j = 0; j < features[output][i].values.size(); ++j) {
            (out ? *out : cout) << " " << features[output][i].values[j];
        }
        (out ? *out : cout) << " " << features[output][i].label;
        
        (out ? *out : cout) << endl;
    }
}