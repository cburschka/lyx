// -*- C++ -*-
/**
 * \file BCView.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming
 * \author Baruch Even
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BCVIEW_H
#define BCVIEW_H

namespace lyx {
namespace frontend {

class ButtonController;
class ButtonPolicy;


/** \c BCView is the View to ButtonController's Controller. It
 *  stores the individual GUI widgets and sets their activation state
 *  upon receipt of instructions from the controller.
 *
 *  It is a base class. The true, GUI, instantiations derive from it.
 */
class BCView
{
public:
	BCView(ButtonController & p) : parent(p) {}
	virtual ~BCView() {}

	//@{
	/// Refresh the status of the Ok, Apply, Restore, Cancel buttons.
	virtual void refresh() const = 0;
	/// Refresh the status of any widgets in the read_only list
	virtual void refreshReadOnly() const = 0;
	//@}

	/// A shortcut to the BP of the BC.
	ButtonPolicy const & bp() const;
	ButtonPolicy & bp();

	ButtonController & parent;
};


} // namespace frontend
} // namespace lyx

#endif // BCVIEW_H
