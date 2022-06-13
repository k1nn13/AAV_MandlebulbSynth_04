#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	//-----OF-SETUP-------//
	ofSetFrameRate(60);
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	ofEnableSmoothing();
	mesh.setMode(OF_PRIMITIVE_POINTS);
	ofDisableArbTex();
	// load an image from disk



	//-------------------GUI------------------------------//


	gui.setup();
	gui.add(masterGainSlider.set("master gain", 0.4, 0.01, 1));
	gui.add(adsrSlider.setup("ADSR", ofVec4f(1000, 5, 5, 1000), ofVec4f(0, 0, 0, 0), ofVec4f(50000, 5000, 5000, 20000)));
	

	//--MIDI-SETUP--//
	midi.listInPorts();			  //display midi instrument name in terminal
	midi.openPort("ReMOTE SL 0"); //select midi port string or int
	midi.addListener(this);

	//---MAXIM-SETUP---//
	fft.setup(1024, 512, 256);
	oct.setup(44100, 1024, 10);
	sampleRate = 44100;
	bufferSize = 512;

	//--SoundStream-Settings--//
	ofSoundStreamListDevices();
	ofSoundStreamSettings settings;
	settings.setApi(ofSoundDevice::MS_ASIO);  //select sound card
	ofSoundStreamSetup(2, 2, this, sampleRate, bufferSize, 4);
	ofxMaxiSettings::setup(sampleRate, 2, bufferSize);
	//=======================================================//



	for (int i = 0; i < DIM; i++) {
		for (int j = 0; j < DIM; j++) {
			for (int k = 0; k < DIM; k++) {
			
				bool edge = false;


				float x = ofMap(i, 0, DIM, -1, 1);
				float y = ofMap(j, 0, DIM, -1, 1);
				float z = ofMap(k, 0, DIM, -1, 1);

				ofVec3f zeta = ofVec3f(0, 0, 0);
				int n = 16;
				int maxIterations = 32;
				int iteration = 0;

				while (true) {

					ofVec3f sphericalZ = spherical(zeta.x, zeta.y, zeta.z);

					float newx = pow(sphericalZ.x, n) * sin(sphericalZ.y*n) * cos(sphericalZ.z*n);
					float newy = pow(sphericalZ.x, n) * sin(sphericalZ.y*n) * sin(sphericalZ.z*n);
					float newz = pow(sphericalZ.x, n) * cos(sphericalZ.y*n);

					zeta.x = newx + x;
					zeta.y = newx + y;
					zeta.z = newx + z;

					iteration++;

					if (sphericalZ.x > maxIterations) {
						if (edge) {
							edge = false;
							mesh.addColor(ofColor(0));
						}
						break;
					}

					if (iteration > maxIterations) {

						if (!edge) {
							edge = true;
							ofVec3f pos = ofVec3f(x * 100, y * 100, z * 100);
							mesh.addColor(ofColor(i*2, i*2, i*10, i*j));
							mesh.addVertex(pos);
						}

						break; 
					}
				}

	
			}
		}
	}

	ofEnableDepthTest();
	//glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	glPointSize(5); // make the points bigger

}


//--------------------------------------------------------------
void ofApp::update(){


	//--ENVELOPES--//
	env[0].setAttack(  adsrSlider->x );
	env[0].setDecay(   adsrSlider->y );
	env[0].setSustain( adsrSlider->z );
	env[0].setRelease( adsrSlider->w );

}

//--------------------------------------------------------------
ofVec3f ofApp::spherical(float x, float y, float z) {
	//convert to sperical coordinates
	float r = glm::sqrt(x*x + y * y + z * z);
	float theta = glm::atan(glm::sqrt(x*x + y * y), z);
	float phi = glm::atan(y, x);

	return ofVec3f(r, theta, phi);
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(0);
	midiDebug();

	ofEnableDepthTest(); 
	ofEnableAlphaBlending();

	cam.begin();



	float scalar = 500;
	ofScale(1 + scalar * envOut[0], 1 + scalar * envOut[0], 1 + scalar * envOut[0]); 
	ofRotateXDeg(ofGetElapsedTimef() * 5);
	ofRotateYDeg(ofGetElapsedTimef() * 8);
	ofRotateZDeg(ofGetElapsedTimef() * -2);
	mesh.draw();
;
	cam.end();
	ofDisableDepthTest();
	ofDisableAlphaBlending();

	gui.draw();

}

//-----------------------AUDIO----------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels) {

	for (int i = 0; i < bufferSize; i++) {

		//---------SYNTHESIS-------//

	
		// convert midi note to frequency
		pitch[0] = mtof.mtof(midiNote);
	

		//---------ENVELOPE----------//
		envOut[0] = env[0].adsr( 1., midiTrigger );				// input midi to trigger envelope
		gain[0] = 0.2;											// set gain on osc

		oscOut[0] = ( osc[0].sinewave( pitch[0] )  * envOut[0] ) * gain[0];	// multiply by envelope transient

		//---------------MIX----------------//
		mix = oscOut[0] * masterGainSlider;
		//=======Master=Stereo=Mix==========//
		masterMix.stereo(mix, outputs, 0.5);
		//==========MASTER-OUT==============//
		output[ i*nChannels     ] = outputs[0];
		output[ i*nChannels + 1 ] = outputs[1];
		//==================================//

		//-----------PROCESS-FFT------------//
		if (fft.process(mix)) {
			oct.calculate(fft.magnitudes);
		}
		//----------------------------------//

	}
}



//-------------------------MIDI---------------------------------
void ofApp::newMidiMessage(ofxMidiMessage &message) {
	
	//-------------------------------//
	if (message.status == MIDI_NOTE_ON) {          // setup midi to trigger note 
		midiNote = message.pitch;
		midiTrigger = 1;
	
	}
	else if (message.status == MIDI_NOTE_OFF) {
		midiTrigger = 0;
	}
	                     // setup midi to trigger pitch
	//-------------------------------//
												   // (this could be used to create a sequence later)
	messages.push_back(message);                   // create an array of midi messages and set size
	if (messages.size() > 16) {
		messages.erase(messages.begin());
	}


}

//--------------------------------------------------------------
void ofApp::midiDebug() {

	//--------------------------------------------// print midi note on/off and pitch to screen
	for (int i = 0; i < messages.size(); i++) { 
		ofxMidiMessage &message = messages[i];
		string midiTriggerDebug;
		if (message.status == MIDI_NOTE_ON) {
			midiTriggerDebug = "On";
		}
		else if (message.status == MIDI_NOTE_OFF) {
			midiTriggerDebug = "Off";
		}
													// draw midi notes to the screen
		ofSetColor(255);
		ofDrawBitmapString("Note " + midiTriggerDebug + ": Pitch: " + ofToString(message.pitch), ofGetWidth() * .8, 128 + i * 16);
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::exit() {
	midi.closePort();
	midi.removeListener(this);
	ofSoundStreamClose();
}
