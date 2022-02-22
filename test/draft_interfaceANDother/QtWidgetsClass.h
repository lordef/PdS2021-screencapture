#pragma once

#include <QWidget>
#include "ui_QtWidgetsClass.h"
#include "ScreenRecorder.h"
#include "ScreenResizeFrame.h"

class QtWidgetsClass : public QWidget, public Ui::QtWidgetsClass
{
	Q_OBJECT

public slots:
	void on_RECButton_clicked();
	void on_STOPButton_clicked();
	void on_PAUSEButton_clicked();
	void on_RESIZEButton_clicked();
	void on_PATHButton_clicked();
	void on_FULLSCREENButton_clicked();
	void on_OPENPATHButton_clicked();
	void CreateErrorMessage();
public:
	QtWidgetsClass(QWidget *parent = Q_NULLPTR);
	~QtWidgetsClass();

private:
	ScreenResizeFrame* scResFr;
	ScreenRecorder* sc;
	std::wstring StringToWstring(const std::string& text);

};
