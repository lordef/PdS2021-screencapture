#include "Recorder.h"
using namespace std;

Recorder::Recorder() {

}

Recorder::~Recorder() {
}

std::string Recorder::retrieveTimestamp()
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

/* Inizializzazione file di output e suo risorse */
int Recorder::initOutputFile() {
    val = 0;
    timestamp = retrieveTimestamp();
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

    options = screen_record.openVideoDevice();
    audio_record.openAudioDevice();

    screen_record.generateVideoStream();
    if (isAudioActive)
        audio_record.generateAudioStream();

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
    val = avformat_write_header(outAVFormatContext, &options);
    if (val < 0) {
        cerr << "Error in writing the header context" << endl;
        exit(-12);
    }
    return 0;
}


/* Creazione thread per video e audio */
void Recorder::CreateThreads() {
    thread t2([this] { this->screen_record.captureVideoFrames(&mu, &cv, &write_lock, &cvw, &audio_record); });
    if (isAudioActive) {
        thread t1([this] { this->audio_record.captureAudio(&mu, &cv, &write_lock, &cvw, &screen_record); });
        t1.join(); //a ---> join() in StopRecording
    }
    t2.join();
}


int Recorder::stopScreenCapture() {
    if (!stopSC) {
        stopSC = true;
        cout << "\nScreenRecorder stopped" << endl;
        return 0;
    }
    // else{
    cout << "\nScreenRecorder is not running" << endl;
    return -1;
    // }
}


int Recorder::toggleScreenCapture() {
    std::lock_guard<std::mutex> lg(mu);
    if (stopSC) {
        cout << "\nScreenRecorder is stopped" << endl;
        return -1;
    }
    if (!pauseSC) {
        pauseSC = true;
        cout << "\nScreenRecorder paused" << endl; //TODO: queste stampe potrebbero essere inutili
    }
    else {
        pauseSC = false;
        cout << "\nScreenRecorder resumed" << endl;
    }
    cv.notify_all();
    return 0;
}


/*******************************************/
/*******************************************/
/* Altre funzioni accessorie */

/* Funzione che racchiude il setup base */
//Utile?
void Recorder::SetUpScreenRecorder() {
    screen_record.openVideoDevice();
    audio_record.openAudioDevice();
    this->initOutputFile();
    this->CreateThreads();
}