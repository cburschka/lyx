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
#include "ui_ParagraphUi.h"

#include "ParagraphParameters.h"

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
	void applyView() override;
	void updateView() override;
	void dispatchParams() override;
	void enableView(bool enable) override;
	bool isBufferDependent() const override { return true; }
	FuncCode getLfun() const override { return LFUN_PARAGRAPH_PARAMS_APPLY; }
	void saveSession(QSettings & settings) const override;
	void restoreSession() override;
	//@}

private:
	///
	void checkAlignmentRadioButtons();
	///
	void alignmentToRadioButtons(LyXAlignment align = LYX_ALIGN_LAYOUT);
	///
	LyXAlignment getAlignmentFromDialog() const;
	///
	ParagraphParameters const & params() const;
	///
	bool haveMultiParSelection() const;
	///
	bool canIndent() const;
	///
	bool hasLabelwidth() const;
	///
	LyXAlignment alignPossible() const;
	///
	void setButtons(bool const in_sync);

private Q_SLOTS:
	///
	void changed();
	///
	void on_synchronizedViewCB_stateChanged(int state);
	///
	void on_linespacing_activated(int);
	/// Apply changes
	void on_buttonBox_clicked(QAbstractButton * button);

private:
	///
	typedef std::map<LyXAlignment, QRadioButton *> RadioMap;
	///
	RadioMap radioMap_;

	///
	QString alignDefaultLabel_;
	///
	ParagraphParameters params_;
};

} // namespace frontend
} // namespace lyx

#endif // QPARAGRAPH_H
