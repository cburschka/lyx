// -*- C++ -*-
/**
 * \file insetbase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author none
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETBASE_H
#define INSETBASE_H

#include <string>
#include <vector>
#include <memory>

class Buffer;
class BufferView;
class FuncRequest;
class MetricsInfo;
class Dimension;
class PainterInfo;
class LaTeXFeatures;
class DispatchResult;

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
	DispatchResult
	dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos);
	// the real dispatcher
	DispatchResult
	dispatch(FuncRequest const & cmd);

	/// cursor enters
	virtual void edit(BufferView * bv, bool left);
	/// cursor enters
	virtual void edit(BufferView * bv, int x, int y);

	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;

	/// request "external features"
	virtual void validate(LaTeXFeatures &) const {}
	/// Appends \c list with all labels found within this inset.
	virtual void getLabelList(Buffer const &,
				  std::vector<std::string> & /* list */) const {}
protected:
	// the real dispatcher
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos);
};

#endif
