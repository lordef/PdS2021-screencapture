//#include <bits/stdc++.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    screenRecorder = new ScreenRecorder;
    setWindowIcon(QIcon("rate.png"));


    ui.pauseButton->setEnabled(false);
    ui.resumeButton->setEnabled(false);
    ui.stopButton->setEnabled(false);
    ui.activeAudioCheckBox->setChecked(true);
    ui.outPathLabel->setText(QString::fromStdString(screenRecorder->getOutputPath()));
}

MainWindow::~MainWindow()
{
}


void MainWindow::on_cropButton_clicked()
{
    cropDlg = new CropDialog(nullptr, screenRecorder);
    cropDlg->show();
}

void MainWindow::on_fullscreenButton_clicked()
{
    /* 
        Reset dei valori di crop, 
        le API si occupano si settare il valore alla dimensione massima del display 
    */
    screenRecorder->setCrop(0,0,0,0); 

}

/*
    Pulsante per impostare dove si vuole salvare il file di output
*/
void MainWindow::on_outPathButton_clicked() {

    /*Apre un dialog per salvare il file*/
    QString outPath = QFileDialog::getSaveFileName(this, "Choose save location...", QString::fromStdString(screenRecorder->getOutputPath())/*, ".mp4"*/);
    
    if (outPath.isEmpty()) {
        /*In caso non si scelga nulla, viene settato il path di default*/
    #if WIN32
        screenRecorder->setOutputPath("..\\media\\output.mp4");
    #elif __linux__
        screenRecorder->setOutputPath("media/output.mp4");
    #endif

    }
    else {
        std::string mp4 = ".mp4";
        if(outPath.toStdString().substr(outPath.length() - 4) == mp4.c_str())
            screenRecorder->setOutputPath(outPath.toStdString());
        else
            screenRecorder->setOutputPath(outPath.toStdString() + ".mp4");
        
    }

    /*Aggiorna la visualizzazione del path nella label della finestra*/
    ui.outPathLabel->setText(QString::fromStdString(screenRecorder->getOutputPath()));
    //this->pathText->setText(QString::fromStdString(screenRecorder->getOutputPath());
}



void MainWindow::on_recordButton_clicked()
{
    /* Settaggi di grafica della finestra */
    setWindowIcon(QIcon("recording.png"));
    ui.recordButton->setEnabled(false);
    ui.pauseButton->setEnabled(true);
    ui.stopButton->setEnabled(true);
    ui.cropButton->setEnabled(false);
    ui.fullscreenButton->setEnabled(false);
    ui.outPathLabel->setEnabled(false);
    ui.outPathButton->setEnabled(false);
    screenRecorder->SetIsAudioActive(ui.activeAudioCheckBox->isChecked());
    ui.activeAudioCheckBox->setEnabled(false);

    /* Avvia lo screen recorder, avviando l'audio e video device e creando il file output*/
    screenRecorder->SetUpScreenRecorder();

    showMinimized(); /* riduce la finestra ad icona */
}

void MainWindow::on_stopButton_clicked()
{
	/*Setta le variabili StopVideo e StopAudio per terminare la registrazione*/

	screenRecorder->StopRecorder();

	/*settaggi di grafica della finestra*/
	setWindowIcon(QIcon("rate.png"));
	ui.recordButton->setEnabled(true);
    ui.activeAudioCheckBox->setEnabled(true);
    ui.activeAudioCheckBox->setChecked(true);
	ui.pauseButton->setEnabled(false);
    ui.resumeButton->setEnabled(false);
	ui.stopButton->setEnabled(false);
	ui.cropButton->setEnabled(true);
	ui.fullscreenButton->setEnabled(true);
	ui.outPathButton->setEnabled(true);
	ui.outPathLabel->setEnabled(true);
	/*Chiama il distruttore di screenRecorder e ne crea una nuova istanza
	per assicurarsi che vengano istanziate strutture dati pulite per FFMPEG*/
	delete screenRecorder;
	screenRecorder = new ScreenRecorder(/*pathText->text().toStdString()*/); //TODO

}

void MainWindow::on_pauseButton_clicked() {
    /* Chiama la funzione per settare la variabile di pausa a true */
    screenRecorder->PauseRecorder();
    ui.resumeButton->setEnabled(true);

    /* Setta l'icona iniziale del programma */
    setWindowIcon(QIcon("pause.png"));

    /* Settaggi di grafica della finestra */
    ui.recordButton->setEnabled(false);
    ui.stopButton->setEnabled(true);

    ui.pauseButton->setEnabled(false);
}

void MainWindow::on_resumeButton_clicked() {
    /* Chiama la funzione per settare la variabile di pausa a true */
    screenRecorder->PauseRecorder();
    /*in uscita dalla pausa, riduce ad icona la finestra e imposta l'icona di registrazione*/
    this->showMinimized();
    setWindowIcon(QIcon("recording.png"));

    ui.pauseButton->setEnabled(true);
    /* Settaggi di grafica della finestra */
    ui.recordButton->setEnabled(false);
    ui.stopButton->setEnabled(true);

    ui.resumeButton->setEnabled(false);
}


