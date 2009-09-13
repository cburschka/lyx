/**
 * \file server_monitor.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 *
 * This program sends commands to a running instance of LyX and
 * receives information back from LyX.
 *
 * Build instructions:
 * 1) Run moc or moc-qt4 on server_monitor.h to produce moc_server_monitor.cpp:
 *    moc-qt4 server_monitor.h -o moc_server_monitor.cpp
 * 2) If the QtGui.pc file is not in the pkg-config search path, find the
 *    directory where it is located (e.g., use the command `locate QtGui.pc')
 *    and set the environment variable PKG_CONFIG_PATH to this directory.
 *    For example:
 *      export PKG_CONFIG_PATH=/path/to/directory    (if using bash)
 *      setenv PKG_CONFIG_PATH /path/to/directory    (if using tcsh)
 *    If the command `pkg-config --modversion QtGui' does not complain and
 *    prints the Qt version, you don't need to set PKG_CONFIG_PATH.
 * 3) Compile using the following command:
 *    g++ server_monitor.cpp -o monitor -I. `pkg-config --cflags --libs QtGui`
 *
 * Alternatively, you can create a Makefile with qmake and then build
 * the executable by running make (or nmake, if you use msvc):
 *    qmake
 *    make
 *
 * Usage:
 * 1) Set the LyXserver pipe path in the LyX preferences (on *nix you can use
 *    any path, for example ~/.lyx/lyxpipe, whereas on Windows the path has
 *    to start with `\\.\pipe\', for example you can use \\.\pipe\lyxpipe).
 * 2) Quit and restart LyX.
 * 3) Launch this program, adjust the pipe name to match that one used in LyX,
 *    push the button labeled "Open pipes" and then try issuing some commands.
 */

#include <QApplication>
#include <QtGui>
#include <QtDebug>

#include "server_monitor.h"

LyXServerMonitor::LyXServerMonitor()
	: pipein(-1), pipeout(-1), thread_exit(false), lyx_listen(false)
{
	createGridGroupBox();
	createCmdsGroupBox();

	char const * const home = getenv("HOME");
	QString const pipeName = (home && home[0]) ?
	    QString::fromUtf8(home) + "/.lyx/lyxpipe" : "\\\\.\\pipe\\lyxpipe";

	pipeNameLE->setText(pipeName);
	clientNameLE->setText("monitor");
	submitCommandPB->setDisabled(true);
	closePipesPB->setDisabled(true);

	connect(openPipesPB, SIGNAL(clicked()), this, SLOT(openPipes()));
	connect(closePipesPB, SIGNAL(clicked()), this, SLOT(closePipes()));
	connect(submitCommandPB, SIGNAL(clicked()), this, SLOT(submitCommand()));
	connect(donePB, SIGNAL(clicked()), this, SLOT(reject()));

	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->addWidget(gridGB);
	mainLayout->addWidget(horizontalGB);
	setLayout(mainLayout);

	setWindowTitle("LyX Server Monitor");
}


LyXServerMonitor::~LyXServerMonitor()
{
	if (pipein != -1)
		closePipes();
}


void LyXServerMonitor::createGridGroupBox()
{
	gridGB = new QGroupBox;
	QGridLayout * layout = new QGridLayout;

	labels[0] = new QLabel("Pipe name");
	pipeNameLE = new QLineEdit;
	layout->addWidget(labels[0], 0, 0, Qt::AlignRight);
	layout->addWidget(pipeNameLE, 0, 1);

	labels[1] = new QLabel("Command");
	commandLE = new QLineEdit;
	layout->addWidget(labels[1], 1, 0, Qt::AlignRight);
	layout->addWidget(commandLE, 1, 1);

	labels[2] = new QLabel("Client name");
	clientNameLE = new QLineEdit;
	layout->addWidget(labels[2], 0, 2, Qt::AlignRight);
	layout->addWidget(clientNameLE, 0, 3);

	labels[3] = new QLabel("Argument");
	argumentLE = new QLineEdit;
	layout->addWidget(labels[3], 1, 2, Qt::AlignRight);
	layout->addWidget(argumentLE, 1, 3);

	labels[4] = new QLabel("Info");
	infoLB = new QLabel;
	infoLB->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	infoLB->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	layout->addWidget(labels[4], 2, 0, Qt::AlignRight);
	layout->addWidget(infoLB, 2, 1, 1, 3);

	labels[5] = new QLabel("Notify");
	notifyLB = new QLabel;
	notifyLB->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	notifyLB->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	layout->addWidget(labels[5], 3, 0, Qt::AlignRight);
	layout->addWidget(notifyLB, 3, 1, 1, 3);

	layout->setColumnMinimumWidth(1, 200);
	layout->setColumnMinimumWidth(3, 200);
	gridGB->setLayout(layout);
}


