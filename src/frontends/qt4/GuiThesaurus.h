// -*- C++ -*-
/**
 * \file GuiThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITHESAURUS_H
#define GUITHESAURUS_H

#include "GuiDialog.h"
#include "ControlThesaurus.h"
#include "ui_ThesaurusUi.h"

class QTreeWidgetItem;

namespace lyx {
namespace frontend {

class GuiThesaurusDialog : public GuiDialog, public Ui::ThesaurusUi
{
	Q_OBJECT

public:
	GuiThesaurusDialog(LyXView & lv);

private Q_SLOTS:
	void change_adaptor();
	void entryChanged();
	void replaceClicked();
	void selectionChanged();
	void selectionClicked(QTreeWidgetItem *, int);
	void itemClicked(QTreeWidgetItem *, int);

private:
	void closeEvent(QCloseEvent * e);
	/// parent controller
	ControlThesaurus & controller() const;
	/// update
	void update_contents();
	///
	void updateLists();
};

} // namespace frontend
} // namespace lyx

#endif // GUITHESAURUS_H
