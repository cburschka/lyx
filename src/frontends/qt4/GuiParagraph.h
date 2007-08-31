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

#ifndef QPARAGRAPH_H
#define QPARAGRAPH_H

#include "GuiDialogView.h"

#include "Layout.h"
#include "ui_ParagraphUi.h"

#include <QDialog>
#include <QCloseEvent>

#include <map>

namespace lyx {
namespace frontend {

class GuiParagraph;

class GuiParagraphDialog : public QDialog, public Ui::ParagraphUi {
	Q_OBJECT
public:
	GuiParagraphDialog(GuiParagraph * form);
	///
	void checkAlignmentRadioButtons();
	///
	void alignmentToRadioButtons(LyXAlignment align = LYX_ALIGN_LAYOUT);
	///
	LyXAlignment getAlignmentFromDialog();
protected:
	void closeEvent (QCloseEvent * e);
private:
	GuiParagraph * form_;
	typedef std::map<LyXAlignment, QRadioButton *> QPRadioMap;
	QPRadioMap radioMap;
	typedef std::map<LyXAlignment, docstring> QPAlignmentLabels;
	QPAlignmentLabels labelMap;
		
protected Q_SLOTS:
	///
	void change_adaptor();
	///
	void enableLinespacingValue(int);
};


class ControlParagraph;

class GuiParagraph
	: public QController<ControlParagraph, GuiView<GuiParagraphDialog> >
{
public:
	friend class GuiParagraphDialog;

	GuiParagraph(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

} // namespace frontend
} // namespace lyx

#endif // QPARAGRAPH_H
