// -*- C++ -*-
/**
 * \file QMathDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QMATHDIALOG_H
#define QMATHDIALOG_H

#include "ui/QMathUi.h"
#include <string>

#include <QDialog>

class QListWidgetItem;

namespace lyx {
namespace frontend {

class IconPalette;

class QMAction : public QAction {
	Q_OBJECT
public:
	QMAction( const QString & text, const std::string & action, QObject * parent)
	: QAction(text,parent), action_(action) {
		connect(this, SIGNAL(triggered()), this, SLOT(action()));
	}
Q_SIGNALS:
	void action(const std::string &);
protected Q_SLOTS:
	void action() {
                // emit signal
		action(action_);
	}
private:
	std::string action_;
};


class QMath;

class QMathDialog : public QDialog, public Ui::QMathUi
{
	Q_OBJECT
public:
	QMathDialog(QMath * form);
public Q_SLOTS:
	virtual void delimiterClicked();
	virtual void expandClicked();
	virtual void functionSelected(QListWidgetItem *);
	virtual void matrixClicked();
	virtual void subscriptClicked();
	virtual void superscriptClicked();
	virtual void equationClicked();
	void symbol_clicked(const std::string &);
	void insertCubeRoot();

	/// about to show a symbol panel
	void showingPanel(int);
protected:
	//needed ? virtual void closeEvent(QCloseEvent * e);
private:
	/// add item to popup menu
	void addMenuItem(QMenu * menu, const QString & label, const std::string &);
	/// make a symbol panel
	IconPalette * makePanel(QWidget * parent, char const ** entries);
	/// add a symbol panel
	void addPanel(int num);

	/// owning form
	QMath * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QMATHDIALOG_H
