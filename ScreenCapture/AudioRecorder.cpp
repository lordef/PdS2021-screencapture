#include "AudioRecorder.h"
using namespace std;



AudioRecorder::AudioRecorder() :  pauseSC(false), stopSC(false), /* started(true), */ activeMenu(true),
                                  magicNumber(300), frameCount(0), end(false)
{
    //TIMESTAMP?
    
    // 
    //avdevice_register_all(); // Inizializza libavdevice e registra tutti i dispositivi di input e output.

    //outAVFormatContext deve essere passato come parametro al costruttore
    // e deve essere già inizializzato
    outAVFormatContext = nullptr;

    cout << "\nAll required functions are registered successfully\n";
}

/* Definiamo il DISTRUTTORE */
/* uninitialize the resources */
AudioRecorder::~AudioRecorder()
{
    // if (started) { //utile?

        // ---------------Audio-----------------
    avformat_close_input(&inAudioFormatContext);
    if (inAudioFormatContext == nullptr) {
        cout << "inAudioFormatContext close successfully" << endl;
    }
    else {
        cerr << "Error: unable to close the inAudioFormatContext" << endl;
        exit(-1);
        //throw "Error: unable to close the file";
    }
    avformat_free_context(inAudioFormatContext);
    if (inAudioFormatContext == nullptr) {
        cout << "AudioFormat freed successfully" << endl;
    }
    else {
        cerr << "Error: unable to free AudioFormatContext" << endl;
        exit(-1);
    }
}

int AudioRecorder::openAudioDevice() {
    audioOptions = nullptr;
    inAudioFormatContext = nullptr;

    inAudioFormatContext = avformat_alloc_context();

    /*Setta le options con valori quali il sample rate e altri*/
    value = av_dict_set(&audioOptions, "sample_rate", "44100", 0);
    if (value < 0) {
        cerr << "Error: cannot set audio sample rate" << endl;
        exit(-1);
    }
    value = av_dict_set(&audioOptions, "async", "1", 0);
    if (value < 0) {
        cerr << "Error: cannot set audio sample rate" << endl;
        exit(-1);
    }

#ifdef __linux__
    //Questi comandi funzionano: 
    // ffmpeg -f alsa -i default -t 30 out.wav
    // ffmpeg -video_size 1024x768 -framerate 25 -f x11grab -i :0.0 output.mp4
    // Il seguente comando è una combianzione dei precedenti, funziona ed è sincronizzato:
    // ffmpeg -video_size 1024x768 -framerate 25 -f x11grab -i :0.0 -f alsa -i default -t 30 av_output.mp4

    audioInputFormat = const_cast<AVInputFormat*>(av_find_input_format("alsa")); //un dispositivo alternativo potrebbe essere xcbgrab, non testato   

    // const char* url = "alsa_input.pci-0000_00_1f.3.analog-stereo"; //funziona con pulse
    // const char* url = "hw:0"; // NON funziona con alsa
    // const char* url = "default"; // funziona con alsa
    deviceName = "default";

    // value = avformat_open_input(&inAudioFormatContext, "alsa_input.pci-0000_00_1f.3.analog-stereo", audioInputFormat, &audioOptions); //così funziona
    // value = avformat_open_input(&inAudioFormatContext, url, audioInputFormat, &audioOptions); //così funziona
    value = avformat_open_input(&inAudioFormatContext, deviceName.c_str(), audioInputFormat, &audioOptions); //così funziona
    if (value != 0) {
        cerr << "Error in opening input device (audio)" << endl;
        exit(-1);
    }

#elif _WIN32
    /*
    if (deviceName == "") {
        deviceName = DS_GetDefaultDevice("a");
        if (deviceName == "") {
            throw std::runtime_error("Fail to get default audio device, maybe no microphone.");
        }
    }*/
    //deviceName = "audio=" + deviceName;
    audioInputFormat = av_find_input_format("dshow");
    
    value = avformat_open_input(&inAudioFormatContext, "audio=Microfono (Realtek(R) Audio)", audioInputFormat, &audioOptions);
    //value = avformat_open_input(&inAudioFormatContext, deviceName.c_str(), audioInputFormat, &audioOptions);
    //value = avformat_open_input(&inAudioFormatContext, "audio=Microphone Array (Realtek(R) Audio)", audioInputFormat, &audioOptions);
    if (value != 0) {
        cerr << "Error in opening input device (audio)" << endl;
        exit(-1);
    }
#endif

    /* Lettura  pacchetti dello stream per ottenerne informazioni */
    value = avformat_find_stream_info(inAudioFormatContext, nullptr);
    if (value != 0) {
        cerr << "Error: cannot find the audio stream information" << endl;
        exit(-1);
    }

    audioStreamIndx = -1;
    for (int i = 0; i < inAudioFormatContext->nb_streams; i++) {
        if (inAudioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndx = i;
            break;
        }
    }
    if (audioStreamIndx == -1) {
        cerr << "Error: unable to find audio stream index" << endl;
        exit(-2);
    }

    return 0;

}

