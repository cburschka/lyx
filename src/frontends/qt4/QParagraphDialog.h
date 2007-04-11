// -*- C++ -*-
/**
 * \file QParagraphDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QPARAGRAPHDIALOG_H
#define QPARAGRAPHDIALOG_H

#include "ui/QParagraphUi.h"

#include <map>
#include <QDialog>
#include <QCloseEvent>
#include "layout.h"

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
protected Q_SLOTS:
	///
	void change_adaptor();
	///
	void enableLinespacingValue(int);
	///
	void on_alignDefaultCB_toggled(bool);
};

} // namespace frontend
} // namespace lyx

#endif // QPARAGRAPHDIALOG_H
