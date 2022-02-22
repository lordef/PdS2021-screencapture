#include "ScreenRecorder.h"
#include <cassert>
#if linux
#include <X11/Xlib.h>
#endif
#include <qdebug.h>
#include <QMessageBox>



using namespace std;

/*Costruttore base*/
ScreenRecorder::ScreenRecorder() : pauseCapture(false), started(false), activeMenu(true), pts(0) {
    /*Funzione di FFMPEG per inizializzare libavdevice
    e registrare correttamente i device di input e output*/
    avdevice_register_all();
#if WIN32
    cropW = GetSystemMetrics(SM_CXSCREEN);
    cropH = GetSystemMetrics(SM_CYSCREEN);
#endif
#if linux
    Display* disp = XOpenDisplay(NULL);
    Screen* scrn = DefaultScreenOfDisplay(disp);
    cropH = scrn->height;
    cropW = scrn->width;
#endif
}
/*Costruttore con parametro stringa di destinazione*/
ScreenRecorder::ScreenRecorder(std::string RecPath) : pauseCapture(false), started(false), activeMenu(true), pts(0) {
    /*Funzione di FFMPEG per inizializzare libavdevice
    e registrare correttamente i device di input e output*/
    avdevice_register_all();
    /*Settaggio di parametri di default del recorder*/
    RecordingPath = RecPath;
    cropX = 0;
    cropY = 0;
#if linux
    Display* disp = XOpenDisplay(NULL);
    Screen* scrn = DefaultScreenOfDisplay(disp);
    cropH = scrn->height;
    cropW = scrn->width;
#endif
#if WIN32
    cropW = GetSystemMetrics(SM_CXSCREEN);
    cropH = GetSystemMetrics(SM_CYSCREEN);
#endif
}

/*Costruttore di copia*/
ScreenRecorder::ScreenRecorder(const ScreenRecorder& p1) : pauseCapture(p1.pauseCapture), started(p1.started), activeMenu(p1.activeMenu)
{
#if WIN32
    cropW = GetSystemMetrics(SM_CXSCREEN);
    cropH = GetSystemMetrics(SM_CYSCREEN);
#endif
#if linux
    Display* disp = XOpenDisplay(NULL);
    Screen* scrn = DefaultScreenOfDisplay(disp);
    cropH = scrn->height;
    cropW = scrn->width;
#endif
}

/*Distruttore*/
ScreenRecorder::~ScreenRecorder() {

    CloseRecorder();
}

/*==================================== VIDEO ==============================*/

/*Funzione che inizializza le strutture dati richieste e apre l'input video*/
int ScreenRecorder::OpenVideoDevice() {
    value = 0;
    options = nullptr;
    pAVFormatContext = nullptr;

    /*Alloca pAVFormatContext, che è un AVFormatContext*/
    pAVFormatContext = avformat_alloc_context();

    /*Settaggio delle dimensioni di registrazione SU LINUX*/
#if linux
    if (cropW==0 || cropH ==0){
        Display* disp = XOpenDisplay(0);
        Screen* scrn = DefaultScreenOfDisplay(disp);
        cropH = scrn->height;
        cropW = scrn->width;
    }
    dimension = to_string(cropW) + "x" + to_string(cropH);
#else
std::string dimension = to_string(cropW) + "x" + to_string(cropH);
#endif
    av_dict_set(&options, "video_size", dimension.c_str(), 0);  

    /*Settaggio di altre ulteriori impostazioni relative 
    al video mediante l'aggiunta di entry nel dizionario
    delle options*/

    value = av_dict_set(&options, "probesize", "60M", 0);
    if (value < 0) {
        SetError( "Error in setting probesize value");
        exit(-1);
    }
    value = av_dict_set(&options, "rtbufsize", "2048M", 0);
    if (value < 0) {
        SetError( "Error in setting bufsize value");
        exit(-1);
    }
    value = av_dict_set(&options, "offset_x", "0", 0);
    if (value < 0) {
        SetError( "Error in setting offset x value");
        exit(-1);
    }
    value = av_dict_set(&options, "offset_y", "0", 0);
    if (value < 0) {
        SetError( "Error in setting offset y value");
        exit(-1);
    }


/*Settaggio delle dimensioni di registrazione dello schermo su Windows.
Usando Screen-capture-recorder, come da indicazioni nella documentazione dello stesso,
e' necessario settare delle chiavi di registro corrispondenti*/
#ifdef _WIN32
    SetCaptureSystemKey(cropX, TEXT("start_x"));
    SetCaptureSystemKey(cropY, TEXT("start_y"));
    SetCaptureSystemKey(cropW, TEXT("capture_width"));
    SetCaptureSystemKey(cropH, TEXT("capture_height"));
   
    /*Apertura dello stream di input*/
    pAVInputFormat = av_find_input_format("dshow");
    if (avformat_open_input(&pAVFormatContext, "video=screen-capture-recorder", pAVInputFormat, &options) != 0) {
        SetError( "Couldn't open input stream");
        exit(-1);
    }


#elif defined linux
    /*Settaggio delle dimensioni di registrazione dello schermo SU LINUX*/
    //stringa per settare il punto iniziale di registrazione in alto a sinistra
    url = ":0.0+" + to_string(cropX) + "," + to_string(cropY); 
    pAVInputFormat = const_cast<AVInputFormat*> (av_find_input_format("x11grab"));
    value = avformat_open_input(&pAVFormatContext, url.c_str(), pAVInputFormat, &options);

    if (value != 0) {
        SetError( "Error in opening input device (video)");
        exit(-1);
    }
#endif

    //Imposta il framerate a 30 frame al secondo

    value = av_dict_set(&options, "framerate", "30", 0);
    if (value < 0) {
        SetError( "Error in setting dictionary value (setting framerate)");
        exit(-1);
    }
    /*Preset di qualità*/
    value = av_dict_set(&options, "preset", "medium", 0);
    if (value < 0) {
        SetError( "Error in setting dictionary value (setting preset value)");
        exit(-1);
    }


    //Ottiene le informazioni sullo stream video dal context pAVFormatContext
    value = avformat_find_stream_info(pAVFormatContext, &options);
    if (value < 0) {
        SetError( "Error in retrieving the stream info");
        exit(-1);
    }

    VideoStreamIndx = -1;
    /*Cerca lo stream video con codec corrispondente a quello contenuto nel pAVFormatContext*/
    for (int i = 0; i < pAVFormatContext->nb_streams; i++) {
        if (pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            VideoStreamIndx = i;
            break;
        }
    }
    if (VideoStreamIndx == -1) {
        SetError( "Error: unable to find video stream index");
        exit(-2);
    }

    /*Ottiene e setta i valori di pAVCodecContex sulla base del codec utilizzato*/
    /*Cerca il decoder e ne ottiene il codec*/
    pAVCodec = const_cast<AVCodec*>(avcodec_find_decoder(pAVFormatContext->streams[VideoStreamIndx]->codecpar->codec_id));
    /*Alloca il Codec Context*/
    pAVCodecContext = avcodec_alloc_context3(pAVCodec);
    pCodecParameters = pAVFormatContext->streams[VideoStreamIndx]->codecpar;
    /*Memorizza in CodecContext i parametri dati dal codec rilevato*/
    value = avcodec_parameters_to_context(pAVCodecContext, pCodecParameters);
    cout << "pAVCodec is: " << pAVCodec ;
    if (pAVCodec == nullptr) {
        SetError( "Error: unable to find decoder video");
        exit(-1);
    }
    if (value < 0)
    {
        cout << "\nUnable to set the parameter of the codec" <<endl;
        exit(1);
    }
    return 0;
}

