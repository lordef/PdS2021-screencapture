#include "QtWidgetsClass.h"
#include "ScreenRecorder.h"
#include <QLabel>
#include <QFileDialog>
#include <QTimer>
#include <stdlib.h>
#include <algorithm>
#include <string>
#if linux
#include <X11/Xlib.h>
#include <QDesktopServices>
#include <libgen.h>         // dirname
#include <unistd.h>         // readlink
#include <linux/limits.h>   // PATH_MAX
#endif
#if WIN32
#include <Windows.h>
#include <shellapi.h>
#pragma comment(lib, "shell32")
#endif
#include <QMessageBox>




/*
* Costruttore di QTWdigetsClass *
* Crea un'istanza di ScreenRecorder
* Crea un timer per un controllo periodico sui messaggi di errore
* Setta il path iniziale in cui salvare il video, che inizialmente è la cartella del programma
*/
QtWidgetsClass::QtWidgetsClass(QWidget* parent)
	: QWidget(parent)
{
	setupUi(this);
	setWindowIcon(QIcon(":/buttons/unicorn.png"));
	sc = new ScreenRecorder;
	QTimer* timer = new QTimer(this);
#if linux
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        char *path;
        if (count != -1) {
            path = dirname(result);
        }
        std::string pathstr=std::string(path);
        pathstr.append("//output.mp4");
        sc->RecordingPath = pathstr;
#endif
	this->pathText->setText(QString::fromStdString(sc->RecordingPath));


	QObject::connect(timer, SIGNAL(timeout()), this, SLOT(CreateErrorMessage()));

	timer->start(1000);
}

QtWidgetsClass::~QtWidgetsClass()
{
}

/*
	Pulsante di REC con le varie funzioni
*/
void QtWidgetsClass::on_RECButton_clicked() {
	/*settaggi di grafica della finestra*/
	setWindowIcon(QIcon(":/buttons/rec-icon-png-23.jpg"));
	recButton->setEnabled(false);
	pauseResumeButton->setEnabled(true);
	stopButton->setEnabled(true);
	sc->recordAudio = checkBox->isChecked();
	checkBox->setEnabled(false);
	sizeButton->setEnabled(false);
	wholeScreenButton->setEnabled(false);
	openPath->setEnabled(false);
	pathButton->setEnabled(false);

	/* Avvia lo screen recorder, avviando l'audio e video device e creando il file output*/
	sc->SetUpScreenRecorder(); 

	showMinimized(); /*riduce la finestra ad icona*/
}

/*
	Pulsante di STOP con le varie funzioni
*/
void QtWidgetsClass::on_STOPButton_clicked() {
	/*Setta le variabili StopVideo e StopAudio per terminare la registrazione*/

	sc->StopRecording();
	
	/*settaggi di grafica della finestra*/
	setWindowIcon(QIcon(":/buttons/unicorn.png"));
	recButton->setEnabled(true);
	pauseResumeButton->setEnabled(false);
	stopButton->setEnabled(false);
	checkBox->setEnabled(true);
	sizeButton->setEnabled(true);
	wholeScreenButton->setEnabled(true);
	openPath->setEnabled(true);
	pathButton->setEnabled(true);
	/*Chiama il distruttore di sc e ne crea una nuova istanza
	per assicurarsi che vengano istanziate strutture dati pulite per FFMPEG*/
	delete sc;
	sc = new ScreenRecorder(pathText->text().toStdString());
}
/*
	Pulsante di STOP con le varie funzioni
*/
void QtWidgetsClass::on_PAUSEButton_clicked() {
	/*Chiama la funzione per settare la variabile di pausa*/
	sc->PauseRecording();
	if (!sc->pauseCapture) {
		/*in uscita dalla pausa, riduce ad icona la finestra e imposta l'icona di registrazione*/
		this->showMinimized();
		setWindowIcon(QIcon(":/buttons/rec-icon-png-23.jpg"));
	}
	else {
		/*Setta l'icona iniziale del programma*/
		setWindowIcon(QIcon(":/buttons/unicorn.png"));
	}
	/*Settaggi di grafica della finestra*/
	recButton->setEnabled(false);
	stopButton->setEnabled(true);
}

/*
	Pulsante di RESIZE
*/
void QtWidgetsClass::on_RESIZEButton_clicked() {
	/*Crea una nuova finestra di tipo ScreenResizeFrame*/
	scResFr = new ScreenResizeFrame(Q_NULLPTR, sc);
	scResFr->show();
}

/*
	Pulsante per impostare dove si vuole salvare il file di output
*/
void QtWidgetsClass::on_PATHButton_clicked() {
	/*Apre un dialog per salvare il file*/
	QString path = QFileDialog::getSaveFileName(this, "Pick save location...", QString::fromStdString(sc->RecordingPath), ".mp4");
	if(path.isEmpty()) {
		/*In caso non si scelga nulla, viene settato il path di default*/
#if WIN32
		sc->RecordingPath = "..\\media\\output.mp4";
#endif
	}
	else {
		sc->RecordingPath = path.toStdString();
	}
	/*Aggiorna la visualizzazione del path nella finestra*/
	this->pathText->setText(QString::fromStdString(sc->RecordingPath));
}

/*
	Pulsante per settare FULLSCREEN
*/
void QtWidgetsClass::on_FULLSCREENButton_clicked()
{
	/*Imposta le coordinate di inizio registrazione a (0,0) 
	cioè l'angolo in alto a sinistra dello schermo*/
	sc->cropX = 0;
	sc->cropY = 0;

	/*Imposta la dimensione di cattura pari alla dimensione dello schermo*/
#if linux
	Display* disp = XOpenDisplay(NULL);
	Screen* scrn = DefaultScreenOfDisplay(disp);
	sc->cropH = scrn->height;
	sc->cropW = scrn->width;
#endif
#if WIN32
	sc->cropW = GetSystemMetrics(SM_CXSCREEN);
	sc->cropH = GetSystemMetrics(SM_CYSCREEN);
#endif
}


/*
	Pulsante per aprire il path di destinazione
*/
void QtWidgetsClass::on_OPENPATHButton_clicked()
{
	/*Ottiene il path assoluto a partire da eventuali path relativi*/
	std::string directory;
	size_t last_slash_idx = sc->RecordingPath.rfind('\\');
	if(last_slash_idx == std::string::npos) last_slash_idx = sc->RecordingPath.rfind('/');
	if(last_slash_idx == std::string::npos) last_slash_idx = sc->RecordingPath.rfind('//');
	if (std::string::npos != last_slash_idx)
	{
		directory = sc->RecordingPath.substr(0, last_slash_idx);
	}
	/*Apre la cartella di destinazione dell'output*/
#if WIN32
	ShellExecute(NULL, L"open", StringToWstring(directory).c_str(), NULL, NULL, SW_NORMAL);
#elif linux
    QDesktopServices::openUrl(QUrl(QString::fromStdString(directory)));
#endif
}


/*Funzione di utility per ottenere una wstring a partire da una string*/
std::wstring QtWidgetsClass::StringToWstring(const std::string& text) {
	return std::wstring(text.begin(), text.end());
}


/*Funzione che viene chiamata periodicamente e controlla la presenza di messaggi di errore*/
void QtWidgetsClass::CreateErrorMessage() {
	auto error_string = sc->GetErrorString();
	if (!error_string.empty()) {
		this->showNormal();
		QMessageBox messageBox;
		messageBox.critical(0, "Error", QString::fromStdString(error_string));
		exit(EXIT_FAILURE);
	}
}



