// -*- C++ -*-
/**
 * \file QParagraph.h
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

#include "QDialogView.h"

#include "Layout.h"
#include "ui/ParagraphUi.h"

#include <QDialog>
#include <QCloseEvent>

#include <map>

namespace lyx {
namespace frontend {

class QParagraph;

class QParagraphDialog : public QDialog, public Ui::QParagraphUi {
	Q_OBJECT
public:
	QParagraphDialog(QParagraph * form);
	///
	void checkAlignmentRadioButtons();
	///
	void alignmentToRadioButtons(LyXAlignment align = LYX_ALIGN_LAYOUT);
	///
	LyXAlignment getAlignmentFromDialog();
protected:
	void closeEvent (QCloseEvent * e);
private:
	QParagraph * form_;
	typedef std::map<LyXAlignment, QRadioButton *> QPRadioMap;
	QPRadioMap radioMap;
	typedef std::map<LyXAlignment, std::string> QPAlignmentLabels;
	QPAlignmentLabels labelMap;
		
protected Q_SLOTS:
	///
	void change_adaptor();
	///
	void enableLinespacingValue(int);
};


class ControlParagraph;

class QParagraph
	: public QController<ControlParagraph, QView<QParagraphDialog> >
{
public:
	friend class QParagraphDialog;

	QParagraph(Dialog &);
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
