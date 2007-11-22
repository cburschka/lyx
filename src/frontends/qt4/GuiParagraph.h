// -*- C++ -*-
/**
 * \file GuiParagraph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPARAGRAPH_H
#define GUIPARAGRAPH_H

// Uncomment this if you prefer dock widget
//#define USE_DOCK_WIDGET

#include "Layout.h"
#include "ui_ParagraphUi.h"
#include "Dialog.h"
#include "ParagraphParameters.h"
#include "GuiView.h"
#include "qt_helpers.h"
#include "debug.h"

#include <QCloseEvent>
#include <QDialog>
#include <QSettings>
#include <QShowEvent>
#include <QGridLayout>

#include <map>
#include <string>

namespace lyx {
namespace frontend {

class GuiParagraph
	: public QDialog, public Ui::ParagraphUi, public Dialog
{
	Q_OBJECT
public:
	GuiParagraph(LyXView & lv);

	/// update
	void updateView();

private:
	///
	void checkAlignmentRadioButtons();
	///
	void alignmentToRadioButtons(LyXAlignment align = LYX_ALIGN_LAYOUT);
	///
	LyXAlignment getAlignmentFromDialog();
	///
	typedef std::map<LyXAlignment, QRadioButton *> RadioMap;
	RadioMap radioMap;
	typedef std::map<LyXAlignment, docstring> AlignmentLabels;
	AlignmentLabels labelMap;

	QString const alignDefaultLabel;

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
		updateView();  // make sure its up-to-date
		QDialog::show();
		raise();
		activateWindow();
	}
	bool isVisibleView() const { return QDialog::isVisible(); }
	void checkStatus() { updateView(); }
	void updateData(std::string const & data)
	{
		initialiseParams(data);
		updateView();
	}
	std::string name() const { return "paragraph"; }

private:
	QString name_;

	void showEvent(QShowEvent * e)
	{
		QSettings settings;
		QString key = name_ + "/geometry";
		QDialog::restoreGeometry(settings.value(key).toByteArray());
	    QDialog::showEvent(e);
	}

	void closeEvent(QCloseEvent * e)
	{
		QSettings settings;
		QString key = name_ + "/geometry";
		settings.setValue(key, QDialog::saveGeometry());
		QDialog::closeEvent(e);
	}

private Q_SLOTS:
	///
	void changed();
	///
	void on_synchronizedViewCB_toggled();
	///
	void on_restorePB_clicked();
	///
	void on_linespacing_activated(int);
	/// Apply changes
	void on_applyPB_clicked();

private:
	///
	bool initialiseParams(std::string const & /*data*/) { return true; }
	/// clean-up on hide.
	void clearParams() {}
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }
	///
	ParagraphParameters & params();
	///
	ParagraphParameters const & params() const;
	///
	bool haveMulitParSelection();
	///
	bool canIndent() const;
	///
	LyXAlignment alignPossible() const;
	///
	LyXAlignment alignDefault() const;

private:
	ParagraphParameters multiparsel_;
};

} // namespace frontend
} // namespace lyx

#endif // QPARAGRAPH_H
