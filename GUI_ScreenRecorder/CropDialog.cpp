#include "CropDialog.h"
//#include "ui_CropDialog.h"

CropDialog::CropDialog(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
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
    /*
    sc->cropX = this->pos().x();
    sc->croY = this->pos().y();
    sc->cropW = this->size().width();
    sc->cropH = this->size().height();
    */
    hide();
}