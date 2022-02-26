#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

private:
	Ui::MainWindow ui;
	CropDialog* cropDlg;

};
#endif // MAINWINDOW_H