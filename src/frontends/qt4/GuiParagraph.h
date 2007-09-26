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

#include "ControlParagraph.h"
#include "Layout.h"
#include "ui_ParagraphUi.h"

#include <QWidget>

#include <map>

namespace lyx {
namespace frontend {

class GuiParagraph : public QWidget, public Ui::ParagraphUi
{
	Q_OBJECT
public:
	GuiParagraph(ControlParagraph & controller);

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

	ControlParagraph & controller_;

	QString const alignDefaultLabel;
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
};

} // namespace frontend
} // namespace lyx

#endif // QPARAGRAPH_H
