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

#include "DialogView.h"
#include "GuiView.h"
#include "qt_helpers.h"
#include "ui_ParagraphUi.h"

#include "Layout.h"
#include "ParagraphParameters.h"

#include <QDialog>
#include <QSettings>
#include <QShowEvent>
#include <QGridLayout>

#include <map>

namespace lyx {
namespace frontend {

class GuiParagraph
	: public DialogView, public Ui::ParagraphUi
{
	Q_OBJECT
public:
	GuiParagraph(GuiView & lv);

	/// Dialog inherited methods
	//@{
	void applyView();
	void updateView();
	void dispatchParams();
	void enableView(bool enable);
	bool isBufferDependent() const { return true; }
	virtual FuncCode getLfun() const { return LFUN_PARAGRAPH_PARAMS_APPLY; }
	//@}

private:
	///
	void checkAlignmentRadioButtons();
	///
	void alignmentToRadioButtons(LyXAlignment align = LYX_ALIGN_LAYOUT);
	///
	LyXAlignment getAlignmentFromDialog();
	///
	ParagraphParameters & params();
	///
	ParagraphParameters const & params() const;
	///
	bool haveMultiParSelection();
	///
	bool canIndent() const;
	///
	LyXAlignment alignPossible() const;
	///
	LyXAlignment alignDefault() const;

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
	/// Apply changes and close
	void on_okPB_clicked();
	/// Close/Cancel dialog
	void on_closePB_clicked();

private:
	///
	typedef std::map<LyXAlignment, QRadioButton *> RadioMap;
	///
	RadioMap radioMap_;

	///
	QString alignDefaultLabel_;
	///
	ParagraphParameters multiparsel_;
};

} // namespace frontend
} // namespace lyx

#endif // QPARAGRAPH_H
