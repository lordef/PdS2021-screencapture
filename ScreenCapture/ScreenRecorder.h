#pragma once
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
#include "AudioRecorder.h"
#ifdef _WIN32
	#include <Windows.h>
	#include <WinUser.h>
#endif



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

	
	AVCodecContext* pAVCodecContext;
	AVFormatContext* pAVFormatContext;

	AVFrame* pAVFrame;
	AVFrame* outFrame;

	AVCodec* outVideoCodec;
	AVCodec* pAVCodec;
	AVCodec* outAVCodec;
	AVCodec* pLocalCodec;

	AVCodecParameters* pCodecParameters;
	AVCodecParameters* pAVCodecParameters;

	AVPacket* pAVPacket;
	AVPacket* packet;

	AVDictionary* options;

	AVOutputFormat* outputAVFormat;
	AVFormatContext* outAVFormatContext;

	AVCodecContext* outAVCodecContext;
	AVCodecContext* outCodecContext; //TODO: Non usato?
	//AVCodecContext* outVideoCodecContext;


	AVStream* video_st;
	AVFrame* outAVFrame; //#TODO: questa variabile non viene utilizzata -> sarebbe outFrame?


	
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

	int outVideoStreamIndex;

	bool threading;

	// std::mutex stop_lockA; //a
	// std::mutex stop_lockV; //a
	// std::mutex error_lock; //a
	// std::string error_msg; //a
	// std::thread videoThread, audioThread; //a

	bool pauseSC; // utile a mettere in pausa la registrazione

	bool stopSC; // utile a terminare la registrazione
	// bool stopCaptureAudio = false; //a
	// bool stopCaptureVideo = false; //a

	bool started; //utile? utile per deallocare quando ScreenRecoder muore
	bool activeMenu;
	bool end; //#FIXME: variabile che potrebbe andare in contrasto con stopSC => CONTROLLARE

	// bool closedAudioRecording = false; //a
	// bool closedVideoRecording = false; //a
	int64_t pts = 0;

	
	int width, height; //ancora utile?
	int w, h; //ancora utile?
	std::string timestamp;
	std::string deviceName;
	uint64_t frameCount;
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
	 
	int captureVideoFrames(mutex* mu, condition_variable* cv, mutex* write_lock,
		condition_variable* cvw, AudioRecorder* audio);

	AVDictionary* openVideoDevice();

	void generateVideoStream();

	void setValue();

	int getMagic();

	int getPtsV();

	
	
	

	// void StopRecording(); //a
	// void PauseRecording(); //a
	// void CloseRecorder(); //a
	// bool ShouldStopAudio(); //a  <-- da copiare
	// bool ShouldStopVideo(); //a  <-- da copiare
	// void StopVideo(); //a
	// void StopAudio(); //a
	// void SetError(std::string error); //a
	// std::string GetErrorString(); //a
	#if _WIN32
		void SetCaptureSystemKey(int valueToSet, LPCWSTR keyToSet);
		// std::string RecordingPath = "..\\media\\output.mp4";
	#endif
};

#endif