void LyXServerMonitor::createCmdsGroupBox()
{
	horizontalGB = new QGroupBox;
	QHBoxLayout * layout = new QHBoxLayout;

	openPipesPB = new QPushButton("&Open pipes");
	layout->addWidget(openPipesPB);

	closePipesPB = new QPushButton("C&lose pipes");
	layout->addWidget(closePipesPB);

	submitCommandPB = new QPushButton("&Submit Command");
	layout->addWidget(submitCommandPB);

	donePB = new QPushButton("&Done");
	layout->addWidget(donePB);

	horizontalGB->setLayout(layout);
}


void LyXServerMonitor::readPipe()
{
	int n;
	errno = 0;
	bool notified = false;

	while ((n = ::read(pipeout, pipedata, BUFSIZE - 1)) && !thread_exit) {
		if (n > 0) {
			pipedata[n] = 0;
			QString const fromLyX =
				QString::fromUtf8(pipedata).trimmed();
			qWarning() << "monitor: Coming: " << fromLyX;
			if (fromLyX.startsWith("LYXSRV:")) {
				if (fromLyX.contains("bye")) {
					qWarning() << "monitor: LyX has closed "
						      "connection!";
					pipethread->emitNotice(fromLyX);
					notified = true;
					break;
				}
				if (fromLyX.contains("hello")) {
					lyx_listen = true;
					qWarning() << "monitor: "
						      "LyX is listening!";
					submitCommandPB->setDisabled(false);
				}
			}
			if (fromLyX[0] == QLatin1Char('I'))
				pipethread->emitInfo(fromLyX);
			else
				pipethread->emitNotice(fromLyX);
#ifdef _WIN32
			// On Windows, we have to close and reopen
			// the pipe after each use.
			::close(pipeout);
			pipeout = ::open(
				outPipeName().toLocal8Bit().constData(),
				O_RDONLY);
			if (pipeout < 0) {
				perror("monitor");
				pipethread->emitNotice("An error occurred, "
							"closing pipes");
				notified = true;
				break;
			}
#endif
		} else if (n < 0) {
#ifdef __CYGWIN__
			if (errno == ECOMM) {
				// When talking to a native Windows version of
				// LyX, the second time we try to use the pipe,
				// read() fails with ECOMM. In this case, we
				// have to simply close and reopen it.
				::close(pipeout);
				pipeout = ::open(
					outPipeName().toLocal8Bit().constData(),
					O_RDONLY);
				if (pipeout >= 0)
					continue;
			}
#endif
			perror("monitor");
			pipethread->emitNotice("An error occurred, closing pipes");
			notified = true;
			break;
		} else
			break;
	}

	if (!notified) {
		if (thread_exit) {
			qWarning() << "monitor: Closing pipes";
			pipethread->emitNotice("Closing pipes");
		} else {
			qWarning() << "monitor: LyX has closed connection!";
			pipethread->emitNotice("LyX has closed connection!");
		}
	}
	QEvent * event = new QEvent(QEvent::User);
	QCoreApplication::postEvent(this, event);
	lyx_listen = false;
	if (!thread_exit)
		pipethread->emitClosing();
}


bool LyXServerMonitor::event(QEvent * e)
{
	if (e->type() == QEvent::User) {
		pipethread->wait();
		thread_exit = false;
		delete pipethread;
		return true;
	}
	return QDialog::event(e);
}


void LyXServerMonitor::showInfo(QString const & msg)
{
	infoLB->setText(msg);
	notifyLB->clear();
}


