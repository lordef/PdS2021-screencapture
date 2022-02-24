#include <bits/stdc++.h>
#include "Recorder.h"

using namespace std;

/* driver function to run the application */
int main()
{
	/*****Registrazione video e audio ******/
	/*Vedi SetUpScreenRecorder() */
	Recorder record;

	//screen_record.openVideoDevice();
	//screen_record.openAudioDevice();

	record.initOutputFile();

	record.CreateThreads();

	/*fine SetUpScreenRecorder() */

	// screen_record.start();	// #TODO: implementata?
	// screen_record.stop();		// #TODO: implementata?

	cout << "\nProgram executed successfully\n";

	return 0;
}