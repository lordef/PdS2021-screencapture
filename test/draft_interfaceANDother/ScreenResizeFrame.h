#pragma once

#include <QWidget>
#include <QtWidgets/qframe.h>
#include "ui_ScreenResizeFrame.h"
#include "ScreenRecorder.h"
class ScreenResizeFrame : public QWidget
{
	Q_OBJECT

public:
	ScreenResizeFrame(QWidget *parent = Q_NULLPTR);
	ScreenResizeFrame(QWidget* parent = Q_NULLPTR, ScreenRecorder* sc_rec = nullptr);
	~ScreenResizeFrame();

	void resizeEvent(QResizeEvent* event);

public slots:
	void on_SETSIZEButton_clicked();
private:
	Ui::ScreenResizeFrame ui;
	QFrame* vFrameL;
	QFrame* vFrameR;
	QFrame* hFrameU;
	QFrame* hFrameD;
	ScreenRecorder* sc;
};
