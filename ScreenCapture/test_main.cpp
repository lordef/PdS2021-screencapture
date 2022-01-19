/*  TODO: File da eliminare prima della consegna */
#include <bits/stdc++.h>
#include "ScreenRecorder.h"
#include <X11/Xlib.h>
#include<tuple>


using namespace std;

/* driver function to run the application */



tuple<int, int> retrieveDisplayDimentionTest()
{
	Display* disp = XOpenDisplay(NULL);
	Screen*  scrn = DefaultScreenOfDisplay(disp);
	int height = scrn->height;
	int width  = scrn->width;
	//cout << "\nStampa dimensioni display\n";
	//cout << width<<"x"<< height;
	
	return make_tuple(height, width);
	
}


int main()
{
	int height, width;
	tie(height, width)=retrieveDisplayDimentionTest();
	//cout << "\nStampa dimensioni display\n";
	//cout << width<<"x"<< height;

	string newString=to_string(height)+"x"+to_string(width);
	cout << newString;	

	return 0;
}



