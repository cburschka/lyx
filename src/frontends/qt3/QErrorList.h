// -*- C++ -*-
/**
 * \file QErrorList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QERRORLIST_H
#define QERRORLIST_H

#include "QDialogView.h"

namespace lyx {
namespace frontend {

class ControlErrorList;
class QErrorListDialog;

class QErrorList :
	public QController<ControlErrorList, QView<QErrorListDialog> >
{
public:
	friend class QErrorListDialog;

	QErrorList(Dialog &);
private:
	/// select an entry
	void select(int item);
	/// required apply
	virtual void apply() {}
	/// build dialog
	virtual void build_dialog();
	/// update contents
	virtual void update_contents();
};

} // namespace frontend
} // namespace lyx

#endif // QERRORLIST_H
