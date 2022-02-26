#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ScreenRecorder.h"
#include <QWidget>
#include "ui_MainWindow.h"
#include "CropDialog.h"
#include<vector>
#include <iostream>

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

private slots:
	void on_cropButton_clicked();
	
	void on_recordButton_clicked();
	void on_stopButton_clicked();
	void on_pauseButton_clicked();
	void on_resumeButton_clicked();

	/* TODO: test on win */
	void on_muteButton_toggled(bool checked);



private:
	Ui::MainWindow ui;
	ScreenRecorder* screenRecorder;
	CropDialog* cropDlg;

};
#endif // MAINWINDOW_H