#include "ScreenRecorder.h"
#include <cassert>

// #include <qdebug.h> //i
// #include <QMessageBox> //i

using namespace std;
// static int64_t last_pts = AV_NOPTS_VALUE; //utile?

/******* #TODO: Funzioni utili da spostare *****************/
#include <bits/stdc++.h>

#ifdef __linux__
#include <X11/Xlib.h> //useful lib installed: sudo apt install libx11-dev 
#include<tuple>

tuple<int, int> retrieveDisplayDimention()
{
	Display* disp = XOpenDisplay(NULL);
	Screen*  scrn = DefaultScreenOfDisplay(disp);
	int height = scrn->height;
	int width  = scrn->width;
	//cout << "\nStampa dimensioni display\n";
	//cout << width<<"x"<< height;
	
	return make_tuple(height, width);
	
}

#endif


/* Definiamo il COSTRUTTORE */
/* Initialize the resources*/
ScreenRecorder::ScreenRecorder() : pauseSC(false), stopSC(false),  activeMenu(true),
                                    magicNumber(300), cropX(0), cropY(0), cropH(1080), cropW(1920), frameCount(0), end (false)

// TODO: aggiustare codice seguente e sostituirlo a quello sopra                                    
// ScreenRecorder::ScreenRecorder( bool isAudioActive = true, 
//                                 int cropX = 0, int cropY = 0, int cropH = 1080, int cropW = 1920,
// 					            int magicNumber = 100, bool activeMenu = true) 

                                // : isAudioActive(isAudioActive), 
                                // cropX(cropX), cropY(cropY), cropH(cropH), cropW(cropW), 
                                // magicNumber(magicNumber), activeMenu(activeMenu)
//Aggiornato - magicNumber=3000
/* #TODO: N.B.: sia per linux che per windows controllare che i valori passati
             rispettino la risoluzione del pc su cui gira il codice
             in particolare che tutte le variabili di crop
             ispirarsi a libavdevice/xcbgrab.c -> cerca la stringa 'outside the screen'*/
{
    #ifdef __linux__
    /*****************/
        // #FIXME: codice temporaneo per debuggare su pc L e I, poiché risoluzioni diverse
        // int cropH, cropW; //height, width
        tie(cropH, cropW) = retrieveDisplayDimention(); //FIXME: commentare perché sovrascrive crop passati
    /*****************/
    #elif _WIN32
        //a
        // TODO: da testare
        // cropW = GetSystemMetrics(SM_CXSCREEN);
        // cropH = GetSystemMetrics(SM_CYSCREEN);
    #endif

    // av_register_all(); //Funzione di inizializzazione deprecata. Può essere tranquillamente omessa.
    // avcodec_register_all(); //Funzione di inzizializzazione deprecata. Può essere tranquillamente omessa.
    avdevice_register_all(); // Inizializza libavdevice e registra tutti i dispositivi di input e output.
    /* Set output path */
    //TODO: RecordingPath = RecPath; //a -> altro costruttore

    cout << "\nAll required functions are registered successfully\n";
}

/* Definiamo il DISTRUTTORE */
/* uninitialize the resources */
ScreenRecorder::~ScreenRecorder()
{
    // if (started) { //utile?
        
        /*value = av_write_trailer(outAVFormatContext); //a
        if (value < 0) {
            cerr << "Error in writing av trailer" << endl;
            exit(-1);
        }*/

        avformat_close_input(&pAVFormatContext); // Chiude un input AVFormatContext aperto: libera tutto e mette a NULL il contenuto del parametro ricevuto
        if (pAVFormatContext == nullptr) {
            cout << "File close successfully" << endl;
        }
        else {
            cerr << "Error: unable to close the file" << endl;
            exit(1);
            //throw "Error: unable to close the file";
        }

        avformat_free_context(pAVFormatContext); // Libera pAVFormatContext e tutti i suoi stream.
        if (pAVFormatContext == nullptr) {
            cout << "VideoFormat freed successfully" << endl;
        }
        else {
            cerr << "Error: unable to free VideoFormatContext" << endl;
            exit(1);
        }
    // } //end - started

}