/*==========================================  AUDIO  ============================*/


/*Funzione analoga a quella video che inizializza le strutture dati e apre l'input audio*/
int ScreenRecorder::OpenAudioDevice() {
    audioOptions = nullptr;
    inAudioFormatContext = nullptr;

    inAudioFormatContext = avformat_alloc_context();
    /*Setta le options con valori quali il sample rate e altri*/
    value = av_dict_set(&audioOptions, "sample_rate", "44100", 0);
    if (value < 0) {
        SetError( "Error: cannot set audio sample rate");
        exit(-1);
    }
    value = av_dict_set(&audioOptions, "async", "1", 0);
    if (value < 0) {
        SetError( "Error: cannot set audio sample rate");
        exit(-1);
    }

/*Apre l'input audio in maniera differenziata su Linux e su Windows*/
#if defined linux
    audioInputFormat = const_cast<AVInputFormat*>(av_find_input_format("alsa"));
    value = avformat_open_input(&inAudioFormatContext, "hw:0", audioInputFormat, &audioOptions);
    if (value != 0) {
        SetError( "Error in opening input device (audio)");
        exit(-1);
    }
#endif

#if defined _WIN32
    audioInputFormat = av_find_input_format("dshow");
    value = avformat_open_input(&inAudioFormatContext, "audio=Microphone Array (Realtek(R) Audio)", audioInputFormat, &audioOptions);
    if (value != 0) {
        SetError( "Error in opening input device (audio)");
        exit(-1);
    }
#endif

    /*Legge pacchetti dello stream per ottenerne informazioni*/
    value = avformat_find_stream_info(inAudioFormatContext, nullptr);
    if (value != 0) {
        SetError( "Error: cannot find the audio stream information");
        exit(-1);
    }

    audioStreamIndx = -1;
    /*Cerca tra gli stream di input attivi, quello il cui codec sia corrisopondente ad AVMEDIA_TYPE_AUDIO*/
    for (int i = 0; i < inAudioFormatContext->nb_streams; i++) {
        if (inAudioFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndx = i;
            break;
        }
    }
    if (audioStreamIndx == -1) {
        SetError( "Error: unable to find audio stream index");
        exit(-2);
    }
    return(0);
}

