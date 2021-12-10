#include <bits/stdc++.h>
#include "ScreenRecorder.h"

using namespace std;

/* driver function to run the application */
int main()
{
	ScreenRecorder screen_record;

	screen_record.openCamera();
	screen_record.init_outputfile();
	screen_record.CaptureVideoFrames();
	//screen_record.start();
	//screen_record.stop();

	cout << "\nprogram executed successfully\n";

	return 0;
}
