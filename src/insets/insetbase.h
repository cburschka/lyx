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
class DispatchResult;
class FuncRequest;
class LaTeXFeatures;
class LCursor;
class MathInset;
class MetricsInfo;
class Dimension;
class PainterInfo;
class UpdatableInset;

/// Common base class to all insets
class InsetBase {
public:
	///
	typedef ptrdiff_t  difference_type;
	/// short of anything else reasonable
	typedef size_t     size_type;
	/// type for cell indices
	typedef size_t     idx_type;
	/// type for cursor positions
	typedef ptrdiff_t  pos_type;
	/// type for row numbers
	typedef size_t     row_type;
	/// type for column numbers
	typedef size_t     col_type;

	/// virtual base class destructor
	virtual ~InsetBase() {}
	/// replicate ourselves
	virtual std::auto_ptr<InsetBase> clone() const = 0;

	/// identification as math inset
	virtual MathInset * asMathInset() { return 0; }
	/// identification as non-math inset
	virtual UpdatableInset * asUpdatableInset() { return 0; }

	// the real dispatcher
	DispatchResult dispatch(LCursor & cur, FuncRequest const & cmd);

	/// cursor enters
	virtual void edit(LCursor & cur, bool left);
	/// cursor enters
	virtual void edit(LCursor & cur, int x, int y);

	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;
	/// last drawn position for 'important' insets
	virtual int x() const { return 0; }
	/// last drawn position for 'important' insets
	virtual int y() const { return 0; }
 
 	/// number of embedded cells
 	virtual size_t nargs() const { return 0; }
 	/// number of rows in gridlike structures
 	virtual size_t nrows() const { return 0; }
 	/// number of columns in gridlike structures
 	virtual size_t ncols() const { return 0; }

	/// request "external features"
	virtual void validate(LaTeXFeatures &) const {}
	/// Appends \c list with all labels found within this inset.
	virtual void getLabelList(Buffer const &,
				  std::vector<std::string> & /* list */) const {}
	/// describe content if cursor inside
	virtual void infoize(std::ostream &) const {}
	/// describe content if cursor behind
	virtual void infoize2(std::ostream &) const {}
protected:
	// the real dispatcher
	virtual
	DispatchResult priv_dispatch(LCursor & cur, FuncRequest const & cmd);
public:
	/** This is not quite the correct place for this enum. I think
	    the correct would be to let each subclass of Inset declare
	    its own enum code. Actually the notion of an InsetOld::Code
	    should be avoided, but I am not sure how this could be done
	    in a cleaner way. */
	enum Code {
		///
		NO_CODE, // 0
		///
		TOC_CODE,  // do these insets really need a code? (ale)
		///
		QUOTE_CODE,
		///
		MARK_CODE,
		///
		REF_CODE,
		///
		URL_CODE, // 5
		///
		HTMLURL_CODE,
		///
		SEPARATOR_CODE,
		///
		ENDING_CODE,
		///
		LABEL_CODE,
		///
		NOTE_CODE, // 10
		///
		ACCENT_CODE,
		///
		MATH_CODE,
		///
		INDEX_CODE,
		///
		INCLUDE_CODE,
		///
		GRAPHICS_CODE, // 15
		///
		BIBITEM_CODE,
		///
		BIBTEX_CODE,
		///
		TEXT_CODE,
		///
		ERT_CODE,
		///
		FOOT_CODE, // 20
		///
		MARGIN_CODE,
		///
		FLOAT_CODE,
		///
		WRAP_CODE,
		///
		SPACE_CODE, // 25
		///
		SPECIALCHAR_CODE,
		///
		TABULAR_CODE,
		///
		EXTERNAL_CODE,
#if 0
		///
		THEOREM_CODE,
#endif
		///
		CAPTION_CODE,
		///
		MATHMACRO_CODE, // 30
		///
		ERROR_CODE,
		///
		CITE_CODE,
		///
		FLOAT_LIST_CODE,
		///
		INDEX_PRINT_CODE,
		///
		OPTARG_CODE, // 35
		///
		ENVIRONMENT_CODE,
		///
		HFILL_CODE,
		///
		NEWLINE_CODE,
		///
		LINE_CODE,
		///
		BRANCH_CODE, // 40
		///
		BOX_CODE,
		///
		CHARSTYLE_CODE,
		///
		VSPACE_CODE,
		///
		MATHGRID_CODE,
		///
		MATHHULL_CODE
	};
	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual InsetBase::Code lyxCode() const { return NO_CODE; }

};

#endif