/*Funzione per la creazione del file di output*/
int ScreenRecorder::InitOutputFile() {
    value = 0;

    outAVFormatContext = nullptr;
    /*Sulla base del path del file, cerca di "indovinare" il formato del file (mp4)*/
    outputAVFormat = const_cast<AVOutputFormat*>(av_guess_format(nullptr, RecordingPath.data(), nullptr));
    if (outputAVFormat == nullptr) {
        SetError( "Error in guessing the video format, try with correct format");
        exit(-5);
    }
    
    /*Alloca il context del file di output sulla base di outputAVFormat ottenuto prima*/
    avformat_alloc_output_context2(&outAVFormatContext, outputAVFormat, outputAVFormat->name, RecordingPath.data());
    if (outAVFormatContext == nullptr) {
        SetError( "Error in allocating outAVFormatContext");
        exit(-4);
    }


    /*===========================================================================*/
    /*Crea gli stream video e (se richiesto) audio*/
    GenerateVideoStream();
if(recordAudio)
    GenerateAudioStream();
    /*Crea un file video vuoto*/
    if (!(outAVFormatContext->flags & AVFMT_NOFILE)) {
        if (avio_open2(&outAVFormatContext->pb, RecordingPath.data(), AVIO_FLAG_WRITE, nullptr, nullptr) < 0) {
            SetError( "Error in creating the video file");
            exit(-10);
        }
    }

    /*Controlla che il file di output contenga almeno uno stream*/
    if (outAVFormatContext->nb_streams == 0) {
        SetError( "Output file does not contain any stream");
        exit(-11);
    }
    /*Alloca i dati dello stream e scrive l'header dello stream al file di output.*/
    value = avformat_write_header(outAVFormatContext, &options);
    if (value < 0) {
        SetError( "Error in writing the header context");
        exit(-12);
    }
    /*Variabile utile al distruttore per chiudere correttamente il file*/
    started = true;
    return 0;
}

/*===================================  VIDEO  ==================================*/

/*genera lo stream video*/
void ScreenRecorder::GenerateVideoStream() {

    /*Salva in outVideoCodec il codec per il formato utilizzato, in questo caso MPEG4*/
    outVideoCodec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_MPEG4));  //AV_CODEC_ID_MPEG4
    if (outVideoCodec == nullptr) {
        SetError( "Error in finding the AVCodec, try again with the correct codec");
        exit(-8);
    }
    
    /*Alloca outVideoCodecContext e setta i valori di default*/
    outVideoCodecContext = avcodec_alloc_context3(outVideoCodec);
    if (outVideoCodecContext == nullptr) {
        SetError( "Error in allocating the codec context");
        exit(-7);
    }

    /*Genera effettivamente lo stream video partendo dal codec e dal context prima definiti*/
    videoSt = avformat_new_stream(outAVFormatContext, outVideoCodec);
    if (videoSt == nullptr) {
        SetError( "Error in creating AVFormatStream");
        exit(-6);
    }

    //Imposta proprietà dello stream video, tra cui il codec, il formato dei pixel, il bitrate
    avcodec_parameters_to_context(outVideoCodecContext, videoSt->codecpar);
    outVideoCodecContext->codec_id = AV_CODEC_ID_MPEG4;
    outVideoCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outVideoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    outVideoCodecContext->bit_rate = 10000000;
    outVideoCodecContext->width = cropW; 
    outVideoCodecContext->height = cropH; 
    outVideoCodecContext->gop_size = 10;
    outVideoCodecContext->global_quality = 500;
    outVideoCodecContext->max_b_frames = 2;
    outVideoCodecContext->time_base.num = 1;
    outVideoCodecContext->time_base.den = 33;
    outVideoCodecContext->bit_rate_tolerance = 400000;

    /*Settaggi di default*/
    if (outVideoCodecContext->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(outVideoCodecContext->priv_data, "preset", "slow", 0);
    }

    if (outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        outVideoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    /*Inizializza il codec context del video di output sulla base del codec del video di output*/
    value = avcodec_open2(outVideoCodecContext, outVideoCodec, nullptr);
    if (value < 0) {
        SetError( "Error in opening the AVCodec");
        exit(-9);
    }

    outVideoStreamIndex = -1;
    /*Cerca lo stream di output basandosi su quale abbia come codec_type AVMEDIA_TYPE_UNKOWN*/
    for (int i = 0; i < outAVFormatContext->nb_streams; i++) {
        if (outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            outVideoStreamIndex = i;
        }
    }
    if (outVideoStreamIndex < 0) {
        SetError( "Error: cannot find a free stream index for video output");
        exit(-1);
    }
    /*Salva nelle relative strutture i parametri ottenuti a partire dal context*/
    avcodec_parameters_from_context(outAVFormatContext->streams[outVideoStreamIndex]->codecpar, outVideoCodecContext);
}

/*===============================  AUDIO  ==================================*/

