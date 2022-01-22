#include <bits/stdc++.h>
#include "ScreenRecorder.h"
#include "AudioRecorder.h"


using namespace std;

/* driver function to run the application */
int main()
{
	/*****Registrazione video ******/
	// ScreenRecorder screen_record;
	// screen_record.openCamera();
	// screen_record.init_outputfile();
	// screen_record.CaptureVideoFrames();

	//screen_record.start();	//non implementata
	//screen_record.stop();		//non implementata

	/***** Registrazione audio *****/
	puts("==== Audio Recorder ====");
    avdevice_register_all();

    AudioRecorder recorder{ "../media/testAudio.aac","" };
    try {
        recorder.Open();
        recorder.Start();

        //record 10 seconds.
        std::this_thread::sleep_for(10s);

        recorder.Stop();
        string reason = recorder.GetLastError();
        if (!reason.empty()) {
            throw std::runtime_error(reason);
        }
    }
    catch (std::exception& e) {
        fprintf(stderr, "[ERROR] %s\n", e.what());
        exit(-1);
    }

    puts("END");

	cout << "\nProgram executed successfully\n";

	return 0;
}


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