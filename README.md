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

