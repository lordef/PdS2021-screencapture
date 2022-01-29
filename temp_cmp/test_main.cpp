/*  TODO: File da eliminare prima della consegna */
#include <bits/stdc++.h>
#include "ScreenRecorder.h"
#include <X11/Xlib.h> //useful lib installed: sudo apt install libx11-dev 
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

std::string space2underscore(std::string text)
{
    std::replace(text.begin(), text.end(), ' ', '_');
    return text;
}

int main()
{
	//int height, width;
	//tie(height, width)=retrieveDisplayDimentionTest();
	//cout << "\nStampa dimensioni display\n";
	//cout << width<<"x"<< height;

	//string newString=to_string(height)+"x"+to_string(width);
	//cout << newString;	
	
	/***Codice Gabriele***/
	std::string timestamp;//Nuovo
	AVOutputFormat* outputAVFormat; //Aggiornato
	stringstream ss;
	time_t result = time(nullptr);
    ss << time;
    timestamp = ss.str();
    string outputName = timestamp + " output.mp4";
   

	cout << "\nStampa nome file\n";
	cout << outputName <<"\n";


	/* Setting name of the output file */
    // declaring argument of time()
    time_t my_time = time(NULL);
	//string format_output_file = "../media/output_";
	//string output_file = "../media/output.mp4";
	string output_file = "output_";
	/***Codice nostro***/
    //ctime() used to give the present time
	string current_time = ctime(&my_time);   
	current_time.erase(current_time.end()-1, current_time.end());
    output_file.append(current_time);
    output_file.append(".mp4"); //ERRORE qui, potrebbe essere perchè non accetta una stringa essendo const char*
	
	replace(output_file.begin(), output_file.end(), ' ', '_');


	cout << "\nStampa nome file\n";
	cout << output_file <<"\n";



	return 0;
}



