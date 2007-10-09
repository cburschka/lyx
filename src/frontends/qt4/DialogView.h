// -*- C++ -*-
/**
 * \file DialogView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef DIALOG_VIEW_H
#define DIALOG_VIEW_H

#include "Dialog.h"
#include "GuiView.h"
#include "qt_helpers.h"
#include "debug.h"

#include <QCloseEvent>
#include <QDialog>
#include <QSettings>
#include <QShowEvent>
#include <QGridLayout>

#include <string>

namespace lyx {
namespace frontend {

/// Window Dialog container for LyX dialogs.
/// This template class that encapsulates a given Widget inside a
/// QDialog and presents a Dialog interface
template<class MyWidget>
class DialogView : public QDialog, public Dialog
{
public:
	DialogView(
		GuiViewBase & parent, ///< the main window where to dock.
		std::string const & name, ///< dialog identifier.
		bool modal = false, ///< Window modality.
		Qt::WindowFlags flags = 0
		)
		: QDialog(&parent, flags), name_(name)
	{
		setModal(modal);
		QGridLayout * gridLayout = new QGridLayout(this);
		gridLayout->setMargin(0);
		widget_ = new MyWidget(*this, this);
		gridLayout->addWidget(widget_);
		setWindowTitle("LyX: " + widget_->windowTitle());
	}

	/// Dialog inherited methods
	//@{
	void applyView() {}
	void hideView()
	{
		clearParams();
		QDialog::hide();
	}
	void showData(std::string const & data)
	{
		initialiseParams(data);
		showView();
	}
	void showView()
	{
		widget_->updateView();  // make sure its up-to-date
		QDialog::show();
		raise();
		activateWindow();
	}
	bool isVisibleView() const { return QDialog::isVisible(); }
	void checkStatus() { updateView(); }
	void redraw() { redrawView(); }
	void redrawView() {}
	void updateData(std::string const & data)
	{
		initialiseParams(data);
		updateView();
	}
	void updateView()
	{
		widget_->updateView();
	}
	void partialUpdateView(int /*id*/) {}
	std::string name() const { return name_; }
	//@}
private:
	/// The encapsulated widget.
	MyWidget * widget_;
	std::string name_;

	void showEvent(QShowEvent * e)
	{
#if (QT_VERSION >= 0x040200)
		QSettings settings;
		std::string key = name_ + "/geometry";
		QDialog::restoreGeometry(settings.value(key.c_str()).toByteArray());
#endif
	    QDialog::showEvent(e);
	}

	void closeEvent(QCloseEvent * e)
	{
#if (QT_VERSION >= 0x040200)
		QSettings settings;
		std::string key = name_ + "/geometry";
		settings.setValue(key.c_str(), QDialog::saveGeometry());
#endif
	  QDialog::closeEvent(e);
	}
};

} // frontend
} // lyx

#endif // DIALOG_VIEW_H
