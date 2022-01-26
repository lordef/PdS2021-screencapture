#ifndef AUDIORECORDER_AUDIORECORDER_H
#define AUDIORECORDER_AUDIORECORDER_H

#ifdef __linux__
    #include <atomic>
    #include <thread>
#elif defined(_WIN32)
    #include "ListAVDevices.h"
#endif

#include "ffmpeg.h"
#include <string>
#include <cstdint>


using std::string;

class AudioRecorder {

private:
    string outfile;
    string deviceName;
    string failReason;
    std::atomic_bool isRun;


    AVFormatContext* audioInFormatCtx;
    AVStream* audioInStream;
    AVCodecContext* audioInCodecCtx;

    SwrContext* audioConverter;
    AVAudioFifo* audioFifo;

    AVFormatContext* audioOutFormatCtx;
    AVStream* audioOutStream;
    AVCodecContext* audioOutCodecCtx;

    std::thread* audioThread;

    void StartEncode();


public:

    AudioRecorder(string filepath, string device)
        :outfile(filepath), deviceName(device), failReason(""), isRun(false) {}

    void Open();
    void Start();
    void Stop();

    ~AudioRecorder() {
        Stop();
    }

    std::string GetLastError() { 
        return failReason; 
    }
};
#endif //AUDIORECORDER_AUDIORECORDER_H

