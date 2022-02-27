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

/* Recupero timestamp */
std::string retrieveTimestamp()
{

    std::string current_time;

    #ifdef __linux__
        //#TODO: dovrebbe tranquillamente funzionare anche per Windows
        // declaring argument of time()
        const time_t now = time(nullptr);

        /* ctime() used to give the present time */
        // current_time = ctime(&my_time); //ctime obsoleta per lo standard POSIX

        /* Main source of this solution: https://en.cppreference.com/w/cpp/chrono/c/asctime */
        char buf[64];
        if (strftime(buf, sizeof buf, "%c\n", std::localtime(&now)) || strftime(buf, sizeof buf, "%a %b %e %H:%M:%S %Y\n", std::localtime(&now)))
        {
            // std::cout << std::setw(40) << "    strftime %c" << buf;
            // std::cout << std::setw(40) << "    strftime %a %b %e %H:%M:%S %Y" << buf;
            current_time = buf;
            current_time.erase(current_time.end() - 1, current_time.end());
            std::replace(current_time.begin(), current_time.end(), ' ', '_');
        }
        // else {
        //     std::cerr << "Error in retrieving timestamp" << std::endl;
        //     exit(-1);
        // }
    #elif _WIN32
        time_t result = time(nullptr);
        stringstream ss;
        ss << time;
        current_time = ss.str();
    #endif

    return current_time;
}

/****************************/

/* Definiamo il COSTRUTTORE */
/* Initialize the resources*/
ScreenRecorder::ScreenRecorder() : isAudioActive(false), started(false), /*activeMenu(true),*/
                                    /*magicNumber(300),*/ cropX(0), cropY(0), cropH(1080), cropW(1920), frameCount(0), end(false),
                                    pauseRec(false), stopRecAudio(false), stopRecVideo(false), closedVideo(false), closedAudio(false)

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
        // tie(cropH, cropW) = retrieveDisplayDimention(); //FIXME: commentare perché sovrascrive crop passati
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

    /* Set timestamp */
    timestamp = retrieveTimestamp();

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

        // ---------------Video-----------------
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
        CloseRecorder();
    // } //end - started

}

/* Inizializzazione file di output e suo risorse */
int ScreenRecorder::initOutputFile() {
    value = 0;

    outAVFormatContext = nullptr;
    /* #TODO: vedi funzione retrieceTimestamp */
    // time_t result = time(nullptr);
    // stringstream ss;
    // ss << time;
    // timestamp = ss.str();

    string outputName = timestamp + "_output.mp4";
    // Setting formato del file
    outputAVFormat = const_cast<AVOutputFormat*>(av_guess_format(nullptr, outputName.c_str(), nullptr));

    if (outputAVFormat == nullptr) {
        cerr << "Error in guessing the video format, try with correct format" << endl;
        exit(-5);
    }

    #ifdef __linux__
        /*
            N.B.:   IN DEBUG la cartella di partenza è quella in cui di trova questo file stesso
                    IN RUN la cartella di partenza è quella del progetto in sé
        */
        // string outputPath = "../media/" + outputName; // DEBUG
        // string outputPath = "media/output.mp4"; // RUN   
        // string outputPath = "../media/" + outputName; // DEBUG  
    #if RUN == 1
        string outputPath = "media/" + outputName; // RUN 
    #else
        string outputPath = "../media/" + outputName; // DEBUG 
    #endif   
    #elif _WIN32
        string outputPath = "..\\media\\" + outputName;
    #endif

    avformat_alloc_output_context2(&outAVFormatContext, outputAVFormat, outputAVFormat->name, outputPath.c_str());
    if (outAVFormatContext == nullptr) {
        cerr << "Error in allocating outAVFormatContext" << endl;
        exit(-4);
    }


    /*===========================================================================*/
    this->generateVideoStream();
    if (isAudioActive)
        this->generateAudioStream();

    //Create an empty video file
    if (!(outAVFormatContext->flags & AVFMT_NOFILE)) {
        //int ret_avio = avio_open2(&outAVFormatContext->pb, outputPath.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
        int ret_avio = avio_open(&outAVFormatContext->pb, outputPath.c_str(), AVIO_FLAG_WRITE);
        if (ret_avio < 0) {
            cerr << "Error in creating the video file" << endl;
            exit(-10);
        }
    }

    /* Controlla che il file di output contenga almeno uno stream */
    if (outAVFormatContext->nb_streams == 0) {
        cerr << "Output file does not contain any stream" << endl;
        exit(-11);
    }

    /* Alloca i dati dello stream e scrive l'header dello stream al file di output. */
    value = avformat_write_header(outAVFormatContext, &options);
    if (value < 0) {
        cerr << "Error in writing the header context" << endl;
        exit(-12);
    }
    started = true;
    return 0;
}


