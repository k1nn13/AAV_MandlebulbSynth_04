#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxMaxim.h"
#include "ofxGui.h"





class ofApp : public ofBaseApp, public ofxMidiListener {

	public:
		void setup();
		void update();
		void draw();
		//-------------------MAXIM---------------------------------//
		void audioOut(float * output, int bufferSize, int nChannels);

		//-------------------MIDI----------------------------------//
		void newMidiMessage(ofxMidiMessage &message);
		void midiDebug();
		void exit();


		ofVec3f spherical(float x, float y, float z);

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		//---------------------------------------------------------//


		//--SETUP-MIDI--//
		ofxMidiIn midi;
		vector <ofxMidiMessage> messages;
		vector <int> midiNotes;


		//--SETUP-SOUND--//
		ofSoundStream soundStream;
		unsigned bufferSize, sampleRate;
		double currentSample, mix, outputs[2];
		ofxMaxiFFT fft;
		ofxMaxiFFTOctaveAnalyzer oct;
		maxiMix masterMix;
		convert mtof;				   //convert midi notes to frequency
		int midiTrigger, midiNote;

		//--GUI--//
		ofxPanel gui;
		ofParameter<float> masterGainSlider;
		ofxVec4Slider adsrSlider;



		//--SYNTHESIS--//
		maxiOsc osc[1];
		maxiOsc lfo[1];
		maxiEnv env[1];
		maxiFilter filter[1];


		maxiDelayline delay[1];

		double oscOut[1];
		double lfoOut[1];
		double envOut[1];
		double pitch[1]; 
		double gain[1];
		double delayOut[1];
	

		//--DRAW-WAVEFROM--//
		vector <float> volHistory;
		float smoothedVol;
		float scaledVol;
		float soundBuffer[512];

		//mandlebulb
		ofEasyCam cam;
		int DIM = 128;
		ofMesh mesh;

};