/* Establishing the connection between camera or screen through its respective folder */
//int ScreenRecorder::openCamera() throw() //#FIXME
AVDictionary* ScreenRecorder::openVideoDevice()
{

    value = 0; // valore di ritorno per valutare esito delle operazioni
    options = nullptr;
    pAVFormatContext = nullptr;

    /* Alloca pAVFormatContext, che è un AVFormatContext */
    pAVFormatContext = avformat_alloc_context();

    /*****/ //utile?
    //#TODO; sezione utilizzata solo da linux - in Windows si agisce con le funzioni _itoa_s
    // string dimension = to_string(width) + "x" + to_string(height);
    //av_dict_set(&options, "video_size", dimension.c_str(), 0);   //option to set the dimension of the screen section to record
    //av_dict_set(&options, "video_size", "1920x1080", 0);   //option to set the dimension of the screen section to record
    /*****/

    value = av_dict_set(&options, "probesize", "60M", 0);
    if (value < 0) {
        cerr << "Error in setting probesize value" << endl;
        exit(-1);
    }

    value = av_dict_set(&options, "rtbufsize", "2048M", 0);
    if (value < 0) {
        cerr << "Error in setting probesize value" << endl;
        exit(-1);
    }

    value = av_dict_set(&options, "offset_x", "0", 0);
    if (value < 0) {
        cerr << "Error in setting offset x value" << endl;
        exit(-1);
    }

    value = av_dict_set(&options, "offset_y", "0", 0);
    if (value < 0) {
        cerr << "Error in setting offset y value" << endl;
        exit(-1);
    }

    /*Questa sezione di codice serve per selezionare la finestra del desktop da registrare. Per fare ciò vado a settare i registri nella maniera più opportuna
     * Con CropX e CropY indico le coordinate del vertice in alto a sinistra della finestra dello schermo che va registrata.
     * Con CropW e CropH vado ad indicare quanto deve essere grande questa finestra
     */
#ifdef _WIN32
    SetCaptureSystemKey(cropX, TEXT("start_x"));
    SetCaptureSystemKey(cropY, TEXT("start_y"));
    SetCaptureSystemKey(cropW, TEXT("capture_width"));
    SetCaptureSystemKey(cropH, TEXT("capture_height"));

    /*Apertura dello stream di input*/
    //pAVInputFormat = av_find_input_format("gdigrab"); //utile?
    pAVInputFormat = av_find_input_format("dshow"); //Uso dshow e non dgrab!!
    //if (avformat_open_input(&pAVFormatContext, "desktop", pAVInputFormat, &options) != 0) {
    if (avformat_open_input(&pAVFormatContext, "video=screen-capture-recorder", pAVInputFormat, &options) != 0) {
        cerr << "Couldn't open input stream" << endl;
        exit(-1);
    }

#elif __linux__

     /*****************/
     /*TODO: WORKING ON cropping del video - capire da ffmpeg: studiare opzioni da qui:*/
     /*
     - Link utile: https://ffmpeg.org/ffmpeg-devices.html#x11grab  sezione "3.21.1 Options" utile per il crop video
     - #TODO: N.B.:
         sia per linux che per windows
         controllare che i valori passati rispettino la risoluzione del pc su cui gira il codice
     */
     /*****************/
    if (cropW==0 || cropH ==0){
        //Set dimensione massima del display
        tie(cropH, cropW) = retrieveDisplayDimention();
    }
    string resolutionS = to_string(cropW) + "x" + to_string(cropH);

    //option to set the dimension of the screen section to record
    value = av_dict_set(&options, "video_size", resolutionS.c_str(), 0); // TODO: sezione utile a _WIN32 ?
    if (value < 0)
    {
        cout << "\nError in setting video_size values";
        exit(1);
    }

    //custom string to set the start point of the screen section
    // int offset_x = 0, offset_y = 0;
    string url = ":0.0+" + to_string(cropX) + "," + to_string(cropY);  

    pAVInputFormat = const_cast<AVInputFormat*>(av_find_input_format("x11grab")); //un dispositivo alternativo potrebbe essere xcbgrab, non testato       

    value = avformat_open_input(&pAVFormatContext, url.c_str(), pAVInputFormat, &options);
    if (value != 0) {
        cerr << "Error in opening input device (video)" << endl;
        exit(-1);
    }
#else //utile?

    value = av_dict_set(&options, "pixel_format", "0rgb", 0);
    if (value < 0) {
        cerr << "Error in setting pixel format" << endl;
        exit(-1);
    }

    value = av_dict_set(&options, "video_device_index", "1", 0);

    if (value < 0) {
        cerr << "Error in setting video device index" << endl;
        exit(-1);
    }

    pAVInputFormat = av_find_input_format("avfoundation");

    if (avformat_open_input(&pAVFormatContext, "Capture screen 0:none", pAVInputFormat, &options) != 0) {  //TODO trovare un modo per selezionare sempre lo schermo (forse "Capture screen 0")
        cerr << "Error in opening input device" << endl;
        exit(-1);
    }

#endif

    /*
     * Con av_dict_set passo determinati parametri a options che mi servirà, dopo, per settare alcuni parametri di
     * pAVFormatContext con avformat_open_input.
     * av_dict_set ritorna un valore >=0 in caso di successo,
     * minore di zero in caso di fallimento.
     */
     /*

    //a --> attivata questa sezione
    /******************************************/ /*
     value = av_dict_set(&options, "framerate", "15", 0); // inizialmente era fissato a 30 su linux
     if (value < 0) // Controllo che non ci siano stati errori con av_dict_set
     {
         cout << "\nError in setting dictionary value";
         exit(1);
     }

     value = av_dict_set(&options, "preset", "medium", 0);
     if (value < 0)
     {
         cout << "\nError in setting preset values";
         exit(1);
     }

    */ 
    /******************************************/

     //La seuente riga è da utilizzare con dshow
     //value = avformat_open_input(&pAVFormatContext, "video=screen-capture-recorder", pAVInputFormat, &options);

     // cout << "\Framerate: " << pAVFormatContext;

    value = avformat_find_stream_info(pAVFormatContext, &options); // Read packets of a media file to get stream information.
    if (value < 0)
    {
        cout << "\nUnable to find the stream information";
        exit(-1);
    }

    VideoStreamIndx = -1;

    /* find the first video stream index . Also there is an API available to do the below operations */
    for (int i = 0; i < pAVFormatContext->nb_streams; i++) // find video stream posistion/index
    {
        if (pAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            VideoStreamIndx = i;
            break;
        }
    }

    if (VideoStreamIndx == -1)
    {
        cout << "\nError: Unable to find the video stream index. (-1)";
        exit(1);
    }

    pCodecParameters = pAVFormatContext->streams[VideoStreamIndx]->codecpar;

    pAVCodec = const_cast<AVCodec*>(avcodec_find_decoder(pAVFormatContext->streams[VideoStreamIndx]->codecpar->codec_id));
    if (pAVCodec == nullptr)
    {
        cout << "\nUnable to find the decoder";
        exit(1);
    }

    /* #TODO: inutile perchè avcodec_find_decoder fatto all'interno del ciclo a riga 149 circa?
    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);// avcodec_find_decoder ha un ritorno di tipo const
    pAVCodec = const_cast<AVCodec *>(avcodec_find_decoder(pAVCodecContext->codec_id));
    if( pAVCodec == NULL )
    {
        cout<<"\nUnable to find the decoder";
        exit(1);
    }
    */

    // Link per capire meglio questa parte: https://awesomeopensource.com/project/leandromoreira/ffmpeg-libav-tutorial

    // assign pAVFormatContext to VideoStreamIndx
    // pAVCodecContext = pAVFormatContext->streams[VideoStreamIndx]->codec; //Errore perché in streams[VideoStreamIndx] non c'è nessun codec

    // Alloca un AVCodecContext e imposta i suoi campi sui valori predefiniti.
    // Ritorna un AVCodecContext riempito con valori predefiniti o NULL in caso di errore.
    pAVCodecContext = avcodec_alloc_context3(pAVCodec);

    // Riempie il CodecContext in base ai valori dei parametri forniti.
    // Ritorna un valore >=0 in caso di successo.
    value = avcodec_parameters_to_context(pAVCodecContext, pCodecParameters);
    if (value < 0)
    {
        cout << "\nUnable to set the parameters of the codec";
        exit(1);
    }

    // Initialize the AVCodecContext to use the given AVCodec.
    //  Prima di utilizzare questa funzione, il contesto deve essere allocato con avcodec_alloc_context3().
    //Aggiornata la avcodec_open2
    /*
    value = avcodec_open2(pAVCodecContext, pAVCodec, NULL);

    if (value < 0)
    {
        cout << "\nUnable to open the av codec";
        exit(1);
    }*/

    // cout << "\npAVCodecContext->width: "<< pAVCodecContext->width;

    return options;
}

