#include <bits/stdc++.h>
#include "ScreenRecorder.h"

using namespace std;

/* driver function to run the application */
int main()
{
	/*****Registrazione video e audio ******/
	/*Vedi SetUpScreenRecorder() */
	ScreenRecorder screen_record;

	screen_record.openCamera();
	screen_record.openAudioDevice();

	screen_record.initOutputFile();

	screen_record.CreateThreads();

	/*fine SetUpScreenRecorder() */

	// screen_record.start();	// #TODO: implementata?
	// screen_record.stop();		// #TODO: implementata?

	cout << "\nProgram executed successfully\n";

	return 0;
}