void AudioRecorder::generateAudioStream() {
    AVCodecParameters* params = inAudioFormatContext->streams[audioStreamIndx]->codecpar;
    inAudioCodec = const_cast<AVCodec*>(avcodec_find_decoder(params->codec_id));

    if (inAudioCodec == nullptr) {
        cerr << "Error: cannot find the audio decoder" << endl;
        exit(-1);
    }

    inAudioCodecContext = avcodec_alloc_context3(inAudioCodec);
    if (avcodec_parameters_to_context(inAudioCodecContext, params) < 0) {
        cout << "Cannot create codec context for audio input" << endl;
    }

    value = avcodec_open2(inAudioCodecContext, inAudioCodec, nullptr);
    if (value < 0) {
        cerr << "Error: cannot open the input audio codec" << endl;
        exit(-1);
    }

    //Generate audio stream
    outAudioCodecContext = nullptr;
    outAudioCodec = nullptr;
    int i;

    outAudioCodec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_AAC));
    if (outAudioCodec == nullptr) {
        cerr << "Error: cannot find requested encoder" << endl;
        exit(1);
    }

    audio_st = avformat_new_stream(outAVFormatContext, outAudioCodec);
    if (audio_st == nullptr) {
        cerr << "Error: cannot create audio stream" << endl;
        exit(1);
    }

    outAudioCodecContext = avcodec_alloc_context3(outAudioCodec);
    if (outAudioCodecContext == nullptr) {
        cerr << "Error: cannot create related VideoCodecContext" << endl;
        exit(1);
    }

    /*Cerca nei sample rate supportati dal codec dello stream di output dell'audio
    Se il sample rate supportato è uguale a quello del context di input, setta il sample rate
    del codec di output pari a quello di input*/
    if ((outAudioCodec)->supported_samplerates) {
        outAudioCodecContext->sample_rate = (outAudioCodec)->supported_samplerates[0];
        for (i = 0; (outAudioCodec)->supported_samplerates[i]; i++) {
            if ((outAudioCodec)->supported_samplerates[i] == inAudioCodecContext->sample_rate)
                outAudioCodecContext->sample_rate = inAudioCodecContext->sample_rate;
        }
    }
    outAudioCodecContext->codec_id = AV_CODEC_ID_AAC;
    outAudioCodecContext->sample_fmt = (outAudioCodec)->sample_fmts ? (outAudioCodec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outAudioCodecContext->channels = inAudioCodecContext->channels;
    outAudioCodecContext->channel_layout = av_get_default_channel_layout(outAudioCodecContext->channels);
    outAudioCodecContext->bit_rate = 90000; // G: 96000
    /** //#FIXME: test: metterli uguali ad audio e video **/
    // outAudioCodecContext->time_base = { 1, inAudioCodecContext->sample_rate }; //#FIXME: dovrebbe settarsi da solo nelle prossime due operaioni a detta dei colleghi; ovvero flags e avcodec_open2
    // outAudioCodecContext->time_base.num = 1; 
    // outAudioCodecContext->time_base.den = 12.5; 
    /****/

    outAudioCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    if ((outAVFormatContext)->oformat->flags & AVFMT_GLOBALHEADER) {
        outAudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outAudioCodecContext, outAudioCodec, nullptr) < 0) {
        cerr << "error in opening the avcodec" << endl;
        exit(1);
    }


    //find a free stream index
    outAudioStreamIndex = -1;
    /* Trova lo stream basandosi su quale ha il codec type AVMEDIA_TYPE_UNKNOWN*/
    for (i = 0; i < outAVFormatContext->nb_streams; i++) {
        if (outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            outAudioStreamIndex = i;
        }
    }
    if (outAudioStreamIndex < 0) {
        cerr << "Error: cannot find a free stream for audio on the output" << endl;
        exit(1);
    }

    avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outAudioCodecContext);
}