void LyXServerMonitor::showNotice(QString const & msg)
{
	infoLB->clear();
	notifyLB->setText(msg);
}


void LyXServerMonitor::openPipes()
{
	if (pipein == -1) {
		qWarning() << "monitor: Opening pipes " << inPipeName()
			   << " and " << outPipeName();
		pipein = ::open(inPipeName().toLocal8Bit().constData(),
				O_WRONLY);
		pipeout = ::open(outPipeName().toLocal8Bit().constData(),
				 O_RDONLY);
		if (pipein < 0 || pipeout < 0) {
			qWarning() << "monitor: Could not open the pipes";
			infoLB->clear();
			notifyLB->setText("Could not open the pipes");
			if (pipein >= 0 || pipeout >= 0)
				closePipes();
			return;
		}
		pipethread = new ReadPipe(this);
		pipethread->start();
		if (!pipethread->isRunning()) {
			qWarning() << "monitor: Could not create pipe thread";
			infoLB->clear();
			notifyLB->setText("Could not create pipe thread");
			closePipes();
			return;
		}
		connect(pipethread, SIGNAL(info(QString const &)),
			this, SLOT(showInfo(QString const &)));
		connect(pipethread, SIGNAL(notice(QString const &)),
			this, SLOT(showNotice(QString const &)));
		connect(pipethread, SIGNAL(closing()),
			this, SLOT(closePipes()));
		openPipesPB->setDisabled(true);
		closePipesPB->setDisabled(false);
		// greet LyX
		QString const clientname = clientNameLE->text();
		snprintf(buffer, BUFSIZE - 1,
			"LYXSRV:%s:hello\n", clientname.toUtf8().constData());
		buffer[BUFSIZE - 1] = '\0';
		::write(pipein, buffer, strlen(buffer));
	} else
		qWarning() << "monitor: Pipes already opened, close them first\n";
}


void LyXServerMonitor::closePipes()
{
	if (pipein == -1 && pipeout == -1) {
		qWarning() << "monitor: Pipes are not opened";
		return;
	}

	if (pipein >= 0) {
		if (lyx_listen) {
			lyx_listen = false;
			QString const clientname = clientNameLE->text();
			if (pipethread->isRunning()) {
				thread_exit = true;
				// The thread, currently blocked on the read()
				// call, will be waked up by the reply from
				// LyX and will exit.
				snprintf(buffer, BUFSIZE - 1,
					"LYXCMD:%s:message:Client '%s' is leaving\n",
					clientname.toUtf8().constData(),
					clientname.toUtf8().constData());
				buffer[BUFSIZE - 1] = '\0';
				::write(pipein, buffer, strlen(buffer));
			}
			// Say goodbye
			snprintf(buffer, BUFSIZE - 1, "LYXSRV:%s:bye\n",
				 clientname.toUtf8().constData());
			buffer[BUFSIZE - 1] = '\0';
			::write(pipein, buffer, strlen(buffer));
			pipethread->wait();
			thread_exit = false;
		}
		::close(pipein);
	}

	if (pipeout >= 0)
		::close(pipeout);
	pipein = pipeout = -1;
	submitCommandPB->setDisabled(true);
	openPipesPB->setDisabled(false);
	closePipesPB->setDisabled(true);
}


void LyXServerMonitor::submitCommand()
{
	if (pipein >= 0) {
		QString const command = commandLE->text();
		QString const argument = argumentLE->text();
		QString const clientname = clientNameLE->text();
		snprintf(buffer, BUFSIZE - 2, "LYXCMD:%s:%s:%s",
			 clientname.toUtf8().constData(),
			 command.toUtf8().constData(),
			 argument.toUtf8().constData());
		buffer[BUFSIZE - 1] = '\0';
		qWarning() << "monitor: Sending: " << buffer;
		strcat(buffer, "\n");
		::write(pipein, buffer, strlen(buffer));
	} else
		qWarning() << "monitor: Pipe is not opened";
}


int main(int argc, char * argv[])
{
	QApplication app(argc, argv);
	LyXServerMonitor dialog;
	return dialog.exec();
}

#include "moc_server_monitor.cpp"
