// -*- C++ -*-
/**
 * \file FindAndReplace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSEARCHADV_H
#define QSEARCHADV_H

#include "GuiWorkArea.h"

#include "DockView.h"
#include "ui_FindAndReplaceUi.h"

#include "BufferView.h"
#include "Buffer.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "Text.h"

#include <QDialog>

#include <string>

namespace lyx {
namespace frontend {

class FindAndReplace : public DockView, public Ui::FindAndReplaceUi
{
	Q_OBJECT
public:
	FindAndReplace(GuiView & parent);

	bool initialiseParams(std::string const &);
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }
	void selectAll();

	/// update
	void updateView() {}
	//virtual void update_contents() {}

protected Q_SLOTS:
	void on_findNextPB_clicked();
	void on_findPrevPB_clicked();
	void on_replacePB_clicked();
	void on_replaceallPB_clicked();
	void on_closePB_clicked();
	void on_regexpInsertCombo_currentIndexChanged(int index);

protected:
	void find(bool backwards);
	virtual bool wantInitialFocus() const { return true; }

private:
	// add a string to the combo if needed
	void remember(std::string const & find, QComboBox & combo);
	void findAdv(bool casesensitive,
			bool matchword, bool backwards,
			bool expandmacros, bool ignoreformat);

private:
	/// Apply changes
	virtual void apply() {}

	void find(docstring const & str, int len, bool casesens,
		  bool words, bool backwards, bool expandmacros);

	void replace(docstring const & findstr,
		     docstring const & replacestr,
		     bool casesens, bool words, bool backwards, bool expandmacros, bool all);
	bool eventFilter(QObject *obj, QEvent *event);
};


} // namespace frontend
} // namespace lyx

#endif // QSEARCHADV_H
