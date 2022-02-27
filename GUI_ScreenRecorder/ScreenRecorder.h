#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H
#include "ffmpeg.h" 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string>
#include<vector>
#pragma comment(lib, "Advapi32.lib")


#include <thread>
#include <mutex>
#include <iomanip>

#include <condition_variable>
#ifdef _WIN32
#include <Windows.h>
#include <WinUser.h>
#include <wtypes.h>
#endif


#include <ctime>
#include <sstream>


#define RUN 1 // utile per debuggare, eliminare prima della consegna -> cerca nel codice "#if RUN == 1" ed elimianre



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
// #include "libavutil/avutil.h" 
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
	AVCodecContext* outCodecContext; 

	AVAudioFifo* fifo;
	AVStream* video_st;
	AVStream* audio_st; 
	AVFrame* outAVFrame; 


	std::mutex mu;
	std::mutex write_lock;
	std::mutex stop_lockA, stop_lockV, error_lock;
	std::condition_variable cv;
	std::condition_variable cvw;
	const char* dev_name;
	const char* output_file;
	std::string error_msg;

	int ptsA;
	int ptsV;

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
	std::thread tV, tA;

	bool isAudioActive;
	bool pauseRec; // utile a mettere in pausa la registrazione
	bool stopRecAudio;// utile a stopppare la registrazione Audio
	bool stopRecVideo;// utile a stopppare la registrazione Video

	bool started; 
	bool end = false; 
	bool closedVideo;
	bool closedAudio;
	int64_t pts = 0;


	int width, height; 
	int w, h; 
	std::string timestamp;
	std::string outputPath;


	std::string deviceName;
	uint64_t frameCount = 0;
	double fps;


public:
	ScreenRecorder();
	

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



	/* Avvia le funzioni principali */
	void SetUpScreenRecorder();

	void StopRecorder();
	bool PauseRecorder();
	void CloseRecorder();
	void VideoStop();
	void AudioStop();


	bool getAudioBool();
	bool getVideoBool();
	std::string getOutputPath();

	void SetIsAudioActive(bool check);
	void setOutputPath(std::string outputPath);
	void setCrop(int cX, int cY, int cW, int cH);

	#if _WIN32
	void SetCaptureSystemKey(int valueToSet, LPCWSTR keyToSet);
	#endif



};

#endif
