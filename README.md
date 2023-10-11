# PdS2021-screencapture

## Nome del Team: GIL
Membri del Team:

* [Gabriolo](https://github.com/Gabriolo)
* [v0q1n8](https://github.com/v0q1n8)
* [lordef](https://github.com/lordef)
-------------------

# Application Programming Project: Screen capture (CAPTURE)
## Project’s summary
The project aims at building a **multiplatform library** capable of **capturing the entire screen**
(or a portion of it) **and** **storing** it in **mp4 format**, with or without audio. <br>
In order to show the
proper behavior of the library, a sample application based on it will be created, as well,
allowing the user to **record/pause/resume/stop a video** stream. <br>
Care should be given to
overall **user experience and usability**, properly handling possible **errors** and propagating
them in an effective and understandable way **to the user**, **and** providing **easy to learn/use
commands** to operate it.

## Required Background and Working Environment
Knowledge of the **C++17** general abstractions **and** of the **C++** Standard Template Library.<br>
Knowledge of **concurrency, synchronization and background processing**. <br>
The system will be developed using third party libraries (e.g., ffmpeg) in order to support
deployment on several platforms.
## Problem Definition
The system to be designed consists of a multi-platform library that supports screen
capturing and recording, and a sample application that demonstrates its usage. <br>
The library will be properly documented, providing a clear definition of its intended usage,
as well as of any error condition that can be reported.
<br>
<br>
By using the **sample application**, <br>
the user will be able to <br>
* define the area to be recorded
* select whether the audio should be captured or not
* activate and stop the recording process
* temporarily pause and subsequently resume it
* define the file that will contain the final recording

The application should also take care to properly indicate any failure of the recording
process, providing meaningful and actionable feedback. <br>
When the recording process is active, a suitable indication should be provided to the user.


## Inspiration Links
* [screen-recorder-ffmpeg-cpp](https://github.com/abdullahfarwees/screen-recorder-ffmpeg-cpp)
* [simplest_ffmpeg_device](https://github.com/leixiaohua1020/simplest_ffmpeg_device)
* [screen-recording-with-ffmpeglib-with-c](https://stackoverflow.com/questions/59929798/screen-recording-with-ffmpeglib-with-c)
* [screen-capture-recorder-to-video-windows-free](https://github.com/rdp/screen-capture-recorder-to-video-windows-free)
* [PostShot](https://github.com/mrousavy/PostShot)
* [screen_capture_lite](https://github.com/smasherprog/screen_capture_lite)

--------------
-------------------
----------

# **Documentazione**

	
## Funzioni generiche


### int initOutputFile()
    Inizializza la comunicazione con la libreria del display, inizializza il file di output e le sue risorse ed in particolare:
        - setta il formato del file
        - setta il nome del file 
        - crea un file video vuoto
        - controlla che il file di output contenga almeno uno stream
        - alloca i dati dello stream e scrive l'header dello stream nel file di output

	

###	int init_fifo() 
    Crea il buffer FIFO in base al formato del campione di output specificato

	
###    int add_samples_to_fifo(uint8_t** converted_input_samples, const int frame_size) 
    Rende il FIFO grande quanto necessario per contenere sia il vecchio che il nuovo campione. Memorizza i nuovi campioni nel buffer FIFO. 
    
 
###    int initConvertedSamples(uint8_t*** converted_input_samples, AVCodecContext* output_codec_context, int frame_size)
    Assegna tanti puntatori quanti sono i canali audio.Ogni puntatore punterà successivamente ai campioni audio dei corrispondenti canali (sebbene possa essere NULL per i formati interleaved).
    Assegna memoria per i campioni di tutti i canali in un unico blocco consecutivo per comodità.

	
###    void CreateThreads()
    Crea thread per video e audio.



###	void SetError(std::string error)
    Imposta la stringa col messaggio di errore in accesso esclusivo.

	
### std::string GetErrorString()
    Legge la stringa con il  messaggio di errore in accesso esclusivo.

    



## Funzioni solo per Windows


### void SetCaptureSystemKey(int valueToSet, LPCWSTR keyToSet);
    Setta i registri nella maniera più opportuna per poter selezionare la finestra del desktop da registrare su Windows.
    
    Con CropX e CropY vengono indicate le coordinate del vertice in alto a sinistra della finestra dello schermo che va registrata, con CropW e CropH viene indicato quanto deve essere grande questa finestra.
     




## Funzioni per il video


### int captureVideoFrames()
    Acquisisce e memorizza i dati video in frame, allocando la memoria richiesta e rilasciandola automaticamente quando necessario.


###	int openVideoDevice()
    Racchiude il setup base per il video. Stabilisce la connessione con lo schermo tramite il dispositivo di input ("x11grab" per Linux e "dshow" per Windows).


###	void generateVideoStream()
    Inizializza il file di output del video e le sue proprietà. 
    In particolare:
        - trova un codificatore (encoder) che matcha con l'ID_codec indicato.
        - ritorna l'encoder in caso di successo, NULL in caso di errore
        - alloca un AVCodecContext e imposta i suoi campi sui valori predefiniti
        - ritorna un AVCodecContext riempito con valori predefiniti o NULL in caso di errore
        - aggiunge un nuovo stream al file media
        - ritorna lo stream appena creato
        - inizializza AVCodecContext per usare un dato AVCodec
        - cerca lo stream di output basandosi sul codec

	

	



## Funzioni per l'audio

	
### int openAudioDevice()
    Racchiude il setup base per l'audio


###   void generateAudioStream()
    Inizializza il file di output dell'audio e le sue proprietà

	
###	void captureAudio() 
    Acquisisce e memorizza i dati audio in pacchett, allocando la memoria richiesta e rilasciandola automaticamente quando necessario.






## API pronte all'uso


	
###    void SetUpScreenRecorder()
    Avvia le funzioni principali:
        - openVideoDevice()
        - openAudioDevice()
        - initOutputFile()
        - CreateThreads()
    Racchiude il setup di base, avvia lo Screen Recorder


###	void StopRecorder()
    Chiude la registrazione.
 
	
### void PauseRecorder()
    Setta la pausa di registrazione.

	
### void CloseRecorder() 
    Chiude la registrazione, chiude il file (se non chiuso precedentemente) e ripulisce le strutture allocate.

###    bool getVideoBool()
    Legge la variabile di stop del video in accesso esclusivo.


###    bool getAudioBool()
    Legge la variabile di stop dell'audio in accesso esclusivo.

###    void AudioStop()
    Imposta lo stop dell'audio in accesso esclusivo.

	
###    void VideoStop()
    Imposta lo stop del video in accesso esclusivo.

