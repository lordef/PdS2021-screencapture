#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include "ffmpeg.h" // #TODO: potrebbe non servire poichè include dopo - riga 17
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>
#include <thread>
#include <time.h> // #TODO: forse utile per dare al file il nome con all'interno la data 



#define __STDC_CONSTANT_MACROS

//FFMPEG LIBRARIES
extern "C"
{
	#include "libavcodec/avcodec.h"
	#include "libavcodec/avfft.h"

	#include "libavdevice/avdevice.h"

	#include "libavfilter/avfilter.h"
	//#include "libavfilter/avfiltergraph.h" //#TODO: commentato perchè su linux non serve
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

		AVCodecContext* pAVCodecContext;

		AVFormatContext* pAVFormatContext;

		AVFrame* pAVFrame;
		AVFrame* outFrame;

		AVCodec* pAVCodec;
		AVCodec* outAVCodec;
		AVCodec* pLocalCodec;
		AVCodecParameters* pCodecParameters;

		AVPacket* pAVPacket;
		AVPacket* packet;

		AVDictionary* options;

		AVOutputFormat* outAVOutputFormat;
		AVFormatContext* outAVFormatContext;
		AVCodecContext* outAVCodecContext;
		AVCodecContext* outCodecContext;

		AVStream* video_st;
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
		bool threading;
		std::thread* demux;
		std::thread* rescale;
		std::thread* mux;



	public:
		ScreenRecorder();
		~ScreenRecorder();

		/* function to initiate communication with display library */
		int init_outputfile();
		int CaptureVideoFrames();
		int openCamera();
		//int start();
		//int stop();
		//int initVideoThreads();
		//void demuxVideoStream(AVCodecContext* codecContext, AVFormatContext* formatContext, int streamIndex);
		//void rescaleVideoStream(AVCodecContext* inCodecContext, AVCodecContext* outCodecContext);
		//void encodeVideoStream(AVCodecContext* codecContext);

};

#endif