/***************************** VIDEO *****************************/

/* Establishing the connection between camera or screen through its respective folder */
//int ScreenRecorder::openCamera() throw() //#FIXME
int ScreenRecorder::openVideoDevice()
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

    return 0;
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
        int he, wi; 
        tie(he, wi) = retrieveDisplayDimention();
        if (cropW == he || cropH == wi){
            outAVCodecContext->time_base.den = 12.5;
        }else{
            outAVCodecContext->time_base.den = 33;
        }

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
int ScreenRecorder::captureVideoFrames() //Da sistemare
{
    int64_t pts = 0; //utile? -> creata nel .h
    int flag;
    int frameFinished = 0;
    // bool endPause = false; //#TODO: dovrebbe essere inutile, perché è il contrario di pauseSC
    int numPause = 0;

    #ifdef __linux__
    #if RUN == 1 
        ofstream outFile{ "media/" + timestamp + "_log.txt", ios::app }; // RUN  //FIXME: capire se va bene altrimenti mettere out inceve di app
    #else  
        ofstream outFile{ "../media/" + timestamp + "_log.txt", ios::app }; // DEBUG
    #endif 
    #elif _WIN32
        ofstream outFile{ "..\\media\\" + timestamp + "_log.txt", ios::app };
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
    
    while (pAVCodecContext->frame_number < magicNumber) //Sezione di DEBUG
    //while (!getVideoBool())  
    {
        
        if (pauseRec) {
            outFile << "///////////////////   Pause  ///////////////////" << endl;
            avformat_close_input(&pAVFormatContext);
            closedVideo = true;
        }

        std::unique_lock<std::mutex> ul(mu);

        cv.wait(ul, [this]() { return !pauseRec; });   //pause capture (not busy waiting)

        if (getVideoBool())  //check if the capture has to stop
            break;
        if (closedVideo) {
#if _WIN32
            value = avformat_open_input(&pAVFormatContext, "video=screen-capture-recorder", pAVInputFormat, &options);
#elif linux
            string resolutionS = to_string(cropW) + "x" + to_string(cropH);
            av_dict_set(&options, "video_size", resolutionS.c_str(), 0);   //option to set the dimension of the screen section to record
            string url = ":0.0+" + to_string(cropX) + "," + to_string(cropY);
            value = avformat_open_input(&pAVFormatContext, url.c_str(), pAVInputFormat, &options);
#endif
            if (value != 0) {
                cerr << "Error in opening input device (video)" << endl;
                exit(-1);
            }
            closedVideo = false;
        }

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
                unique_lock<mutex> ulw(write_lock);

                //Effettuo conversione dei pts 
                ptsV = outPacket->pts / video_st->time_base.den;

                #ifdef _WIN32
                    /* Per sincronizzare video e audio */
                    cvw.notify_one();
                    cvw.wait(ulw, [this](){return ((ptsA - 2 >= ptsV) || end); });
                #endif
                
                outFile << "Scrivo VIDEO-PTS_TIME: " << ptsV << "\n" << endl;
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
        cvw.notify_one();
    #endif

    outFile.close();


    av_packet_free(&pAVPacket);
    sws_freeContext(swsCtx_);
    av_frame_free(&pAVFrame);
    av_frame_free(&outFrame);
    /*
     * Libera un blocco di memoria che è stato allocato con av_malloc (z) () o av_realloc ().
     * Riceve come parametro il puntatore al blocco di memoria che deve essere liberato.
     */
    av_free(video_outbuf);

    started = false;
    CloseRecorder();

    return 0;
} 


/***************************** FINE - VIDEO *****************************/


/***************************** AUDIO *****************************/

int ScreenRecorder::openAudioDevice() {
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
    }
    deviceName = "audio=" + deviceName;*/
    audioInputFormat = av_find_input_format("dshow");
    value = avformat_open_input(&inAudioFormatContext, "audio=Microphone Array (Realtek(R) Audio)", audioInputFormat, &audioOptions);
    //value = avformat_open_input(&inAudioFormatContext, deviceName.c_str(), audioInputFormat, &audioOptions);
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

void ScreenRecorder::generateAudioStream() {
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

int ScreenRecorder::init_fifo()
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(fifo = av_audio_fifo_alloc(outAudioCodecContext->sample_fmt,
        outAudioCodecContext->channels, outAudioCodecContext->sample_rate))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

int ScreenRecorder::add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size) {
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

int ScreenRecorder::initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size) {
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



void ScreenRecorder::captureAudio() {
    int ret;
    AVPacket* inPacket, * outPacket;
    AVFrame* rawFrame, * scaledFrame;
    uint8_t** resampledData;
    init_fifo();

    #ifdef __linux__
    #if RUN == 1 
        ofstream outFile{ "media/" + timestamp + "_log.txt", ios::app }; // RUN
    #else  
        ofstream outFile{ "../media/" + timestamp + "_log.txt", ios::app }; // DEBUG
    #endif 
    #elif _WIN32
        ofstream outFile{ "..\\media\\" + timestamp + "_log.txt", ios::app };
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

    while (pAVCodecContext->frame_number < magicNumber) { //Sezione di DEBUG
    // while (!getAudioBool()) { //a --> variabile stop solo per video e funzione !ShouldStopAudio()  

        if (pauseRec) {
            avformat_close_input(&inAudioFormatContext);
            closedAudio = true;
        }

        std::unique_lock<std::mutex> ul(mu);
        cv.wait(ul, [this]() { return !pauseRec; });

        if (getAudioBool()) {
            break;
        }
        if (closedAudio) {
#if _WIN32
            value = avformat_open_input(&inAudioFormatContext, "audio=Microphone Array (Realtek(R) Audio)", audioInputFormat, &audioOptions);
#elif linux
            
            deviceName = "default";
            value = avformat_open_input(&inAudioFormatContext, deviceName.c_str(), audioInputFormat, &audioOptions);
            
#endif
            if (value != 0) {
                cerr << "Error in opening input device (audio)" << endl;
                exit(-1);
            }
            closedAudio= false;
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

                        unique_lock<mutex> ulw(write_lock);   
                        //Effettuo conversione dei pts
                        ptsA = outPacket->pts / outAudioCodecContext->sample_rate;

                        #ifdef __linux__
                            // cvw.wait(ulw, [this]() {return ptsA <= ptsV; });
                        #elif _WIN32
                            cvw.notify_one();
                            cvw.wait(ulw, [this]() {return ((ptsA - 2 <= ptsV) || end); }); //FIXME: stopSC invece di end?
                        #endif

                        //cout << outPacket << endl;
                        outFile << "Scrivo AUDIO-PTS_TIME: " << ptsA << "\n" << endl;
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
        unique_lock<mutex> ulw(write_lock);
        end = true; //FIXME: stopSC invece di end?
        cvw.notify_one();
    #endif

    outFile.close();
}

/***************************** FINE - AUDIO *****************************/


/* Creazione thread per video e audio */
void ScreenRecorder::CreateThreads() {
    tV = std::thread(&ScreenRecorder::captureVideoFrames, this);
    if (isAudioActive) {
        tA = std::thread(&ScreenRecorder::captureAudio, this);
        tA.join(); //a ---> join() in StopRecording
    }
    tV.join();
}

/*
int ScreenRecorder::stopScreenCapture() {
    if (!stopSC) {
        stopSC = true;
        cout << "\nScreenRecorder stopped" << endl;
        return 0;
    }
    // else{
    cout << "\nScreenRecorder is not running" << endl;
    return -1;
    // }
}*/

/*
int ScreenRecorder::toggleScreenCapture() {
    std::lock_guard<std::mutex> lg(mu);
    if(stopSC){
        cout << "\nScreenRecorder is stopped" << endl;
        return -1;
    }
    if (!pauseSC) {
        pauseSC = true;
        cout << "\nScreenRecorder paused" << endl; //TODO: queste stampe potrebbero essere inutili
    }
    else{
        pauseSC = false;
        cout << "\nScreenRecorder resumed" << endl;
    }
    cv.notify_all();
    return 0;
}*/


/*******************************************/
/*******************************************/
/* Altre funzioni accessorie */

/* Funzione che racchiude il setup base */
void ScreenRecorder::SetUpScreenRecorder() { 
    this->openVideoDevice();
    this->openAudioDevice();
    this->initOutputFile();
    this->CreateThreads();
}

/*Funzione per la chiusura della registrazione*/
void ScreenRecorder::StopRecorder() {
    if (isAudioActive) {
        /*Se stava registrando l'audio, setta la variabile stopCaptureAudio a true*/
        AudioStop();
        /*Eseguo il join del thread*/
        tA.join();
    }
    /*Setta la variabile stopCaptureVideo a true*/
    VideoStop();
    /*Esegue il join del thread*/
    tV.join();
    /*Se era in pausa, ne esce e risveglia le condition variable per consentire la conclusione del thread*/
    if (pauseRec) {
        pauseRec = false;
    }
    cv.notify_all();
}

/*Setta la pausa*/
void ScreenRecorder::PauseRecorder()
{
    /*Inverte il valore della variabile pauseCapture e, se era attiva la pausa, sveglia i thread con la notify_all*/
    pauseRec = !pauseRec;
    if (!pauseRec) {
        cv.notify_all();
    }
}

/*Funzione per impostare lo stop dell'audio in accesso esclusivo*/
void ScreenRecorder::AudioStop()
{
    unique_lock<mutex> lk(stop_lockA);
    stopRecAudio = true;
}

/*Funzione per impostare lo stop del video in accesso esclusivo*/
void ScreenRecorder::VideoStop()
{
    unique_lock<mutex> lk(stop_lockV);
    stopRecVideo = true;
}

/*Funzione per leggere la variabile di stop dell'audio in accesso esclusivo*/

bool ScreenRecorder::getAudioBool()
{
    bool return_value;
    unique_lock<mutex> lk(stop_lockA);
    return_value = stopRecAudio;
    lk.unlock();
    return return_value;
}

/*Funzione per leggere la variabile di stop del video in accesso esclusivo*/

bool ScreenRecorder::getVideoBool()
{
    bool return_value;
    unique_lock<mutex> lk(stop_lockV);
    return_value = stopRecVideo;
    lk.unlock();
    return return_value;
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
        SetError("Error: unable to free AudioFormatContext");
        exit(-1);
    }


    avformat_close_input(&pAVFormatContext);
    if (pAVFormatContext == nullptr) {
        cout << "File close successfully" << endl;
    }
    else {
        SetError("Error: unable to close the file");
        exit(-1);
    }

    avformat_free_context(pAVFormatContext);
    if (pAVFormatContext == nullptr) {
        cout << "VideoFormat freed successfully" << endl;
    }
    else {
        SetError("Error: unable to free VideoFormatContext");
        exit(-1);
    }

    avio_closep(&outAVFormatContext->pb);
}

/*Funzione per impostare la stringa col messaggio di errore in accesso esclusivo*/

void ScreenRecorder::SetError(std::string error)
{
    unique_lock<mutex> lk (error_lock);
    error_msg = error;
}
/*Funzione per leggere la stringa col messaggio di errore in accesso esclusivo*/

std::string ScreenRecorder::GetErrorString()
{
    unique_lock<mutex> lk (error_lock);
    std::string returnValue = error_msg;
    return returnValue;
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

/****************** API ******************/
