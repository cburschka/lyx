// -*- C++ -*-
/**
 * \file insetbutton.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_BUTTON_H
#define INSET_BUTTON_H

#include "inset.h"
#include "LString.h"
#include <boost/weak_ptr.hpp>

/** Used to provide an inset that looks like a button.
 */
class InsetButton: public Inset {
public:
	///
	void dimension(BufferView *, LyXFont const &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	dispatch_result localDispatch(FuncRequest const & cmd);

protected:
	///
	virtual void cache(BufferView *) const;
	///
	virtual BufferView * view() const;
	/// This should provide the text for the button
	virtual string const getScreenLabel(Buffer const *) const = 0;
private:
	mutable boost::weak_ptr<BufferView> view_;
};

#endif
