#include <bits/stdc++.h>
#include "ScreenRecorder.h"

using namespace std;

/* driver function to run the application */
int main()
{
	ScreenRecorder screen_record;

	screen_record.openCamera();
	screen_record.openAudioDevice();
	screen_record.initOutputFile();
	screen_record.CreateThreads();
	//screen_record.start();
	//screen_record.stop();


	/* Metodi per distinguere il SO */
	/* Metodo 1 */
	// #ifdef __linux__
    // cout << "esiste MACRO __linux__" << endl;
    // #endif
    // #ifdef _WIN32
    // cout << "esiste MACRO _WIN32" << endl;
    // #endif

	/* Metodo 2 */
    // #ifdef __linux__
    // cout << "esiste MACRO __linux__" << endl;
    // #elif defined(_WIN32)
    // cout << "esiste MACRO _WIN32" << endl;
    // #endif
	/*****/

	cout << "\nprogram executed successfully\n";

	return 0;
}
