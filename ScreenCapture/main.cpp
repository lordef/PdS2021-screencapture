#include <bits/stdc++.h>
#include "ScreenRecorder.h"

using namespace std;

/* driver function to run the application */
int main()
{
	/*****Registrazione video e audio ******/
	ScreenRecorder screen_record;
	screen_record.SetUpScreenRecorder();

	cout << "\nProgram executed successfully\n";

	return 0;
}