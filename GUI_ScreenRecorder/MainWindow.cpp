//#include <bits/stdc++.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    screenRecorder = new ScreenRecorder;

    ui.pauseButton->setEnabled(false);
    ui.resumeButton->setEnabled(false);
    ui.stopButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
}


void MainWindow::on_cropButton_clicked()
{
    cropDlg = new CropDialog(nullptr,screenRecorder);
    cropDlg->show();

}



void MainWindow::on_recordButton_clicked()
{
    /* Settaggi di grafica della finestra */
    //setWindowIcon(QIcon(":icons/recording.png"));
    ui.recordButton->setEnabled(false);
    ui.pauseButton->setEnabled(true);
    ui.stopButton->setEnabled(true);
    ui.cropButton->setEnabled(false);
    ui.fullscreenButton->setEnabled(false);
    ui.outPathLabel->setEnabled(false);
    ui.outPathButton->setEnabled(false);
    ui.muteButton->setEnabled(false);


    	/* Avvia lo screen recorder, avviando l'audio e video device e creando il file output*/
    screenRecorder->SetUpScreenRecorder();

    showMinimized(); /* riduce la finestra ad icona */
}

void MainWindow::on_stopButton_clicked()
{
	/*Setta le variabili StopVideo e StopAudio per terminare la registrazione*/

	screenRecorder->StopRecorder();

	/*settaggi di grafica della finestra*/
	//setWindowIcon(QIcon(":/buttons/unicorn.png"));
	ui.recordButton->setEnabled(true);
    ui.muteButton->setEnabled(true);
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
    //setWindowIcon(QIcon(":/buttons/unicorn.png"));

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
    //setWindowIcon(QIcon(":/buttons/rec-icon-png-23.jpg"));
    ui.pauseButton->setEnabled(true);
    /* Settaggi di grafica della finestra */
    ui.recordButton->setEnabled(false);
    ui.stopButton->setEnabled(true);

    ui.resumeButton->setEnabled(false);
}


void MainWindow::on_muteButton_toggled(bool checked)
{
    screenRecorder->isAudioActive = checked; //TODO: forse un set
}

