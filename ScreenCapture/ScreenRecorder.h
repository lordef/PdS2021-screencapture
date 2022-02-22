#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H
#include "ffmpeg.h" // #TODO: potrebbe non servire poichè include dopo - riga 17
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string>

#include <thread>
#include <mutex>
#include <iomanip>

#include <condition_variable>
#ifdef _WIN32
	#include <Windows.h>
	#include <WinUser.h>
	// #include "ListAVDevices.h" //TODO: NON dovrebbe servire - no a
#endif


#include <ctime>
#include <sstream>

// #define QT 1 //a
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
// #include "libavutil/avutil.h" //a //TODO: non dovrebbe servire
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
	AVInputFormat* audioInputFormat;
	
	AVCodecContext* pAVCodecContext;
	AVCodecContext* inAudioCodecContext; 
	AVCodecContext* outAudioCodecContext;
	AVFormatContext* pAVFormatContext;

	AVFrame* pAVFrame;
	AVFrame* outFrame;

	AVCodec* outVideoCodec;
	AVCodec* pAVCodec;
	AVCodec* outAVCodec;
	AVCodec* pLocalCodec;
	AVCodec* inAudioCodec;
	AVCodec* outAudioCodec;
	AVCodecParameters* pCodecParameters;
	AVCodecParameters* pAVCodecParameters;

	AVPacket* pAVPacket;
	AVPacket* packet;

	AVDictionary* options;
	AVDictionary* audioOptions;

	AVOutputFormat* outputAVFormat;
	AVFormatContext* outAVFormatContext;

	AVFormatContext* inAudioFormatContext;
	AVCodecContext* outAVCodecContext;
	AVCodecContext* outCodecContext; //TODO: Non usato?
	//AVCodecContext* outVideoCodecContext;

	AVAudioFifo* fifo;//Nuovo

	AVStream* video_st;
	AVStream* audio_st; // TODO: no a 
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
	
	int magicNumber; //TODO: eliminare prima di consegna
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
	int audioStreamIndx;
	int outVideoStreamIndex;
	int outAudioStreamIndex;
	bool threading;
	std::thread* demux;
	std::thread* rescale;
	std::thread* mux;
	// std::mutex stop_lockA; //a
	// std::mutex stop_lockV; //a
	// std::mutex error_lock; //a
	// std::string error_msg; //a
	// std::thread videoThread, audioThread; //a
	
	bool isAudioActive;
	bool pauseSC = false; // utile a mettere in pausa la registrazione

	bool stopSC = false; // utile a terminare la registrazione
	// bool stopCaptureAudio = false; //a
	// bool stopCaptureVideo = false; //a

	bool started; //utile? utile per deallocare quando ScreenRecoder muore
	bool activeMenu;
	bool end = false; //#FIXME: variabile che potrebbe andare in contrasto con stopSC => CONTROLLARE

	// bool closedAudioRecording = false; //a
	// bool closedVideoRecording = false; //a
	int64_t pts = 0;


	int width, height; //ancora utile?
	int w, h; //ancora utile?
	std::string timestamp;
	std::string deviceName;
	uint64_t frameCount = 0;
	double fps;

	// #if linux //a
	// 	std::string url;
	// 	std::string dimension;
	// #endif

public:
	ScreenRecorder();
	// ScreenRecorder( bool isAudioActive, int cropX, int cropY, int cropH, int cropW,
	// 				int magicNumber, bool activeMenu); // #TODO: valutare se sopprimere magicNumber e activeMenu 

	// ScreenRecorder(std::string RecPath); //a //utile? -> settaggio dell'outputPath
	// ScreenRecorder(const ScreenRecorder& p1); //a //utile?

	~ScreenRecorder();

	/* Function to initiate communication with display library */
	int initOutputFile(); 
	int captureVideoFrames();

	int openVideoDevice();
	int openAudioDevice();

	void generateVideoStream();
	void generateAudioStream(); 

	int init_fifo(); 
	int add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size); 
	int initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size);
	
	void captureAudio(); 
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
	int stopScreenCapture(); // <-- doing

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

	// void StopRecording(); //a
	// void PauseRecording(); //a
	// void CloseRecorder(); //a
	// bool ShouldStopAudio(); //a  <-- da copiare
	// bool ShouldStopVideo(); //a  <-- da copiare
	// void StopVideo(); //a
	// void StopAudio(); //a
	// void SetError(std::string error); //a
	// std::string GetErrorString(); //a
	#if WIN32
		void SetCaptureSystemKey(int valueToSet, LPCWSTR keyToSet);
		// std::string RecordingPath = "..\\media\\output.mp4";
	#endif
};

#endif
