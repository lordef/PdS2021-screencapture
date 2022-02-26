//#include <bits/stdc++.h>

#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

MainWindow::~MainWindow()
{
}


void MainWindow::on_cropButton_clicked()
{
    cropDlg = new CropDialog(nullptr);
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

    //	sc->recordAudio = checkBox->isChecked();
    //	checkBox->setEnabled(false);


    //	/* Avvia lo screen recorder, avviando l'audio e video device e creando il file output*/
    //	sc->SetUpScreenRecorder();

    showMinimized(); /* riduce la finestra ad icona */
}