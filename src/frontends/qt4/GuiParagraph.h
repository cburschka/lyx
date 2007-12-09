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
#include "DialogView.h"
#include "ParagraphParameters.h"
#include "GuiView.h"
#include "qt_helpers.h"
#include "support/debug.h"

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
	: public DialogView, public Ui::ParagraphUi
{
	Q_OBJECT
public:
	GuiParagraph(GuiView & lv);

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
	void enableView(bool enable);

	std::string name() const { return "paragraph"; }

private:
	QString name_;

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
