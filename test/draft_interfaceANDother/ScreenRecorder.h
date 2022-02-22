#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H
#define QT 1
#include "ffmpeg.h"
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
#if WIN32
#include <Windows.h>
#include <WinUser.h>
#elif linux
#endif


#define __STDC_CONSTANT_MACROS

//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"

#include "libavdevice/avdevice.h"

#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"

#include "libavformat/avformat.h"
#include "libavformat/avio.h"

	// libav resample
#include "libavutil/avutil.h"
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
	AVInputFormat* audioInputFormat;
	AVOutputFormat* output_format;

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
	AVCodecContext* outVideoCodecContext;
	AVAudioFifo* fifo;
	AVStream* videoSt;
	AVFrame* outAVFrame;

	const char* dev_name;
	const char* output_file;
	
	double video_pts;

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
	bool stopCaptureAudio = false;
	bool stopCaptureVideo = false;
	bool started;
	bool activeMenu;
	bool closedAudioRecording = false;
	bool closedVideoRecording = false;
	int width, height;
	int64_t pts = 0;
	
#if linux
	std::string url;
	std::string dimension;
#endif


public:

	ScreenRecorder();
	ScreenRecorder(std::string RecPath);
	ScreenRecorder(const ScreenRecorder& p1);
	~ScreenRecorder();

	/* function to initiate communication with display library */
	int OpenVideoDevice();
	int OpenAudioDevice();
	int InitOutputFile();
	void GenerateVideoStream();
	void GenerateAudioStream();
	int InitFifo();
	int AddSamplesToFifo(uint8_t** converted_input_samples, const int frame_size);
	int InitConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size);
	void CaptureAudio();
	int CaptureVideoFrames();
	void CreateThreads();
	void SetUpScreenRecorder();
	void StopRecording();
	void PauseRecording();
	void CloseRecorder();
	bool ShouldStopAudio();
	bool ShouldStopVideo();
	void StopVideo();
	void StopAudio();
	void SetError(std::string error);
	std::string GetErrorString();
	int cropX = 0;
	int cropY = 0;
	int cropH ;
	int cropW ;
	int ptsA;
	int ptsV;
	bool recordAudio = true;
	bool pauseCapture;
	std::mutex mu;
	std::mutex write_lock;
	std::mutex stop_lockA;
	std::mutex stop_lockV;
	std::mutex error_lock;
	std::condition_variable cv;
	std::condition_variable cvw; 
	std::string error_msg;
	std::thread videoThread, audioThread;

#if WIN32
	void SetCaptureSystemKey(int valueToSet, LPCWSTR keyToSet);
	std::string RecordingPath = "..\\media\\output.mp4";
#endif
};

#endif