int AudioRecorder::init_fifo()
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(fifo = av_audio_fifo_alloc(outAudioCodecContext->sample_fmt,
        outAudioCodecContext->channels, outAudioCodecContext->sample_rate))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

int AudioRecorder::add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size) {
    int error;
    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void**)converted_input_samples, frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

int AudioRecorder::initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size) {
    int error;
    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = (uint8_t**)calloc(output_codec_context->channels,
        sizeof(**converted_input_samples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if (av_samples_alloc(*converted_input_samples, nullptr,
        output_codec_context->channels,
        frame_size,
        output_codec_context->sample_fmt, 0) < 0) {

        exit(1);
    }
    return 0;
}

void AudioRecorder::setValue(AVFormatContext* f){
    outAVFormatContext = f;

    
}




void AudioRecorder::captureAudio(mutex* mu, condition_variable* cv, mutex* write_lock,
                                        condition_variable* cvw, ScreenRecorder* screen) {
    int ret;
    mux = mu;
    AVPacket* inPacket, * outPacket;
    AVFrame* rawFrame, * scaledFrame;
    uint8_t** resampledData;

    init_fifo();

#ifdef __linux__
#if RUN == 1 
    //ofstream outFile{ "media/" + timestamp + "_log.txt", ios::app }; // RUN
#else  
   // ofstream outFile{ "../media/" + timestamp + "_log.txt", ios::app }; // DEBUG
#endif 
#elif _WIN32
    //ofstream outFile{ "..\\media\\" + timestamp + "_log.txt", ios::app };
#endif

    //allocate space for a packet
    //inPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    inPacket = av_packet_alloc();
    if (!inPacket) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
    }
    //av_init_packet(inPacket); //utile?

    //allocate space for a packet
    rawFrame = av_frame_alloc();
    if (!rawFrame) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
    }

    scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
    }

    outPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!outPacket) {
        cerr << "Cannot allocate an AVPacket for encoded video" << endl;
        exit(1);
    }

    //init the resampler
    SwrContext* resampleContext = nullptr;
    resampleContext = swr_alloc_set_opts(resampleContext,
        av_get_default_channel_layout(outAudioCodecContext->channels),
        outAudioCodecContext->sample_fmt,
        outAudioCodecContext->sample_rate,
        av_get_default_channel_layout(inAudioCodecContext->channels),
        inAudioCodecContext->sample_fmt,
        inAudioCodecContext->sample_rate,
        0,
        nullptr);
    if (!resampleContext) {
        cerr << "Cannot allocate the resample context" << endl;
        exit(1);
    }
    if ((swr_init(resampleContext)) < 0) {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&resampleContext);
        exit(1);
    }

    //while (true) {
    while (screen->getMagic() < magicNumber) { //a --> variabile stop solo per video e funzione !ShouldStopAudio()  

        if (pauseSC) {
            cout << "Pause audio" << endl;
            // avformat_close_input(&inAudioFormatContext); //a
            // closedAudioRecording = true; //a
        }

        std::unique_lock<std::mutex> ul(*mux);
        cv->wait(ul, [this]() { return !pauseSC; });

        if (stopSC) {
            break;
        }

        /*Se e' entrato in pausa, l'input sara' stato chiuso, quindi viene riaperto*/
        // if (closedAudioRecording) { //a
        // #if WIN32
        //     if (deviceName == "") {
        //             deviceName = DS_GetDefaultDevice("a");
        //             if (deviceName == "") {
        //                 throw std::runtime_error("Fail to get default audio device, maybe no microphone.");
        //             }
        //         }
        //         deviceName = "audio=" + deviceName;
        //         audioInputFormat = av_find_input_format("dshow");
        //         //value = avformat_open_input(&inAudioFormatContext, "audio=Microfono (Realtek(R) Audio)", audioInputFormat, &audioOptions);
        //         value = avformat_open_input(&inAudioFormatContext, deviceName.c_str(), audioInputFormat, &audioOptions);
        //         if (value != 0) {
        //             cerr << "Error in opening input device (audio)" << endl;
        //             exit(-1);
        //         }
        // #elif linux
        //     deviceName = "default";
        //     value = avformat_open_input(&inAudioFormatContext, deviceName.c_str(), audioInputFormat, &audioOptions);
        //     if (value != 0) {
        //         cerr << "Error in opening input device (audio)" << endl;
        //         exit(-1);
        //     }        
        // #endif
        // closedAudioRecording = false;
            // }     

        ul.unlock();


        /* Prendiamo il prossimo frame di uno stream

        da inAudioFormatContext a inPacket*/
        if (av_read_frame(inAudioFormatContext, inPacket) >= 0 && inPacket->stream_index == audioStreamIndx) {
            //decode audio routing
            av_packet_rescale_ts(inPacket, inAudioFormatContext->streams[audioStreamIndx]->time_base, inAudioCodecContext->time_base);
            /* Forniamo un pacchetto grezzo come input al decoder

            Da inPacket a inAudioCodecContext*/
            if ((ret = avcodec_send_packet(inAudioCodecContext, inPacket)) < 0) {
                cout << "Cannot decode current audio packet " << ret << endl;
                continue;
            }

            while (ret >= 0) {
                /*Restituisce dati di output decodificati da un decodificatore.
                *
                Da inAudioCodecContext a rawFrame*/
                ret = avcodec_receive_frame(inAudioCodecContext, rawFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    cerr << "Error during decoding" << endl;
                    exit(1);
                }

                /*Setta lo start_time dello stream pari al pts del frame*/
                if (outAVFormatContext->streams[outAudioStreamIndex]->start_time <= 0) {
                    outAVFormatContext->streams[outAudioStreamIndex]->start_time = rawFrame->pts;
                }
                initConvertedSamples(&resampledData, outAudioCodecContext, rawFrame->nb_samples);

                swr_convert(resampleContext,
                    resampledData, rawFrame->nb_samples,
                    (const uint8_t**)rawFrame->extended_data, rawFrame->nb_samples);

                /*Aggiungiamo al buffer fifo dei sample audio di input convertiti*/
                add_samples_to_fifo(resampledData, rawFrame->nb_samples);

                //raw frame ready
                //av_init_packet(outPacket);
                outPacket = av_packet_alloc();
                outPacket->data = nullptr;
                outPacket->size = 0;

                const int frame_size = FFMAX(av_audio_fifo_size(fifo), outAudioCodecContext->frame_size);

                scaledFrame = av_frame_alloc();
                if (!scaledFrame) {
                    cerr << "Cannot allocate an AVPacket for encoded video" << endl;
                    exit(1);
                }
                /*Inizializziamo scaledFrame*/
                scaledFrame->nb_samples = outAudioCodecContext->frame_size;
                scaledFrame->channel_layout = outAudioCodecContext->channel_layout;
                scaledFrame->format = outAudioCodecContext->sample_fmt;
                scaledFrame->sample_rate = outAudioCodecContext->sample_rate;
                av_frame_get_buffer(scaledFrame, 0);

                while (fifo && av_audio_fifo_size(fifo) >= outAudioCodecContext->frame_size) {
                    /* Legge, dal primo paramatro, una certa quantità di dati (indicata dal terzo parametro)

                    da fifo a scaledFrame->data ??*/
                    ret = av_audio_fifo_read(fifo, (void**)(scaledFrame->data), outAudioCodecContext->frame_size);

                    scaledFrame->pts = frameCount * audio_st->time_base.den * 1024 / outAudioCodecContext->sample_rate;
                    frameCount++;
                    //Alternativa //a
                    // scaledFrame->pts = pts; 
                    // pts += scaledFrame->nb_samples; 


                    /*Forniamo un frame all'encoder.
                    Utilizzeremo, poi, avcodec_receive_packet() per recuperare i pacchetti di output memorizzati nel buffer.

                    Da ScaledFrame a encoder.
                    Da encoder a outAudioCodecContext*/
                    if (avcodec_send_frame(outAudioCodecContext, scaledFrame) < 0) {
                        cout << "Cannot encode current audio packet " << endl;
                        exit(1);
                    }
                    while (ret >= 0) {
                        /*Leggiamo dei dati codificati dall'encoder

                        Da outAudioCodecContext a outPacket*/
                        ret = avcodec_receive_packet(outAudioCodecContext, outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        else if (ret < 0) {
                            cerr << "Error during encoding" << endl;
                            exit(1);
                        }
                        /* Converte un campo valido di timing, all'interno di un pacchetto, da un timestamp
                       (indicato come secondo parametro) ad un altro (indicato come terzo parametro)*/
                        av_packet_rescale_ts(outPacket, outAudioCodecContext->time_base, outAVFormatContext->streams[outAudioStreamIndex]->time_base);

                        outPacket->stream_index = outAudioStreamIndex;

                        unique_lock<mutex> ulw(*write_lock);
                        //Effettuo conversione dei pts
                        ptsA = outPacket->pts / outAudioCodecContext->sample_rate;
                        ptsV = screen->getPtsV();

#ifdef __linux__
                        // cvw.wait(ulw, [this]() {return ptsA <= ptsV; });
#elif _WIN32
                        cvw->notify_one();
                        cvw->wait(ulw, [this]() {return ((ptsA - 2 <= ptsV) || end); }); //FIXME: stopSC invece di end?
#endif

//cout << outPacket << endl;
                        //outFile << "Scrivo AUDIO-PTS_TIME: " << ptsA << "\n" << endl;
                        cout << "\n Scrivo AUDIO-PTS_TIME: " << ptsA << endl;

                        //cout << outAVFormatContext << " " << outPacket << endl;
                        /*Scriviamo il pacchettoin un output media file assicurandoci un corretto interleaving.
                         Questa funzione eseguirà il buffering dei pacchetti internamente, assicurarandosi che i pacchetti
                         nel file di output siano correttamente interleavati nell'ordine di dts crescente.

                         Da outPacket a outAVFormatContext*/
                        if (av_interleaved_write_frame(outAVFormatContext, outPacket) != 0)
                            // era: if (av_write_frame(outAVFormatContext, outPacket) != 0)
                        {
                            cerr << "Error in writing audio frame" << endl;
                        }

                        ulw.unlock();

                        /*Libera il pacchetto*/
                        av_packet_unref(outPacket);
                    }
                    ret = 0;
                }

                /*Operazioni di liberazione di memoria*/
                av_frame_free(&scaledFrame);
                av_packet_unref(outPacket);

            }
        }

    }

#ifdef _WIN32
    unique_lock<mutex> ulw(*write_lock);
    end = true; //FIXME: stopSC invece di end?
    cvw->notify_one();
#endif

    //outFile.close();
}

int AudioRecorder::getPtsA() {
    return ptsA;
}


