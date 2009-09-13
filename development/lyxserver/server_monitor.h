/**
 * \file server_monitor.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef SERVER_MONITOR_H
#define SERVER_MONITOR_H

#include <errno.h>
#include <fcntl.h>

#include <QDialog>
#include <QFile>
#include <QLineEdit>

#ifdef _WIN32
#include <windows.h>
#ifdef _MSC_VER
#include <io.h>
#define open _open
#define close _close
#define read _read
#define write _write
#define snprintf _snprintf
#define O_RDONLY _O_RDONLY
#define O_WRONLY _O_WRONLY
#endif
#endif

class QGroupBox;
class QLabel;
class QPushButton;
class ReadPipe;

class LyXServerMonitor : public QDialog
{
	Q_OBJECT

	enum { BUFSIZE = 512 };

public:
	LyXServerMonitor();
	///
	~LyXServerMonitor();
	///
	void readPipe();
	///
	QString inPipeName() { return pipeNameLE->text() + ".in"; }
	///
	QString outPipeName() { return pipeNameLE->text() + ".out"; }

public Q_SLOTS:
	void openPipes();
	void closePipes();
	void submitCommand();
	void showInfo(QString const &);
	void showNotice(QString const &);

private:
	void createCmdsGroupBox();
	void createGridGroupBox();
	bool event(QEvent *);

	QGroupBox * horizontalGB;
	QGroupBox * gridGB;
	QLabel * labels[6];
	QLineEdit * pipeNameLE;
	QLineEdit * clientNameLE;
	QLineEdit * commandLE;
	QLineEdit * argumentLE;
	QLabel * infoLB;
	QLabel * notifyLB;
	QPushButton * openPipesPB;
	QPushButton * closePipesPB;
	QPushButton * submitCommandPB;
	QPushButton * donePB;

	int pipein;
	int pipeout;
	bool thread_exit;
	bool lyx_listen;
	char buffer[BUFSIZE];
	char pipedata[BUFSIZE];
	ReadPipe * pipethread;
};


class ReadPipe : public QThread
{
	Q_OBJECT

public:
	ReadPipe(LyXServerMonitor * monitor) : lyxmonitor(monitor) {}
	///
	void run() { lyxmonitor->readPipe(); }
	///
	void emitInfo(QString const & msg) { emit info(msg); }
	///
	void emitNotice(QString const & msg) { emit notice(msg); }
	///
	void emitClosing() { emit closing(); }

signals:
	void info(QString const &);
	void notice(QString const &);
	void closing();

private:
	LyXServerMonitor * lyxmonitor;
};

#endif
