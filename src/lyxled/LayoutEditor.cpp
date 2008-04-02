
#include "LayoutEditor.h"

#include "ui_MainWindowUi.h"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTreeView>


namespace lyx {

/////////////////////////////////////////////////////////////////////////
//
// LayoutTree
//
/////////////////////////////////////////////////////////////////////////

LayoutTree::LayoutTree(QWidget * parent) 
	: QTreeView(parent)
{}


/////////////////////////////////////////////////////////////////////////
//
// LayoutEditor
//
/////////////////////////////////////////////////////////////////////////

LayoutEditor::LayoutEditor(QWidget * parent) 
	: QWidget(parent)
{}


/////////////////////////////////////////////////////////////////////////
//
// MainWindow
//
/////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
{
	ui_ = new Ui::MainWindow;
	ui_->setupUi(this);

	model_ = new QStandardItemModel(this);
	view_ = new LayoutTree(this);
	view_->setModel(model_);
	//setCentralWidget(view_);

	ui_->dockLayoutTree->setWidget(view_);
}


MainWindow::~MainWindow()
{
	delete ui_;
}

static bool isInsensitivelyEqual(QString const & s1, QString const & s2)
{
	return s1.compare(s2, Qt::CaseInsensitive) == 0;
}

void MainWindow::loadLayoutFile(QString const & fileName)
{
	loadLayoutFile(fileName, model_->invisibleRootItem());
	view_->expandAll();
}


void MainWindow::loadLayoutFile(QString const & fileName,
	QStandardItem * parent)
{
	QFile file(fileName);
#if 0
	file.open(QIODevice::ReadOnly);
	QString contents = file.readAll();
	file.close();
	qDebug() << "contents: " << contents;
#endif

	file.open(QIODevice::ReadOnly);

	QTextStream ts(&file);
	while (!ts.atEnd()) {
		QList<QStandardItem *> row;
		QString code;
		ts >> code;
		//qDebug() << "CODE: " << code;
		if (code.startsWith('#')) {
			QString line = code + ' ' + ts.readLine();
			//row.append(new QStandardItem("Comment"));
			//row.append(new QStandardItem(code + ' ' + ts.readLine()));
			//parent->appendRow(row);
		} else if (isInsensitivelyEqual(code, "Input")) {
			QString inputFile;
			ts >> inputFile;
			QStandardItem * item = new QStandardItem(inputFile);
			row.append(item);
			parent->appendRow(row);
			inputFile = fileName.left(fileName.lastIndexOf('/')) + '/' + inputFile;
			qDebug() << "INPUT: " << inputFile;
			loadLayoutFile(inputFile, item);
		} else if (isInsensitivelyEqual(code, "Style")) {
			QString style;
			ts >> style;
			//while (!ts.atEnd() && !isInsensitivelyEqual(code, "EndStyle"))
			//	ts >> code;
			QStandardItem * item = new QStandardItem(style);
			row.append(item);
			parent->appendRow(row);
		} else {
			//row.append(new QStandardItem(code));
			//parent->appendRow(row);
		}
	}
	
	file.close();
}

} // namespace lyx

#include "LayoutEditor.h"

#include "ui_MainWindowUi.h"

#include <QDebug>
#include <QFile>
#include <QList>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTreeView>


namespace lyx {

/////////////////////////////////////////////////////////////////////////
//
// LayoutTree
//
/////////////////////////////////////////////////////////////////////////

LayoutTree::LayoutTree(QWidget * parent) 
	: QTreeView(parent)
{}


/////////////////////////////////////////////////////////////////////////
//
// LayoutEditor
//
/////////////////////////////////////////////////////////////////////////

LayoutEditor::LayoutEditor(QWidget * parent) 
	: QWidget(parent)
{}


/////////////////////////////////////////////////////////////////////////
//
// MainWindow
//
/////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
{
	ui_ = new Ui::MainWindow;
	ui_->setupUi(this);

	model_ = new QStandardItemModel(this);
	view_ = new LayoutTree(this);
	view_->setModel(model_);
	//setCentralWidget(view_);

	ui_->dockLayoutTree->setWidget(view_);
}


MainWindow::~MainWindow()
{
	delete ui_;
}

static bool isInsensitivelyEqual(QString const & s1, QString const & s2)
{
	return s1.compare(s2, Qt::CaseInsensitive) == 0;
}

void MainWindow::loadLayoutFile(QString const & fileName)
{
	loadLayoutFile(fileName, model_->invisibleRootItem());
	view_->expandAll();
}


void MainWindow::loadLayoutFile(QString const & fileName,
	QStandardItem * parent)
{
	QFile file(fileName);
#if 0
	file.open(QIODevice::ReadOnly);
	QString contents = file.readAll();
	file.close();
	qDebug() << "contents: " << contents;
#endif

	file.open(QIODevice::ReadOnly);

	QTextStream ts(&file);
	while (!ts.atEnd()) {
		QList<QStandardItem *> row;
		QString code;
		ts >> code;
		//qDebug() << "CODE: " << code;
		if (code.startsWith('#')) {
			QString line = code + ' ' + ts.readLine();
			//row.append(new QStandardItem("Comment"));
			//row.append(new QStandardItem(code + ' ' + ts.readLine()));
			//parent->appendRow(row);
		} else if (isInsensitivelyEqual(code, "Input")) {
			QString inputFile;
			ts >> inputFile;
			QStandardItem * item = new QStandardItem(inputFile);
			row.append(item);
			parent->appendRow(row);
			inputFile = fileName.left(fileName.lastIndexOf('/')) + '/' + inputFile;
			qDebug() << "INPUT: " << inputFile;
			loadLayoutFile(inputFile, item);
		} else if (isInsensitivelyEqual(code, "Style")) {
			QString style;
			ts >> style;
			//while (!ts.atEnd() && !isInsensitivelyEqual(code, "EndStyle"))
			//	ts >> code;
			QStandardItem * item = new QStandardItem(style);
			row.append(item);
			parent->appendRow(row);
		} else {
			//row.append(new QStandardItem(code));
			//parent->appendRow(row);
		}
	}
	
	file.close();
}

} // namespace lyx
