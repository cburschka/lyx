// -*- C++ -*-
/**
 * \file insetbase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author none
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETBASE_H
#define INSETBASE_H

#include "LString.h"

#include <vector>

class BufferView;
class FuncRequest;
class MetricsInfo;
class Dimension;
class PainterInfo;
class LaTeXFeatures;

/** Dispatch result codes
		DISPATCHED          = the inset catched the action
		DISPATCHED_NOUPDATE = the inset catched the action and no update
				is needed here to redraw the inset
		FINISHED            = the inset must be unlocked as a result
				of the action
		FINISHED_RIGHT      = FINISHED, but put the cursor to the RIGHT of
				the inset.
		FINISHED_UP         = FINISHED, but put the cursor UP of
				the inset.
		FINISHED_DOWN       = FINISHED, but put the cursor DOWN of
				the inset.
		UNDISPATCHED        = the action was not catched, it should be
				dispatched by lower level insets
*/
enum dispatch_result {
	UNDISPATCHED = 0,
	DISPATCHED,
	DISPATCHED_NOUPDATE,
	FINISHED,
	FINISHED_RIGHT,
	FINISHED_UP,
	FINISHED_DOWN,
	DISPATCHED_POP
};


/// Common base class to all insets
class InsetBase {
public:
	///
	typedef int      difference_type;
	/// short of anything else reasonable
	typedef size_t   size_type;
	/// type for cell indices
	typedef size_t  idx_type;
	/// type for cursor positions
	typedef size_t  pos_type;
	/// type for row numbers
	typedef size_t  row_type;
	/// type for column numbers
	typedef size_t  col_type;

	/// virtual base class destructor
	virtual ~InsetBase() {}
	/// replicate ourselves
	virtual std::auto_ptr<InsetBase> clone() const = 0;

	// the real dispatcher
	virtual dispatch_result dispatch
		(FuncRequest const & cmd, idx_type & idx, pos_type & pos);

	/// small wrapper for the time being
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;

	/// Methods to cache and retrieve a cached BufferView.
	virtual void cache(BufferView *) const {}
	///
	virtual BufferView * view() const { return 0; }
	/// request "external features"
	virtual void validate(LaTeXFeatures &) const {}
	/// fill in all labels in the inset
	virtual void getLabelList(std::vector<string> &) const {}
};

#endif