/*Genera lo stream audio. Per la maggior parte sono funzioni analoghe e parallele
a quelle dello stream video*/
void ScreenRecorder::GenerateAudioStream() {
    AVCodecParameters* params = inAudioFormatContext->streams[audioStreamIndx]->codecpar;
    inAudioCodec = const_cast<AVCodec*>(avcodec_find_decoder(params->codec_id));
    if (inAudioCodec == nullptr) {
        SetError( "Error: cannot find the audio decoder");
        exit(-1);
    }

    inAudioCodecContext = avcodec_alloc_context3(inAudioCodec);
    if (avcodec_parameters_to_context(inAudioCodecContext, params) < 0) {
        cout << "Cannot create codec context for audio input" <<endl;
    }

    value = avcodec_open2(inAudioCodecContext, inAudioCodec, nullptr);
    if (value < 0) {
        SetError( "Error: cannot open the input audio codec");
        exit(-1);
    }

    //Genera lo stream audio
    outAudioCodecContext = nullptr;
    outAudioCodec = nullptr;
    int i;

    AVStream* audio_st = avformat_new_stream(outAVFormatContext, nullptr);
    if (audio_st == nullptr) {
        SetError( "Error: cannot create audio stream");
        exit(1);
    }

    outAudioCodec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_AAC));
    if (outAudioCodec == nullptr) {
        SetError( "Error: cannot find requested encoder");
        exit(1);
    }

    outAudioCodecContext = avcodec_alloc_context3(outAudioCodec);
    if (outAudioCodecContext == nullptr) {
        SetError( "Error: cannot create related VideoCodecContext");
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

    /*Setta paramtri di default*/
    outAudioCodecContext->codec_id = AV_CODEC_ID_AAC;
    outAudioCodecContext->sample_fmt = (outAudioCodec)->sample_fmts ? (outAudioCodec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outAudioCodecContext->channels = inAudioCodecContext->channels;
    outAudioCodecContext->channel_layout = av_get_default_channel_layout(outAudioCodecContext->channels);
    outAudioCodecContext->bit_rate = 96000;
    outAudioCodecContext->time_base = { 1, inAudioCodecContext->sample_rate };

    outAudioCodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    if ((outAVFormatContext)->oformat->flags & AVFMT_GLOBALHEADER) {
        outAudioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outAudioCodecContext, outAudioCodec, nullptr) < 0) {
        SetError( "error in opening the avcodec");
        exit(1);
    }

    outAudioStreamIndex = -1;
    /*Trova lo stream basandosi su quale ha il codec type AVMEDIA_TYPE_UNKNOWN*/
    for (i = 0; i < outAVFormatContext->nb_streams; i++) {
        if (outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            outAudioStreamIndex = i;
        }
    }
    if (outAudioStreamIndex < 0) {
        SetError( "Error: cannot find a free stream for audio on the output");
        exit(1);
    }

    avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outAudioCodecContext);
}