/* Initialize the video output file and its properties  */
void ScreenRecorder::generateVideoStream() //Nome aggiornato
{
    // Trova un codificatore (encoder) che matcha con l'ID_codec indicato.
    // Ritorna l'encoder in caso di successo, NULL in caso di errore
    pLocalCodec = const_cast<AVCodec*>(avcodec_find_encoder(AV_CODEC_ID_MPEG4));
    if (pLocalCodec == nullptr)
    {
        cout << "\nUnable to find the encoder";
        exit(-8);
    }

    // Alloca un AVCodecContext e imposta i suoi campi sui valori predefiniti.
    // Ritorna un AVCodecContext riempito con valori predefiniti o NULL in caso di errore.
    outAVCodecContext = avcodec_alloc_context3(pLocalCodec);
    if (!outAVCodecContext)
    {
        cout << "\nError in allocating the codec contexts";
        exit(-7);
    }

    // Aggiunge un nuovo stream al file media
    // Ritorna lo stream appena creato
    video_st = avformat_new_stream(outAVFormatContext, pLocalCodec);
    if (!video_st) // Effettuo un check
    {
        cout << "\nError in creating a av format new stream";
        exit(-6);
    }

    value = avcodec_parameters_to_context(outAVCodecContext, video_st->codecpar);
    if (value < 0) // Eseguo check
    {
        cout << "\nUnable to set the parameter of the codec";
        exit(1);
    }

    outAVCodecContext->codec_id = AV_CODEC_ID_MPEG4; // AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outAVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outAVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    outAVCodecContext->width = cropW;
    outAVCodecContext->height = cropH; 

    //a --> 10000000
#ifdef __linux__
    outAVCodecContext->bit_rate = 90000; //FIXME: era 10 000 000, G: 96000
#elif _WIN32
    outAVCodecContext->bit_rate = 10000000;
#endif
    outAVCodecContext->gop_size = 10; //aggiornato -> era 3
    outAVCodecContext->global_quality = 500; //Nuovo
    outAVCodecContext->max_b_frames = 2;
    outAVCodecContext->time_base.num = 1;

    //a --> 33
    #ifdef __linux__
        outAVCodecContext->time_base.den = 12.5;// 15fps 
        // outAVCodecContext->time_base.den = 25;// 15fps

    #elif _WIN32
        outAVCodecContext->time_base.den = 25;// 15fps
        // outAVCodecContext->time_base.den = 25*((1920*1080)/(cropH*cropW));// 15fps //#TODO: ragionarci

    #endif
    outAVCodecContext->bit_rate_tolerance = 400000; //a --> 400000


    if (outAVCodecContext->codec_id == AV_CODEC_ID_H264)
    {
        // This function set the field of obj with the given name to value.
        av_opt_set(outAVCodecContext->priv_data, "preset", "slow", 0);
    }

    /* Some container formats (like MP4) require global headers to be present
       Mark the encoder so that it behaves accordingly. */
    if (outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) // Effettuo un check sui flag
    {
        outAVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // Initialize the AVCodecContext to use the given AVCodec.
    // Prima di utilizzare questa funzione, il contesto deve essere allocato con avcodec_alloc_context3().
    value = avcodec_open2(outAVCodecContext, pLocalCodec, nullptr);//Aggiornata
    if (value < 0) // Effettuo un check
    {
        cout << "\nError in opening the avcodec";
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
        cerr << "Error: cannot find a free stream index for video output" << endl;
        exit(-1);
    }
    avcodec_parameters_from_context(outAVFormatContext->streams[outVideoStreamIndex]->codecpar, outAVCodecContext);
}


/*
Funzione per acquisire e memorizzare i dati in frame allocando la memoria richiesta
e rilasciando automaticamente la memoria
*/
int ScreenRecorder::captureVideoFrames(mutex* mu, condition_variable* cv, mutex* write_lock,
                                        condition_variable* cvw, AudioRecorder* audio) //Da sistemare
{
    int64_t pts = 0; //utile? -> creata nel .h
    int flag;
    int frameFinished = 0;
    // bool endPause = false; //#TODO: dovrebbe essere inutile, perché è il contrario di pauseSC
    int numPause = 0;

    #ifdef __linux__
    #if RUN == 1 
        //ofstream outFile{ "media/" + timestamp + "_log.txt", ios::app }; // RUN  //FIXME: capire se va bene altrimenti mettere out inceve di app
    #else  
        //ofstream outFile{ "../media/" + timestamp + "_log.txt", ios::app }; // DEBUG
    #endif 
    #elif _WIN32
        //ofstream outFile{ "..\\media\\" + timestamp + "_log.txt", ios::app };
    #endif

    int frameIndex = 0;
    // int flag;
    // int frameFinished;

    /*Quando decodifichi un singolo pacchetto, non hai ancora informazioni sufficienti per avere un frame
     * (a seconda del tipo di codec).
     * Quando decodifichi un GRUPPO di pacchetti che rappresenta un frame,
     * solo allora hai un'immagine! Ecco perché frameFinished ti farà sapere che hai decodificato abbastanza
     * per avere un frame.
     * */

     // int frame_index = 0;
    value = 0;

    /* av_packet_alloc alloca un AVPacket e imposta i suoi campi sui valori predefiniti. */
    pAVPacket = av_packet_alloc();
    if (!pAVPacket)
        exit(1); //( "Error in allocating AVPacket")
    // cout<<""<<pAVPacket->

    // Le successive 2 righe di codice son deprecate commentate perchè av_init_packet è stato deprecato

    /*av_malloc alloca un blocco di dimenzione pari a "sizeof(AVPacket)" Byte e ritorna un puntatore al blocco
     * allocato oppure NULL se il blocco non può essere allocato.
     * NB: pAVPacket è di tipo AVPacket *. Non a caso si esegue un cast a "AVPacket *" del ritorno di av-malloc
     * Di fatto, pAVPacket è un puntatore ad un pacchetto.*/

     // pAVPacket = (AVPacket *)av_malloc(sizeof(AVPacket));

     /*Inizializza i campi facoltativi di un pacchetto con valori predefiniti.
      *NB: questa funzione non tocca i membri "data" e "size", che devono essere inizializzati separatamente.*/
      // av_init_packet(pAVPacket);



      /*av_frame_alloc() alloca un AVFrame e imposta i suoi campi sui valori predefiniti.
       *La struttura risultante deve essere liberata utilizzando av_frame_free().
       * Ritorna un AVFrame riempito con valori predefiniti o NULL in caso di errore.
       * NB: Alloca solo l'AVFrame e non il buffer di dati. Questo deve essere allocato con altri mezzi,
       * ad es. con av_frame_get_buffer() o manualmente.*/
    pAVFrame = av_frame_alloc();

    if (!pAVFrame) // Verifichiamo che l'operazione svolta da "av_frame_alloc()" abbia avuto successo
    {
        cout << "\nError: Unable to release the avframe resources";
        exit(1);
    }


    outFrame = av_frame_alloc(); //#TODO: dovrebbe essere av_frame_free() ??? --> a cosa serve questa riga
    if (!outFrame)
    {
        cout << "\nError: Unable to release the avframe resources for outframe";
        exit(1);
    }


    int video_outbuf_size; //utile? #FIXME: non viene utilizzata, deve essere inutile

    /*"av_image_get_buffer_size" restituisce il numero di byte necessari per memorizzare un'immagine.
     * NB: Le specifiche dell'immagine sono indicate tra parentesi*/
    int nbytes = av_image_get_buffer_size(outAVCodecContext->pix_fmt, outAVCodecContext->width, outAVCodecContext->height, 32);

    uint8_t* video_outbuf = (uint8_t*)av_malloc(nbytes);
    if (video_outbuf == nullptr)
    {
        cout << "\nUnable to allocate memory";
        exit(1);
    }


    /* Setup the data pointers and linesizes based on the specified image parameters and the provided array. */
    /* Si effettua un setup dei data pointers e delle linesizes (??) in base alle specifiche dell'immagine e
     * ed all'array fornito.
     * La funzione ritorna la dimensione in byte richiesta per video_outbuf oppure un valore minore di zero in caso di errore.
     *
     * SIGNIFICATO PARAMETRI:
     * "outFrame->data" indica i data pointers da compilare
     * "outFrame->linesize" indica le linesize per l'immagine in outFrame->data da compilare
     * "video_outbuf" indica il buffer che contiene o conterrà i dati dell'immagine effettivi. Può valere NULL
     * "AV_PIX_FMT_YUV420P" indica il formato pixel dell'immagine
     * "outAVCodecContext->width" e "outAVCodecContext->height" indicano rispettivamente la larghezza e l'altezza dell'immagine in pixel
     * L'ultimo parametro indica il valore usato per allineare le linesizes
     */
    // value = av_image_fill_arrays(outFrame->data, outFrame->linesize, video_outbuf, AV_PIX_FMT_YUV420P, outAVCodecContext->width, outAVCodecContext->height, 1);
    value = av_image_fill_arrays(outFrame->data, outFrame->linesize, video_outbuf, outAVCodecContext->pix_fmt, outAVCodecContext->width, outAVCodecContext->height, 1);

    if (value < 0) // Verifico che non ci siano errori
    {
        cout << "\nError in filling image array";
        exit(1);
    }


    // Allocate and return swsContext.
    // a pointer to an allocated context, or NULL in case of error
    SwsContext* swsCtx_;
    if (!(swsCtx_ = sws_alloc_context()))
    {
        cout << "\nError nell'allocazione del SwsContext";
        exit(1);
    }


    /*La funzione sws_getContext alloca un SwsContext e ne ritorna il puntatore (o NULL in caso di errore).
     * I primi 3 parametri sono riferiti all'immagine sorgente. // tra AVPAcket e AVCodec -> pAVCodecContext
     * I secondi 3 parametri sono riferiti all'immagine di destinazione. // tra AVCodec e AVFrame -> outAVCodecContext
     * Il settimo parametri specifica quale algoritmo utilizzare per ri-scalare
     * I restanti parametri sono altri flag di cui non ci servono i particolari
     */
     // sws_getContext Deprecated : Use sws_getCachedContext() instead.
     /*
     swsCtx_ = sws_getContext(pAVCodecContext->width,
         pAVCodecContext->height,
         pAVCodecContext->pix_fmt,
         outAVCodecContext->width,
         outAVCodecContext->height,
         outAVCodecContext->pix_fmt,
         SWS_BICUBIC, NULL, NULL, NULL );
     */

    value = sws_init_context(swsCtx_, NULL, NULL);
    if (value < 0)
    {
        cout << "\nErrore nell'inizializzazione del SwsContext";
        exit(1);
    }

    swsCtx_ = sws_getCachedContext(swsCtx_,
        pAVCodecContext->width,
        pAVCodecContext->height,
        pAVCodecContext->pix_fmt,
        outAVCodecContext->width,
        outAVCodecContext->height,
        outAVCodecContext->pix_fmt,
        SWS_BILINEAR, NULL, NULL, NULL);

// cout << "\nswsCtx_: " << swsCtx_ <<"\n";
    if (avcodec_open2(pAVCodecContext, pAVCodec, &options) < 0) { //NUOVA
        cerr << "Could not open codec" << endl;
        exit(-1);
    }

    /*
    //#TODO: non più frame => dovrebbe essere inutile questa sezione, poiché ora scegliamo il tempo di registrazione
    int ii = 0;
    int no_frames = 100;
    cout << "\nEnter No. of frames to capture : ";
    cin >> no_frames;
    */

    AVPacket* outPacket;
    //int j = 0;
    //outPacket = av_packet_alloc();
    int got_picture;

    time_t startTime; 
    time(&startTime);


    /*av_read_frame è una funzione che ad ogni chiamata trasmette un frame preso da uno stream.
         * In caso di successo il paccheto sarà reference-counted (pAVPacket->buf viene settato)
         * e sarà disponibile a tempo indeterminato.
         * Il pacchetto deve essere lberato con av_packet_unref() quando non è più utile.
         * Per il video, il pacchetto contiene esattamente un fotogramma.
         * Per l'audio, invece, dipende:
         * - se ogni frame ha una dimensione nota (ad es. dati PCM o ADPCM), allora il pacchetto conterrà un numero intero di frame;
         * - se ogni frame ha una dimensione variabile (ad es. audio MPEG), allora il pacchetto conterrà un solo frame.
         *  pAVPacket->pts può valere AV_NOPTS_VALUE se il formato video contiene B-frame, quindi è meglio fare affidamento a
         *  pAVPacket->dts (pAVPacket->dts e pAVPacket->pts sono due timestamp).
         *  av_read_frame ritorna 0 se tutto è ok, un valore negativo in caso di errore o un EOF.
         *  In caso di errore, pAVPacket sarà vuoto (come se provenisse da av_packet_alloc()).
         *  NB:pAVPacket verrà inizializzato, quindi potrebbe essere necessario terminarlo anche se non contiene dati.
    */
    while (pAVCodecContext->frame_number < magicNumber) //a --> variabile stop solo per video e funzione !ShouldStopVideo()  
    {
        /*
        cout << "\npAVPacket->buf: " << pAVPacket->buf;
        cout << "\npAVFrame->buf: " << pAVFrame->buf;
        cout << "\noutFrame->buf: " << outFrame->buf;
        */

        /* #FIXME: Testing pause-stop etc.*/
        // if (pAVCodecContext->frame_number == 50){
        //     toggleScreenCapture();
        // }
    
        /*********************************/

        if (pauseSC) {
            cout << "Pause" << endl;
            //outFile << "///////////////////   Pause  ///////////////////" << endl;
            cout << "outAVCodecContext->time_base: " << outAVCodecContext->time_base.num << ", " << outAVCodecContext->time_base.den << endl;
        }

       /* #FIXME: Testing pause-stop etc.*/
    //     if (pAVCodecContext->frame_number == 50){
    //        for (int iter=0; iter < 100000; iter++){
    //            cout << "\n" << iter << endl;
    //        }
    //        toggleScreenCapture();
    //    }

        std::unique_lock<std::mutex> ul(*mu);

        cv->wait(ul, [this]() { return !pauseSC; });   //pause capture (not busy waiting)

        // if (endPause) {      //#TODO: dovrebbe essere inutile, perché è il contrario di pauseSC
        //     endPause = false;
        // }

        if (stopSC)  //check if the capture has to stop
            break;

        ul.unlock();

        //A qui

        /*
        //#TODO: non più frame
        if (ii++ == no_frames)
        {
            // value = AVERROR_EOF;
            break;
        }
        */

        /* Prendiamo il prossimo frame di uno stream
        da pAVFormatContext a pAVPacket*/
        if (av_read_frame(pAVFormatContext, pAVPacket) >= 0 && pAVPacket->stream_index == VideoStreamIndx)
        {
            // char buf[1024];

            // FUNZIONE DEPRECATA
            // value = avcodec_decode_video2( pAVCodecContext , pAVFrame , &frameFinished , pAVPacket );


            av_packet_rescale_ts(pAVPacket, pAVFormatContext->streams[VideoStreamIndx]->time_base, pAVCodecContext->time_base);//Nuova

             /* avcodec_send_packet fornisce dati compressi grezzi in un AVPacket come input al decodificatore.
             * Internamente questa chiamata copierà i campi rilevanti di pAVCodecContext che possono influenzare
             * la decodifica per-packet e li applicherà quando il pacchetto verrà effettivamente decodificato.
             *
             * Da pavPacket a pAVCodecContext.
             */
            value = avcodec_send_packet(pAVCodecContext, pAVPacket);

            if (value < 0) // Verifichiamo che non ci siano stati errori
            {
                cout << "\nProblem with avcodec_send_packet";
                // exit(1);
            }
            /*
            cout << "\npAVCodecContext->width: " << pAVCodecContext->width;
            cout << "\npAVCodecContext->height: " << pAVCodecContext->height;
            cout << "\npAVCodecContext->coded_width: " << pAVCodecContext->coded_width;
            cout << "\npAVCodecContext->coded_height: " << pAVCodecContext->coded_height;
            cout << "\npAVCodecContext->pix_fmt: " << pAVCodecContext->pix_fmt;
            cout << "\npAVCodecContext->codec->id: " << pAVCodecContext->codec->id;
            */

            /* avcodec_receive_frame restituisce i dati di output decodificati da un decodificatore.
             *
             * Da pAVCodecContext a pAVFrame */
            value = avcodec_receive_frame(pAVCodecContext, pAVFrame);

            //#TODO: non più frame da scegliere
            //cout << "\nFrame: " << pAVCodecContext->frame_number << "\n";

            // cout << "\npAVFrame->data[0]: " << pAVFrame->data[0];
            // cout << "\nvalue AVERROR_EOF: " << AVERROR_EOF;
            if (value == AVERROR(EAGAIN) || value == AVERROR_EOF)
            {
                cout << "\nOutput not available in this state.  Try to send new input. ";
                // break;
                // exit(1);
            }
            else if (value < 0)
            {
                cout << "\nError during decoding";
                exit(1);
            }
            // snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
            // pgm_save(frame->data[0], frame->linesize[0], frame->width, frame->height, buf);

            /* Funzione utile per riscalare lo slice dell'immagine (presa da pAVFrame->data) e salvare
             * il risultato in outFrame->data.
             * Ritorna l'altezza dell'immagine di output
             *
             * Da pavFrame a outFrame
             */
            value = sws_scale(swsCtx_, pAVFrame->data, pAVFrame->linesize, 0, pAVCodecContext->height, outFrame->data, outFrame->linesize);

            if (value < 0)
            {
                cout << "\nProblem with sws_scale ";
                // break;
                exit(1);
            }
            //Frame decodificato con successo


            // av_init_packet(&outPacket); funzione deprecata
            outPacket = av_packet_alloc();
            outPacket->data = nullptr; // i dati del pacchetto verranno allocati dall'encoder
            outPacket->size = 0;
            // FUNZIONE DEPRECATA
            // avcodec_encode_video2(outAVCodecContext , &outPacket ,outFrame , &got_picture);
            outFrame->width = outAVCodecContext->width;
            outFrame->height = outAVCodecContext->height;
            outFrame->format = outAVCodecContext->pix_fmt;
            // outAVCodecContext->pix_fmt;


            /*Forniamo un frame all'encoder.
            Utilizzeremo, poi, avcodec_receive_packet() per recuperare i pacchetti di output memorizzati nel buffer.

            Da outFrame a encoder.
            Da encoder a outAVCodecContext*/
            value = avcodec_send_frame(outAVCodecContext, outFrame);
            if (value < 0)
            {
                cout << "\nError sending a frame for encoding. ERROR CODE: " << value;
                continue;
                // exit(1);
            }
            /*Leggiamo dei dati codificati dall'encoder

             Da outAVCodecContext a outPacket*/
            value = avcodec_receive_packet(outAVCodecContext, outPacket); // Legge i dati codificati dall'encoder.
            if (value == AVERROR(EAGAIN))
            {
                cout << "\nOutput not available in this state.  Try to send new input";
                continue;
                // exit(1);
            }
            else if (value < 0 && value != AVERROR_EOF)
            {
                // cout << "\nAVERROR_EOF: " << AVERROR_EOF;
                // cout << "\nAVERROR(EAGAIN): " << AVERROR(EAGAIN);
                cout << "\nError during encoding";
                // continue;
                exit(1);
            }

            if (value >= 0) // Frame successfully encoded :)
            {
                
                if (outPacket->pts != AV_NOPTS_VALUE)
                    outPacket->pts = av_rescale_q(outPacket->pts, outAVCodecContext->time_base, video_st->time_base);//FIXME: ultimo parametro protrebbe essere AV_TIME_BASE_Q Provare a capire da qui -> https://stackoverflow.com/questions/25837947/synchronizing-ffmpeg-video-frames-using-pts
                // Rescales a 64-bit integer by 2 rational numbers.
                // Nel codice di Abdullah veniva usata la stessa funzione ma con un parametro diverso (che dava errore):
                //  outPacket.pts = av_rescale_q(outPacket.pts, video_st->codec->time_base, video_st->time_base);
                // video_st->codec->time_base è stato deprecato.
                if (outPacket->dts != AV_NOPTS_VALUE)
                    outPacket->dts = av_rescale_q(outPacket->dts, outAVCodecContext->time_base, video_st->time_base);
                // Rescales a 64-bit integer by 2 rational numbers.
                // Nel codice di Abdullah veniva usata la stessa funzione ma con un parametro diverso (che dava errore):
                // outPacket.dts = av_rescale_q(outPacket.dts, video_st->codec->time_base, video_st->time_base);
                // video_st->codec->time_base è stato deprecato.

                //printf("\nWrite frame %3d (size= %2d)\n", j++, outPacket->size / 1000);

                /*
                    * "av_write_frame" serve per scrivere un pacchetto (outpacket) in un file multimediale di output.
                     * Ritorna 0 se tutto è ok, un valore <0 se ci sono errori, 1 se è stato flushato
                */
                

                /* //utile?
                time_t timer;
                double seconds;

                mu.lock();

                if (!activeMenu) {
                    time(&timer);
                    seconds = difftime(timer, startTime);
                    int h = (int)(seconds / 3600);
                    int m = (int)(seconds / 60) % 60;
                    int s = (int)(seconds) % 60;

                    std::cout << std::flush << "\r" << std::setw(2) << std::setfill('0') << h << ':'
                        << std::setw(2) << std::setfill('0') << m << ':'
                        << std::setw(2) << std::setfill('0') << s << std::flush;
                }

                mu.unlock();
                */ //utile?  end


                /* Acquisisco write lock per scrivere il frame video sul file */
                unique_lock<mutex> ulw(*write_lock);

                //Effettuo conversione dei pts 
                ptsV = outPacket->pts / video_st->time_base.den;
                ptsA = audio->getPtsA();

                #ifdef _WIN32
                    /* Per sincronizzare video e audio */
                    cvw->notify_one();
                    cvw->wait(ulw, [this](){return ((ptsA - 2 >= ptsV) || end); });
                #endif
                
                //outFile << "Scrivo VIDEO-PTS_TIME: " << ptsV << "\n" << endl;
                //cout << outPacket << endl;
                cout << "\n Scrivo VIDEO-PTS_TIME: " << ptsV << endl;
                /*Scriviamo il pacchetto in un output media file.
                NB: Per l'audio viene usato av_interleaved_write_frame()...come mai?
                NB: A differenza di av_interleaved_write_frame(), av_write_frame non effettua nessuna copia
                dei pacchetti nel buffer => più efficiente, meno efficace

                Da outPacket a outAVFormatContext*/
                if (av_interleaved_write_frame(outAVFormatContext, outPacket) != 0)
                {
                    cout << "\nError in writing video frame" << endl;
                }

                ulw.unlock();


                /* Una volta scritto il frame, libero il pacchetto*/
                av_packet_free(&outPacket);

               /*
                * Pulisce il pacchetto.
                * Elimina il riferimento al buffer a cui fa riferimento il pacchetto e resetta
                * i rimanenti campi del pacchetto ai loro valori predefiniti.
                */
                // av_packet_unref(&outPacket); //utile?


                cout.flush(); //utile?

            } // got_picture

            /* Deallocazione e riallocazione della memoria, per evitare memory leakage */
            // av_packet_unref(&outPacket); //utile?
            av_packet_free(&outPacket);
            av_packet_free(&pAVPacket);
            pAVPacket = av_packet_alloc();
            if (!pAVPacket)
                exit(1);

            av_frame_free(&pAVFrame);
            pAVFrame = av_frame_alloc();
            if (!pAVFrame) // Verifichiamo che l'operazione svolta da "av_frame_alloc()" abbia avuto successo
            {
                cout << "\nUnable to release the avframe resources";
                exit(1);
            }

            av_frame_free(&outFrame);
            outFrame = av_frame_alloc();
            if (!outFrame)
            {
                cout << "\nUnable to release the avframe resources for outframe";
                exit(1);
            }

            value = av_image_fill_arrays(outFrame->data, outFrame->linesize, video_outbuf, AV_PIX_FMT_YUV420P, outAVCodecContext->width, outAVCodecContext->height, 1);
            if (value < 0) // Verifico che non ci siano errori
            {
                cout << "\nError in filling image array";
                exit(1);
            }
        }

    } // End of while-loop

    // stopSC = true; //#TODO: ha senso settarla qui? Non dovrebbe settarsi a true una volta finiti entrambi i thread Video e Audio?


    /* Conclusione della registrazione, 
        ora si liberano le strutture di memoria e 
        si chiude il file
    */

    av_packet_free(&outPacket);
    /*
     * Scrive il trailer dello stream in un file multimediale di output e
     * libera i dati privati ​​del file. Se non ci sono stati errori, ritorna 0.
     */
    value = av_write_trailer(outAVFormatContext);
    if (value < 0)
    {
        cout << "\nError in writing av trailer";
        exit(1);
    }

    #ifdef _WIN32
        /* Notifica la fine della registrazione al thread audio per evitare deadlock */
        end = true; //FIXME: utilizzare variabile stopSC? - utile e funzionante per linux
        cvw->notify_one();
    #endif

    //outFile.close();


    av_packet_free(&pAVPacket);
    sws_freeContext(swsCtx_);
    av_frame_free(&pAVFrame);
    av_frame_free(&outFrame);
    /*
     * Libera un blocco di memoria che è stato allocato con av_malloc (z) () o av_realloc ().
     * Riceve come parametro il puntatore al blocco di memoria che deve essere liberato.
     */
    av_free(video_outbuf);

    return 0;
}

int ScreenRecorder::getMagic() {
    return pAVCodecContext->frame_number;
}

int ScreenRecorder::getPtsV() {
    return ptsV;
}

#if _WIN32
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
            &hKey, &value) != ERROR_SUCCESS) 
            cout<<"Errore nel settare la chiave di registro"<<endl;

    // SetError("Errore nel settare la chiave di registro");
    RegSetValueEx(hKey, keyToSet, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
    RegCloseKey(hKey);
}
#endif