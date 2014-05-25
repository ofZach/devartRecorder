#include "ofMain.h"
#include "testApp.h"
#include "ofAppNoWindow.h"

//#define VISUAL_OUTPUT
//#define AUDIO_OUTPUT


//========================================================================
int main( ){
    
#ifdef VISUAL_OUTPUT
	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new testApp());
#else
    ofAppNoWindow window;
    ofSetupOpenGL(&window,1024,768,OF_WINDOW);
    ofRunApp(new testApp);
    
    
#endif
    
}