/*Inizializza la coda FIFO per i sample audio*/
int ScreenRecorder::InitFifo()
{
    /* Crea la coda FIFO per i sample audio basandosi sul codec audio. */
    if (!(fifo = av_audio_fifo_alloc(outAudioCodecContext->sample_fmt,
        outAudioCodecContext->channels, outAudioCodecContext->sample_rate))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

int ScreenRecorder::AddSamplesToFifo(uint8_t** converted_input_samples, const int frame_size) {
    int error;
    /*Espande la FIFO per consentire di tenere sia i vecchi 
    che i nuovi sample.*/
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /* Salva i nuovi sample nella FIFO */
    if (av_audio_fifo_write(fifo, (void**)converted_input_samples, frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

int ScreenRecorder::InitConvertedSamples(uint8_t*** converted_input_samples,
    AVCodecContext* output_codec_context,
    int frame_size) {
    int error;
    /* Alloca un puntatore per ogni canale audio. 
    * Ogni puntatore puntera' ai sample audio dei corrispondenti
    * canali (puo' essere NULL per formati interleaved)    
    */
    if (!(*converted_input_samples = (uint8_t**)calloc(output_codec_context->channels,
        sizeof(**converted_input_samples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    /*Alloca memoria per i sample di tutti i canali in un blocco di memoria consecutivo*/
    if (av_samples_alloc(*converted_input_samples, nullptr,
        output_codec_context->channels,
        frame_size,
        output_codec_context->sample_fmt, 0) < 0) {

        exit(1);
    }
    return 0;
}

/*Funzione per la cattura dell'audio*/
void ScreenRecorder::CaptureAudio() {
    int ret;
    AVPacket* inPacket, * outPacket;
    AVFrame* rawFrame, * scaledFrame;
    uint8_t** resampledData;
    /*inizializza la fifo*/
    InitFifo();
  
    /*Alloca un pacchetto*/
    inPacket = av_packet_alloc();
    if (!inPacket) {
        SetError( "Cannot allocate an AVPacket for encoded video");
        exit(1);
    }
    /*Alloca un frame*/
    rawFrame = av_frame_alloc();
    if (!rawFrame) {
        SetError( "Cannot allocate an AVPacket for encoded video");
        exit(1);
    }
    /*Alloca un frame*/
    scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        SetError( "Cannot allocate an AVPacket for encoded video");
        exit(1);
    }
    /*Alloca un pacchetto*/
    outPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
    if (!outPacket) {
        SetError( "Cannot allocate an AVPacket for encoded video");
        exit(1);
    }

    /*Inizializza il resampler*/
    /*Crea il context del resampler*/
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
        SetError( "Cannot allocate the resample context");
        exit(1);
    }
    if ((swr_init(resampleContext)) < 0) {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&resampleContext);
        exit(1);
    }

    /*Ciclo al cui interno avviene la lettura dei frame
    e il loro processamento. Questo ciclo continua
    finche' non viene settata a true la variabile
    stopCaptureAudio*/
    while (!ShouldStopAudio()) {
        /*Controllo se la variabile pauseCapture è stata settata a true*/
        if (pauseCapture) {
            /*In tal caso, chiudo l'input per riaprirlo alla fine della pausa
            per garantire una corretta sincronizzazione*/
            avformat_close_input(&inAudioFormatContext);
            closedAudioRecording = true;
        }

        /*Unique lock per effettuare la cv.wait sulla pauseCapture*/
        std::unique_lock<std::mutex> ul(mu);
        
        cv.wait(ul, [this]() {return !pauseCapture; });
        if (ShouldStopAudio()) {
            break;
        }
        /*Se e' entrato in pausa, l'input sara' stato chiuso, quindi viene riaperto*/
        if (closedAudioRecording) {
#if WIN32
            avformat_open_input(&inAudioFormatContext, "audio=Microphone Array (Realtek(R) Audio)", audioInputFormat, &audioOptions);
#elif linux
            avformat_open_input(&inAudioFormatContext, "hw:0", audioInputFormat, &audioOptions);
#endif
            closedAudioRecording = false;
        }
        ul.unlock();
        /*Effettua la lettura del frame*/
        if (av_read_frame(inAudioFormatContext, inPacket) >= 0 && inPacket->stream_index == audioStreamIndx) {
            /* Converte un campo valido di timing, all'interno di un pacchetto (primo parametro), da un timestamp
             (indicato come secondo parametro) ad un altro (indicato come terzo parametro)*/
            av_packet_rescale_ts(outPacket, inAudioFormatContext->streams[audioStreamIndx]->time_base, inAudioCodecContext->time_base);
            /*Invia il pacchetto col timestamp convertito*/
            if ((ret = avcodec_send_packet(inAudioCodecContext, inPacket)) < 0) {
                cout << "Cannot decode current audio packet " << ret ;
                continue;
            }

            while (ret >= 0) {
                /* Ottiene i dati di output decodificati da un decodificatore.*/
                ret = avcodec_receive_frame(inAudioCodecContext, rawFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    SetError( "Error during decoding");
                    exit(1);
                }
                /*Setta lo start_time dello stream pari al pts del frame*/
                if (outAVFormatContext->streams[outAudioStreamIndex]->start_time <= 0) {
                    outAVFormatContext->streams[outAudioStreamIndex]->start_time = rawFrame->pts;
                }
                InitConvertedSamples(&resampledData, outAudioCodecContext, rawFrame->nb_samples);

                swr_convert(resampleContext,
                    resampledData, rawFrame->nb_samples,
                    (const uint8_t**)rawFrame->extended_data, rawFrame->nb_samples);


                /*Aggiungiamo al buffer fifo dei sample audio di input convertiti*/
                AddSamplesToFifo(resampledData, rawFrame->nb_samples);

                outPacket = av_packet_alloc();
                outPacket->data = nullptr;
                outPacket->size = 0;

                const int frame_size = FFMAX(av_audio_fifo_size(fifo), outAudioCodecContext->frame_size);

                /*alloca il frame*/
                scaledFrame = av_frame_alloc();
                if (!scaledFrame) {
                    SetError( "Cannot allocate an AVPacket for encoded video");
                    exit(1);
                }

                /*Setta valori per il frame appena allocato*/
                scaledFrame->nb_samples = outAudioCodecContext->frame_size;
                scaledFrame->channel_layout = outAudioCodecContext->channel_layout;
                scaledFrame->format = outAudioCodecContext->sample_fmt;
                scaledFrame->sample_rate = outAudioCodecContext->sample_rate;
                av_frame_get_buffer(scaledFrame, 0);

                /*Legge sample dalla coda fifo*/
                while (fifo && av_audio_fifo_size(fifo) >=0 && av_audio_fifo_size(fifo) >= outAudioCodecContext->frame_size) {
                    ret = av_audio_fifo_read(fifo, (void**)(scaledFrame->data), outAudioCodecContext->frame_size);
                    scaledFrame->pts = pts;
                    pts += scaledFrame->nb_samples;
                    /*invia il frame al codificatore*/
                    if (avcodec_send_frame(outAudioCodecContext, scaledFrame) < 0) {
                        cout << "Cannot encode current audio packet " ;
                        exit(1);
                    }
                    while (ret >= 0) {
                        /*Riceve un pacchetto decodificato dal decodificatore*/
                        ret = avcodec_receive_packet(outAudioCodecContext, outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        else if (ret < 0) {
                            SetError( "Error during encoding");
                            exit(1);
                        }

                        /* Converte un campo valido di timing, all'interno di un pacchetto (primo parametro), da un timestamp
                        (indicato come secondo parametro) ad un altro (indicato come terzo parametro)*/
                        av_packet_rescale_ts(outPacket, outAudioCodecContext->time_base, outAVFormatContext->streams[outAudioStreamIndex]->time_base);

                        outPacket->stream_index = outAudioStreamIndex;

                        /*Ottiene unique_lock per poter scrivere il frame*/
                        unique_lock<mutex> ulw(write_lock);
                        if (av_interleaved_write_frame(outAVFormatContext, outPacket) != 0)
                        {
                            SetError( "Error in writing audio frame");
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
}

/*Funzione per la cattura del video*/
int ScreenRecorder::CaptureVideoFrames() {
    int64_t pts = 0;
    int flag;
    int frameFinished = 0;
    int numPause = 0;
    AVFrame* croppedFrame;
#if linux
    string screenRes;
#endif

    /*file di log per debug*/
    ofstream outFile{ "..\\media\\log.txt", ios::out };

    int frameIndex = 0;
    value = 0;
    /*Come nell'audio, alloca pacchetti e frame*/
    pAVPacket = av_packet_alloc();
    if (pAVPacket == nullptr) {
        SetError( "Error in allocating AVPacket");
        exit(-1);
    }

    pAVFrame = av_frame_alloc();
    if (pAVFrame == nullptr) {
        SetError( "Error: unable to alloc the AVFrame resources");
        exit(-1);
    }

    outFrame = av_frame_alloc();
    if (outFrame == nullptr) {
        SetError( "Error: unable to alloc the AVFrame resources for out frame");
        exit(-1);
    }

    int videoOutBuffSize;
    /*Ottiene il numero di byte richiesti per allocare il video dati i parametri forniti*/
    int nBytes = av_image_get_buffer_size(outVideoCodecContext->pix_fmt, outVideoCodecContext->width, outVideoCodecContext->height, 32);
    uint8_t* videoOutBuff = (uint8_t*)av_malloc(nBytes);

    if (videoOutBuff == nullptr) {
        SetError( "Error: unable to allocate memory");
        exit(-1);
    }
    /* Si effettua un setup dei data pointers e delle linesizes in base alle specifiche dell'immagine e
     * all'array fornito.
     * La funzione ritorna la dimensione in byte richiesta per videoOutBuff oppure un valore minore di zero in caso di errore.
     *
     * SIGNIFICATO PARAMETRI:
     * "outFrame->data" : data pointers da compilare
     * "outFrame->linesize" : linesize per l'immagine in outFrame->data da compilare
     * "videoOutBuff": buffer che contiene o conterrà i dati dell'immagine effettivi, può essere NULL
     * "AV_PIX_FMT_YUV420P" : formato pixel dell'immagine
     * "outAVCodecContext->width" e "outAVCodecContext->height": larghezza e altezza dell'immagine in pixel
     * 1: valore usato per allineare le linesizes
     */
    value = av_image_fill_arrays(outFrame->data, outFrame->linesize, videoOutBuff, AV_PIX_FMT_YUV420P, outVideoCodecContext->width, outVideoCodecContext->height, 1);
    if (value < 0) {
        SetError( "Error in filling image array");
    }

    /*Context SWS, per il riscalamento dell'immagine*/
    SwsContext* swsCtx_;

    if (!(swsCtx_ = sws_alloc_context()))
    {
        cout << "\nError nell'allocazione del SwsContext";
        exit(1);
    }
    value = sws_init_context(swsCtx_, NULL, NULL);
    if (value < 0)
    {
        SetError("\nErrore nell'inizializzazione del SwsContext");
        exit(1);
    }


    swsCtx_ = sws_getCachedContext(swsCtx_,
        pAVCodecContext->width,
        pAVCodecContext->height,
        pAVCodecContext->pix_fmt,
        outVideoCodecContext->width,
        outVideoCodecContext->height,
        outVideoCodecContext->pix_fmt,
        SWS_BILINEAR, NULL, NULL, NULL);


    if (avcodec_open2(pAVCodecContext, pAVCodec, &options) < 0) {
        SetError( "Could not open codec");
        exit(-1);
    }


    AVPacket* outPacket;
    int gotPicture;

    time_t startTime;
    time(&startTime);

    /*Funzionamento analogo a quello dell'audio*/
    while (!ShouldStopVideo()) {

        if (pauseCapture) {
            outFile << "///////////////////   Pause  ///////////////////" << endl;
            avformat_close_input(&pAVFormatContext);
            closedVideoRecording = true;
        }
        std::unique_lock<std::mutex> ul(mu);
        cv.wait(ul, [this]() { return !pauseCapture; });   //pause capture (not busy waiting)

        if (ShouldStopVideo()) {  //check if the capture has to stop
            break;
        }

        if (closedVideoRecording) {
#if WIN32
            avformat_open_input(&pAVFormatContext, "video=screen-capture-recorder", pAVInputFormat, &options);
#elif linux
            dimension = to_string(cropW) + "x" + to_string(cropH);
            av_dict_set(&options, "video_size", dimension.c_str(), 0);   //option to set the dimension of the screen section to record
            avformat_open_input(&pAVFormatContext, url.c_str(), pAVInputFormat, &options);
#endif
            closedVideoRecording = false;
        }
        ul.unlock();
        if (av_read_frame(pAVFormatContext, pAVPacket) >= 0 && pAVPacket->stream_index == VideoStreamIndx) {
            av_packet_rescale_ts(pAVPacket, pAVFormatContext->streams[VideoStreamIndx]->time_base, pAVCodecContext->time_base);

                value = avcodec_send_packet(pAVCodecContext, pAVPacket);
                if (value < 0) {
                    cout << AVERROR(value);
                    cout << "Unable to decode video" << endl;
                }

                value = avcodec_receive_frame(pAVCodecContext, pAVFrame);
                cout << "\nFrame: " << pAVCodecContext->frame_number << "\n";
                if (value == AVERROR(EAGAIN) || value == AVERROR_EOF) {
                    cout << "\nOutput not available in this state.  Try to send new input. " <<endl;
                }
                else if (value < 0)
                {
                    SetError("\nError during decoding");
                    exit(1);
                }

                value = sws_scale(swsCtx_, pAVFrame->data, pAVFrame->linesize, 0, pAVFrame->height, outFrame->data, outFrame->linesize);


                if (value < 0) {
                    SetError("\nProblem with sws_scale ");
                    exit(1);
                }
                //Frame decodificato con successo
                outPacket = av_packet_alloc();
                outPacket->data = nullptr;
                outPacket->size = 0;

                outFrame->width = outVideoCodecContext->width;
                outFrame->height = outVideoCodecContext->height;
                outFrame->format = outVideoCodecContext->pix_fmt;

                value = avcodec_send_frame(outVideoCodecContext, outFrame);
                if (value < 0)
                {
                    cout << "\nError sending a frame for encoding. ERROR CODE: " << value;
                    continue;
                }

                value = avcodec_receive_packet(outVideoCodecContext, outPacket); //Legge i dati codificati dall'encoder.
                if (value == AVERROR(EAGAIN))
                {
                    cout << "\nOutput not available in this state.  Try to send new input";
                    continue;
                }
                else if (value < 0 && value != AVERROR_EOF)
                {
                    SetError("\nError during encoding");
                    exit(1);
                }

                if (value >= 0) {
                    /*Riscala usando i pts (presentation time stamp) e i dts (decompression time stamp)*/
                    if (outPacket->pts != AV_NOPTS_VALUE) {
                        outPacket->pts = av_rescale_q(outPacket->pts, outVideoCodecContext->time_base, videoSt->time_base);
                    }
                    if (outPacket->dts != AV_NOPTS_VALUE) {
                        outPacket->dts = av_rescale_q(outPacket->dts, outVideoCodecContext->time_base, videoSt->time_base);
                    }



                    outFile << "outPacket->duration: " << outPacket->duration << ", " << "pAVPacket->duration: " << pAVPacket->duration << endl;
                    outFile << "outPacket->pts: " << outPacket->pts << ", " << "pAVPacket->pts: " << pAVPacket->pts << endl;
                    outFile << "outPacket.dts: " << outPacket->dts << ", " << "pAVPacket->dts: " << pAVPacket->dts << endl;

                    
                    /*Acquisisco write lock per scrivere il frame video sul file*/
                    unique_lock<mutex> ulw(write_lock);
                    if (av_write_frame(outAVFormatContext, outPacket) != 0) {
                        cout <<  "Error in writing video frame" << endl;
                    }
                    ulw.unlock();
                    /*Una volta scritto il frame, libero il pacchetto*/
                    av_packet_free(&outPacket);
                }
                /*Libero la memoria*/
                av_packet_free(&outPacket);
                av_packet_free(&pAVPacket);
                pAVPacket = av_packet_alloc();
                if (!pAVPacket)
                    exit(1);

                av_frame_free(&pAVFrame);
                pAVFrame = av_frame_alloc();
                if (!pAVFrame) // Verifichiamo che l'operazione svolta da "av_frame_alloc()" abbia avuto successo
                {
                    SetError("\nUnable to release the avframe resources");
                    exit(1);
                }

                av_frame_free(&outFrame);
                outFrame = av_frame_alloc();
                if (!outFrame)
                {
                    SetError("\nUnable to release the avframe resources for outframe");
                    exit(1);
                }
                value = av_image_fill_arrays(outFrame->data, outFrame->linesize, videoOutBuff, AV_PIX_FMT_YUV420P, outVideoCodecContext->width, outVideoCodecContext->height, 1);
                if (value < 0) // Verifico che non ci siano errori
                {
                    SetError("\nError in filling image array");
                    exit(value);
                }
            }
    }
    
    /*Una volta conclusa la registrazione, libera le strutture di memoria e chiude il file*/

    av_packet_free(&outPacket);

    value = av_write_trailer(outAVFormatContext);
    /*
     * Scrive il trailer dello stream in un file multimediale di output e
     * libera i dati privati ​​del file. Se non ci sono stati errori, ritorna 0.
     */
    if (value < 0)
    {
        SetError("\nError in writing av trailer");
        exit(1);
    }

    outFile.close();


    av_packet_free(&pAVPacket);
    sws_freeContext(swsCtx_);
    av_frame_free(&pAVFrame);
    av_frame_free(&outFrame);

    av_free(videoOutBuff);

    started = false;
    CloseRecorder();

    return 0;
}

/*Funzione che inizializza i thread per la registrazione video (e audio, se richiesta)*/
void ScreenRecorder::CreateThreads() {
    videoThread = std::thread(&ScreenRecorder::CaptureVideoFrames, this);
    if (recordAudio) {
        audioThread = std::thread(&ScreenRecorder::CaptureAudio, this);
    }
}

/*Funzione wrapper che chiama altre funzioni per il setup dello screen recorder*/
void ScreenRecorder::SetUpScreenRecorder() {
    OpenVideoDevice();
    OpenAudioDevice();
    InitOutputFile();
    CreateThreads();
}

/*Funzione per la chiusura della registrazione*/
void ScreenRecorder::StopRecording() {
    if (recordAudio) {
        /*Se stava registrando l'audio, setta la variabile stopCaptureAudio a true*/
        StopAudio();
        /*Eseguo il join del thread*/
        audioThread.join();
    }
    /*Setta la variabile stopCaptureVideo a true*/
    StopVideo();
    /*Esegue il join del thread*/
    videoThread.join();
    /*Se era in pausa, ne esce e risveglia le condition variable per consentire la conclusione del thread*/
    if (pauseCapture) {
        pauseCapture = false;
    }
        cv.notify_all();
}

/*Setta la pausa*/
void ScreenRecorder::PauseRecording()
{
    /*Inverte il valore della variabile pauseCapture e, se era attiva la pausa, sveglia i thread con la notify_all*/
    pauseCapture = !pauseCapture;
    if (!pauseCapture) {
        cv.notify_all();
    }
}

/*Funzione di utilità per la chiusura del recorder che chiude il file (se non chiuso precedentemente) e ripulisce le strutture allocate*/
void ScreenRecorder::CloseRecorder()
{
    if (started) {
            value = av_write_trailer(outAVFormatContext);
            if (value < 0) {
                SetError("Error in writing av trailer. Error code n: " + value);
                exit(-1);
            }
    }
    avformat_close_input(&inAudioFormatContext);
    if (inAudioFormatContext == nullptr) {
        cout << "inAudioFormatContext close successfully" << endl;
    }
    else {
        SetError("Error: unable to close the inAudioFormatContext");
        exit(-1);
        //throw "Error: unable to close the file";
    }

    avformat_free_context(inAudioFormatContext);
    if (inAudioFormatContext == nullptr) {
        cout << "AudioFormat freed successfully" << endl;
    }
    else {
        SetError( "Error: unable to free AudioFormatContext");
        exit(-1);
    }


    avformat_close_input(&pAVFormatContext);
    if (pAVFormatContext == nullptr) {
        cout << "File close successfully" << endl;
    }
    else {
        SetError( "Error: unable to close the file");
        exit(-1);
    }

    avformat_free_context(pAVFormatContext);
    if (pAVFormatContext == nullptr) {
        cout << "VideoFormat freed successfully" << endl;
    }
    else {
        SetError( "Error: unable to free VideoFormatContext");
        exit(-1);
    }

    avio_closep(&outAVFormatContext->pb);
}

/*Funzione per impostare la stringa col messaggio di errore in accesso esclusivo*/
void ScreenRecorder::SetError(std::string error)
{
    std::unique_lock lk(error_lock);
    error_msg = error;
}
/*Funzione per leggere la stringa col messaggio di errore in accesso esclusivo*/
std::string ScreenRecorder::GetErrorString()
{
    std::unique_lock lk(error_lock);
    std::string returnValue = error_msg;
    return returnValue;
}

/*Funzione per leggere la variabile di stop dell'audio in accesso esclusivo*/
bool ScreenRecorder::ShouldStopAudio()
{
    bool return_value;
    std::unique_lock lk(stop_lockA);
    return_value = stopCaptureAudio;
    lk.unlock();
    return return_value;
}
/*Funzione per impostare lo stop dell'audio in accesso esclusivo*/
void ScreenRecorder::StopAudio()
{
    std::unique_lock lk(stop_lockA);
    stopCaptureAudio = true;
}
/*Funzione per leggere la variabile di stop del video in accesso esclusivo*/
bool ScreenRecorder::ShouldStopVideo()
{
    bool return_value;
    std::unique_lock lk(stop_lockV);
    return_value = stopCaptureVideo;
    lk.unlock();
    return return_value;
}
/*Funzione per impostare lo stop del video in accesso esclusivo*/
void ScreenRecorder::StopVideo()
{
    std::unique_lock lk(stop_lockV);
    stopCaptureVideo = true;
}

#if WIN32
void ScreenRecorder::SetCaptureSystemKey(int valueToSet, LPCWSTR keyToSet) {
    HKEY hKey;
    char hexString[20];
    _itoa_s(valueToSet, hexString, 16);
    DWORD value = strtoul(hexString, NULL, 16);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\screen-capture-recorder\\"), 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
            TEXT("SOFTWARE\\screen-capture-recorder\\"),
            0, NULL, 0,
            KEY_WRITE, NULL,
            &hKey, &value) != ERROR_SUCCESS) SetError("Errore nel settare la chiave di registro");
    RegSetValueEx(hKey, keyToSet, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
    RegCloseKey(hKey);
}
#endif
