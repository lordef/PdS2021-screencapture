#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H
#include "ffmpeg.h" // #TODO: potrebbe non servire poichè include dopo - riga 17
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string>//Aggiornato -> era #include <string.h>

#include <thread>//Nuovo
#include <mutex>//Nuovo
#include <iomanip>//Nuovo

#ifdef __linux__
	#include <condition_variable> //#TODO: dovrebbe serveire anche per Windows
#elif _WIN32
	#include <Windows.h>//Nuovo
	#include <WinUser.h>//Nuovo
	#include "ListAVDevices.h"
#endif


#include <ctime>//Nuovo
#include <sstream>//Nuovo


// #define AUDIO 1 //TODO: ora inutile -> sostituita da isAudioActive
#define RUN 1 //#TODO: utile per debuggare, eliminare prima della consegna -> cerca nel codice "#if RUN == 1" ed elimianre



#define __STDC_CONSTANT_MACROS

//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"

#include "libavdevice/avdevice.h"

#include "libavfilter/avfilter.h"
#ifdef __linux__
	//#include "libavfilter/avfiltergraph.h" //#TODO: commentato perchè su linux non serve
#elif _WIN32
	#include "libavfilter/avfiltergraph.h"
#endif
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#include "libavformat/avformat.h"
#include "libavformat/avio.h"

	// libav resample

#include "libavutil/opt.h"
#include "libavutil/common.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/file.h"

// lib swresample

#include "libswscale/swscale.h"

}



class ScreenRecorder
{
private:
	AVInputFormat* pAVInputFormat;
	AVOutputFormat* output_format;
	AVInputFormat* audioInputFormat; //Nuovo
	
	AVCodecContext* pAVCodecContext;
	AVCodecContext* inAudioCodecContext; //Nuovo
	AVCodecContext* outAudioCodecContext; //Nuovo
	AVFormatContext* pAVFormatContext;

	AVFrame* pAVFrame;
	AVFrame* outFrame;

	AVCodec* outVideoCodec; //Nuovo
	AVCodec* pAVCodec;
	AVCodec* outAVCodec;
	AVCodec* pLocalCodec;
	AVCodec* inAudioCodec; //Nuovo
	AVCodec* outAudioCodec; //Nuovo
	AVCodecParameters* pCodecParameters;
	AVCodecParameters* pAVCodecParameters; //Nuovo

	AVPacket* pAVPacket;
	AVPacket* packet;

	AVDictionary* options;
	AVDictionary* audioOptions; //Nuovo

	AVOutputFormat* outputAVFormat; //Aggiornato
	AVFormatContext* outAVFormatContext;

	AVFormatContext* inAudioFormatContext; //Nuovo
	AVCodecContext* outAVCodecContext;
	AVCodecContext* outCodecContext; //Non usato?
	//AVCodecContext* outVideoCodecContext; //Nuovo

	AVAudioFifo* fifo;//Nuovo

	AVStream* video_st;
	AVStream* audio_st;
	AVFrame* outAVFrame; //#TODO: questa variabile non viene utilizzata -> sarebbe outFrame?


	std::mutex mu; //Nuovo
	std::mutex write_lock; //Nuovo
	std::condition_variable cv; //Nuovo
	std::condition_variable cvw;
	const char* dev_name;
	const char* output_file;

	// double video_pts;
	int ptsA;
	int ptsV;
	boolean end;
	int magicNumber; //TODO: cosa sta a rappresentare
	int cropX; 
	int cropY; 
	int cropH; 
	int cropW; 

	int out_size;
	int codec_id;
	int value;
	int value2;
	int value3;
	int VideoStreamIndx;
	int audioStreamIndx; //Nuovo
	int outVideoStreamIndex; //Nuovo
	int outAudioStreamIndex; //Nuovo
	bool threading; //Nuovo
	std::thread* demux;
	std::thread* rescale;
	std::thread* mux;
	
	bool isAudioActive;
	bool pauseSC; // utile a mettere in pausa la registrazione
	bool stopSC; // utile a terminare la registrazione
	bool started; // utile per deallocare quando ScreenRecoder muore
	bool activeMenu; //Nuovo
	int width, height; //Nuovo
	int w, h; //Nuovo
	std::string timestamp;//Nuovo
	std::string deviceName;
	uint64_t  frameCount;
	double fps;

public:
	ScreenRecorder();
	~ScreenRecorder();

	/* Function to initiate communication with display library */
	int initOutputFile(); //Aggiornato
	int captureVideoFrames();
	int openCamera();

	int openVideoDevice(); //Nuovo
	int openAudioDevice(); //Nuovo

	void generateVideoStream(); //Nuovo
	void generateAudioStream(); //Nuovo

	int init_fifo(); //Nuovo
	int add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size); //Nuovo
	int initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size); //Nuovo
	
	void captureAudio(); //Nuovo
	void CreateThreads(); //Nuovo
	
	/*** API ancora da implementare/testare ***/

	/* Define the area to be recorded */
	// #TODO: incorporata nel cotruttore; si vedano i parametri cropX-Y-H-W
	// da testare in Windows se funziona
	// fare un cotruttore che prenda questi dati di crop come input, cosa da fare anche per la prossima API

	/* Select whether the audio should be captured or not */
	/*
	#TODO: fare un costruttore che setta isAudioActive a true o false, ora di defualt sta a true
	Questa scelta non ci permette di mettere o togliere l'audio mentre si sta registrando, dovrebbe andar bene
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
	static void SetUpScreenRecorder();
};

#endif
