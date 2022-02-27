#include "CropDialog.h"
//#include "ui_CropDialog.h"

CropDialog::CropDialog(QWidget *parent, ScreenRecorder *screenRecorder)
	: QWidget(parent)
{
	ui.setupUi(this);
    this->screenRecorder = screenRecorder;
}

CropDialog::~CropDialog()
{
}

void CropDialog::resizeEvent(QResizeEvent* event)
{
    //QRect oldpos = ui.setScreenSizeButton->geometry();
    ui.setScreenSizeButton->
        move((size().width() / 2) - (ui.setScreenSizeButton->size().width() / 2),
            (size().height() / 2) - (ui.setScreenSizeButton->size().height() / 2));

    QWidget::resizeEvent(event);

}

void CropDialog::on_setScreenSizeButton_clicked()
{
    //TODO: sc stands for ScreenRecorder
    //screenRecorder->setCrop(pos().x(), pos().y(), size().width(), size().height());
    screenRecorder->setCrop(pos().x(), pos().y(), size().width(), size().height());
    hide();
}