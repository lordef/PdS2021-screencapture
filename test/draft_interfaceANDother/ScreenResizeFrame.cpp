#include "ScreenResizeFrame.h"
#include <QtWidgets/qframe.h>
#include <QVBoxLayout>
#include <ScreenRecorder.h>


ScreenResizeFrame::ScreenResizeFrame(QWidget* parent, ScreenRecorder* sc_rec)
	: QWidget(parent)
{
	ui.setupUi(this);
	sc = sc_rec;
	setWindowFlags(Qt::Window
	| Qt::WindowMaximizeButtonHint);
}

ScreenResizeFrame::ScreenResizeFrame(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}


ScreenResizeFrame::~ScreenResizeFrame()
{
}

void ScreenResizeFrame::resizeEvent(QResizeEvent* event)
{
	QRect oldpos = ui.setSizeButton->geometry();
    ui.setSizeButton->move(((size().width()/2)-(ui.setSizeButton->size().width()/2)), ((size().height()/2) - (ui.setSizeButton->size().height() / 2)));
	
    QWidget::resizeEvent(event);
}

void ScreenResizeFrame::on_SETSIZEButton_clicked()
{
	sc->cropX = this->pos().x();
	sc->cropY = this->pos().y();
	sc->cropW = this->size().width();
	sc->cropH = this->size().height();
	hide();
}
