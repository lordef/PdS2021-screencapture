#include "utils.h"

tuple<int, int> retrieveDisplayDimention()
{
	Display* disp = XOpenDisplay(NULL);
	Screen*  scrn = DefaultScreenOfDisplay(disp);
	int height = scrn->height;
	int width  = scrn->width;
	//cout << "\nStampa dimensioni display\n";
	//cout << width<<"x"<< height;
	
	return make_tuple(height, width);
}