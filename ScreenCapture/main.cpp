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

    #ifdef __linux__
        AudioRecorder recorder{ "media/testAudio.aac","" }; //#TODO: Dovrebbe funzionare così
    #elif _WIN32
        AudioRecorder recorder{ "../media/testAudio.aac","" };
    #endif
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

    /*  #TODO: a breve termine */
    /* Situazione attuale in RUN; TERMINAL OUTPUT:  */
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