// -*- C++ -*-
/**
 * \file Gui.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASE_GUI_H
#define BASE_GUI_H

#include "support/strfwd.h"

#include <vector>


namespace lyx {

class Buffer;
class Inset;

namespace frontend {

class LyXView;


/**
 * A Gui class manages the different frontend elements.
 */
class Gui
{
public:
	virtual ~Gui() {}

	///
	virtual int createRegisteredView() = 0;
	///
	virtual bool unregisterView(int id) = 0;
	///
	virtual bool closeAllViews() = 0;

	///
	virtual LyXView & view(int id) const = 0;
	///
	std::vector<int> const & viewIds() { return view_ids_; }

	///
	virtual void hideDialogs(std::string const & name, Inset * inset) const = 0;
	///
	virtual Buffer const * updateInset(Inset const * inset) const = 0;

protected:
	///
	std::vector<int> view_ids_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_GUI_H
