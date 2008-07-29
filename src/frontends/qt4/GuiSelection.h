// -*- C++ -*-
/**
 * \file qt4/GuiSelection.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISELECTION_H
#define GUISELECTION_H

#include "frontends/Selection.h"

#include <QObject>

namespace lyx {
namespace frontend {

/**
 * The Qt4 version of the Selection.
 */
class GuiSelection : public QObject, public Selection
{
	Q_OBJECT
public:
	GuiSelection();
	virtual ~GuiSelection() {}

	/** Selection overloaded methods
	 */
	//@{
	void haveSelection(bool own);
	docstring const get() const;
	void put(docstring const & str);
	bool empty() const;
	//@}

private Q_SLOTS:
	void on_dataChanged();

private:
	// Direct call clipboard()->text(QClipboard::Selection) inside onDataChanged causes
	// selection to be obtained. Now imagine the some LyX instance A, when making selection -
	// each change triggers onDataChange in all others instances for each mouse
	// or keyboard move. This in turn causes many calls of requestSelection in A
	// which interferes with the selecting itself. As a result middle button pasting
	// for more instances don't work and debugging is a hell. So we just schedule
	// obtaining of selection on the time empty() is actually called.
	mutable bool schedule_check_;
	bool const selection_supported_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISELECTION_H
