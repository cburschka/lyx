// -*- C++ -*-
/**
 * \file GuiParagraph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIPARAGRAPH_H
#define GUIPARAGRAPH_H

#include "GuiDialog.h"
#include "ControlParagraph.h"
#include "Layout.h"
#include "ui_ParagraphUi.h"

#include <map>

namespace lyx {
namespace frontend {

class GuiParagraphDialog : public GuiDialog, public Ui::ParagraphUi
{
	Q_OBJECT
public:
	GuiParagraphDialog(LyXView & lv);
	///
	void checkAlignmentRadioButtons();
	///
	void alignmentToRadioButtons(LyXAlignment align = LYX_ALIGN_LAYOUT);
	///
	LyXAlignment getAlignmentFromDialog();
private:
	///
	void closeEvent(QCloseEvent * e);
	///
	typedef std::map<LyXAlignment, QRadioButton *> RadioMap;
	RadioMap radioMap;
	///
	typedef std::map<LyXAlignment, docstring> AlignmentLabels;
	AlignmentLabels labelMap;
		
private Q_SLOTS:
	///
	void change_adaptor();
	///
	void enableLinespacingValue(int);
	/// parent controller
	ControlParagraph & controller();
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
};

} // namespace frontend
} // namespace lyx

#endif // QPARAGRAPH_H
