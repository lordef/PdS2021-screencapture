#include <bits/stdc++.h>
#include "ScreenRecorder.h"

using namespace std;

/* driver function to run the application */
int main()
{
	/*****Registrazione video e audio ******/

	ScreenRecorder screen_record;

	screen_record.openCamera();
	screen_record.openAudioDevice();
	screen_record.initOutputFile();
	screen_record.CreateThreads();
	//screen_record.start();	// #TODO: implementata?
	//screen_record.stop();		// #TODO: implementata?


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

	cout << "\nProgram executed successfully\n";

	return 0;
}



    /*  #TODO: a breve termine */
    /* Situazione attuale in RUN; TERMINAL OUTPUT:  */ //N.B.: A VOLTE AD ISABELLA NON COMPARE
    /*
    ==== Audio Recorder ====
    Start record.
    [aac @ 0x555557111080] Qavg: 55.663
    [aac @ 0x555557111080] 2 frames left in the queue on closing
    Stop record.
    [ERROR] std::exception
    [1] + Done                       "/usr/bin/gdb" --interpreter=mi --tty=${DbgTerm} 0<"/tmp/Microsoft-MIEngine-In-40xhisgm.x0q" 1>"/tmp/Microsoft-MIEngine-Out-gkxkoqxg.l1a"
    
    */
    /*************************************************************/
    /* Situazione attuale in DEBUG; TERMINAL OUTPUT: */
    /*
    ==== Audio Recorder ====
    [ERROR] Fail to open output file.

    Forse risolto, ma controllare riga 25
    */