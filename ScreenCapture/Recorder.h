#include "AudioRecorder.h"
#include "ScreenRecorder.h"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <iomanip>
#include <sstream>

#include <mutex>
#include <condition_variable>
#include <ctime>
class Recorder
{

private:
	bool isAudioActive;
	bool stopSC;
	bool pauseSC; // utile a mettere in pausa la registrazione
	int val;

	std::string timestamp;
	std::mutex mu; 
	std::mutex write_lock; 
	std::condition_variable cv; 
	std::condition_variable cvw;

	AVOutputFormat* outputAVFormat;
	AVFormatContext* outAVFormatContext;
	AVDictionary* options;

	ScreenRecorder screen_record;
	AudioRecorder audio_record;


public:

	Recorder();
	~Recorder();
	std::string retrieveTimestamp();
	int initOutputFile();

	void CreateThreads();
	/*** API ancora da implementare/testare ***/

	/* Define the area to be recorded */
	// #TODO: incorporata nel costruttore; si vedano i parametri cropX-Y-H-W
	// da testare in Windows se funziona
	// fare un costruttore che prenda questi dati di crop come input, cosa da fare anche per la prossima API

	/* Select whether the audio should be captured or not */
	/*
	#TODO: fare un costruttore che setta isAudioActive a true o false, ora di default sta a true
		Convenzione da discutere: q
			questa scelta non ci permette di mettere o togliere l'audio mentre si sta registrando, dovrebbe andar bene
	*/

	/* Activate and stop the recording process */
	//#TODO: bisogna capie se sfruttare unique_lock o meccanismi del genere
	int stopScreenCapture();

	/* Temporarily pause and subsequently resume it */
	//#TODO: bisogna capie se sfruttare unique_lock o meccanismi del genere
	//#TODO: da testare
	int toggleScreenCapture();


	/* Define the file that will contain the final recording */
	//#TODO --> vedi funzione initOutputFile() e adattarla a prendere un input (sarebbe il filepath)

	/* Indication of recording in progress */
	//#TODO: magari fare polling su variabile di stop (stopSC) -> forse solo interfaccia


	/*** fine - API ancora da implementare/testare ***/



	/* Avvia le funzioni principali */
	void SetUpScreenRecorder();


};

