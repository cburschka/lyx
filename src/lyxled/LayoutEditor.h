#ifndef LAYOUTEDITOR_H
#define LAYOUTEDITOR_H

#include <QMainWindow>
#include <QObject>
#include <QTreeView>
#include <QWidget>

class QStandardItem;
class QStandardItemModel;

namespace Ui { class MainWindow; }

namespace lyx {

class LayoutTree;
class LayoutEditor;

/////////////////////////////////////////////////////////////////////////
//
// LayoutTree
//
/////////////////////////////////////////////////////////////////////////

class LayoutTree : public QTreeView
{
	Q_OBJECT

public:
	LayoutTree(QWidget * parent);
};


/////////////////////////////////////////////////////////////////////////
//
// LayoutTree
//
/////////////////////////////////////////////////////////////////////////

class LayoutEditor : public QWidget
{
	Q_OBJECT

public:
	LayoutEditor(QWidget * parent);
};


/////////////////////////////////////////////////////////////////////////
//
// MainWindow
//
/////////////////////////////////////////////////////////////////////////

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();

	void loadLayoutFile(QString const & file);

public:
	void loadLayoutFile(QString const & file, QStandardItem * parent);

	Ui::MainWindow * ui_;
	LayoutTree * view_;
	QStandardItemModel * model_;
};

} // namespace lyx

#endif // LAYOUTEDITOR_H
