#pragma once

#include "ScreenRecorder.h"
#include <QWidget>
#include "ui_CropDialog.h"
#include <QtWidgets/qframe.h>

class CropDialog : public QWidget
{
	Q_OBJECT

public:
	explicit CropDialog(QWidget *parent /*= Q_NULLPTR*/, ScreenRecorder* screenRecorder);
	~CropDialog();
	void resizeEvent(QResizeEvent* event);

private slots:
	void on_setScreenSizeButton_clicked();

private:
	Ui::CropDialog ui;

	QFrame* vFrameL; //TODO: utilizzarle
	QFrame* vFrameR;
	QFrame* hFrameU;
	QFrame* hFrameD;
	ScreenRecorder* screenRecorder;
};
