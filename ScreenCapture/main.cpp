#include <bits/stdc++.h>
#include "ScreenRecorder.h"
#include "AudioRecorder.h"

using namespace std;

/* driver function to run the application */
int main()
{	/*
	ScreenRecorder screen_record;

	screen_record.openCamera();
	screen_record.init_outputfile();
	screen_record.CaptureVideoFrames();
	//screen_record.start();
	//screen_record.stop();

	cout << "\nprogram executed successfully\n";

	return 0;
	*/
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
    return 0;

}
