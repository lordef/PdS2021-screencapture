#ifndef AUDIORECORDER_AUDIORECORDER_H
#define AUDIORECORDER_AUDIORECORDER_H

#ifdef __linux__
#include <atomic>
#include <thread>
#elif _WIN32
//#include "ListAVDevices.h"
#endif

#include "ffmpeg.h"
#include <string>
#include <cstdint>
#include <iostream>
#include "ScreenRecorder.h"

#pragma once

class AudioRecorder
{
private:
	bool pauseSC;
	bool stopSC;
	bool activeMenu;
	bool end;
	
	int magicNumber;
	int value;
	int audioStreamIndx;
	int outAudioStreamIndex;
	int ptsA;
	int ptsV;
	uint64_t frameCount;

	std::string deviceName;

	AVAudioFifo* fifo;

	AVCodec* inAudioCodec;
	AVCodec* outAudioCodec;

	AVStream* audio_st;

	AVCodecContext* inAudioCodecContext;
	AVCodecContext* outAudioCodecContext;

	AVFormatContext* inAudioFormatContext;
	AVFormatContext* outAVFormatContext;

	AVInputFormat* audioInputFormat;

	AVDictionary* audioOptions;
	ScreenRecorder screen;

	mutex* mux;

public:
	AudioRecorder();
	~AudioRecorder();
	int openAudioDevice();
	void generateAudioStream();
	int init_fifo();
	int add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size);
	int initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size);

	void setValue(AVFormatContext* f);
	void captureAudio(mutex* mu, condition_variable* cv, mutex* write_lock,
						condition_variable* cvw, ScreenRecorder* screen);
	int getPtsA();

};

#endif 