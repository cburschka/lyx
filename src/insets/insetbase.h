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
class CursorSlice;
class DispatchResult;
class FuncRequest;
class FuncStatus;
class LaTeXFeatures;
class LCursor;
class LyXLex;
class LyXText;
class MathInset;
class MetricsInfo;
class Dimension;
class PainterInfo;
class OutputParams;
class UpdatableInset;

namespace lyx { namespace graphics { class PreviewLoader; } }



/// Common base class to all insets

// Do not add _any_ (non-static) data members as this would inflate
// everything storing large quantities of insets. Mathed e.g. would
// suffer.

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
	/// true for 'math' math inset, but not for e.g. mbox
	virtual bool inMathed() const { return false; }

	// the real dispatcher
	void dispatch(LCursor & cur, FuncRequest & cmd);
	/// do we want to handle this event?
	virtual bool getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;

	/// cursor enters
	virtual void edit(LCursor & cur, bool left);
	/// cursor enters
	virtual InsetBase * editXY(LCursor & cur, int x, int y);

	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;
	/// draw inset selection if necessary
	virtual void drawSelection(PainterInfo &, int, int) const {}
	///
	virtual bool editing(BufferView * bv) const;
	/// draw four angular markers
	void drawMarkers(PainterInfo & pi, int x, int y) const;
	/// draw two angular markers
	void drawMarkers2(PainterInfo & pi, int x, int y) const;
	/// add space for markers
	void metricsMarkers(Dimension & dim, int framesize = 1) const;
	/// add space for markers
	void metricsMarkers2(Dimension & dim, int framesize = 1) const;
	/// last drawn position for 'important' insets
	virtual int xo() const { return 0; }
	/// last drawn position for 'important' insets
	virtual int yo() const { return 0; }
	/// set x/y drawing position cache if available
	virtual void setPosCache(PainterInfo const &, int, int) const {}
	/// do we cover screen position x/y?
	virtual bool covers(int x, int y) const;
	/// get the screen positions of the cursor (see note in cursor.C)
	virtual void getCursorPos(CursorSlice const & cur, int & x, int & y) const;

	/// is this an inset that can be moved into?
	virtual bool isActive() const { return nargs() > 0; }
	/// Where should we go when we press the up or down cursor key?
	virtual bool idxUpDown(LCursor & cur, bool up) const;
	/// Where should we go when we press the up or down cursor key?
	virtual bool idxUpDown2(LCursor & cur, bool up) const;
	/// Move one cell to the left
	virtual bool idxLeft(LCursor &) const { return false; }
	/// Move one cell to the right
	virtual bool idxRight(LCursor &) const { return false; }

	/// Move one physical cell up
	virtual bool idxNext(LCursor &) const { return false; }
	/// Move one physical cell down
	virtual bool idxPrev(LCursor &) const { return false; }

	/// Target pos when we enter the inset from the left by pressing "Right"
	virtual bool idxFirst(LCursor &) const { return false; }
	/// Target pos when we enter the inset from the right by pressing "Left"
	virtual bool idxLast(LCursor &) const { return false; }

	/// Delete a cell and move cursor
	virtual bool idxDelete(idx_type &) { return false; }
	/// pulls cell after pressing erase
	virtual void idxGlue(idx_type) {}
	// returns list of cell indices that are "between" from and to for
	// selection purposes
	virtual bool idxBetween(idx_type idx, idx_type from, idx_type to) const;

	/// to which column belongs a cell with a given index?
	virtual col_type col(idx_type) const { return 0; }
	/// to which row belongs a cell with a given index?
	virtual row_type row(idx_type) const { return 0; }
	/// cell idex corresponding to row and column;
	virtual idx_type index(row_type row, col_type col) const;
	/// any additional x-offset when drawing a cell?
	virtual int cellXOffset(idx_type) const { return 0; }
	/// any additional y-offset when drawing a cell?
	virtual int cellYOffset(idx_type) const { return 0; }
	/// number of embedded cells
	virtual size_t nargs() const { return 0; }
	/// number of rows in gridlike structures
	virtual size_t nrows() const { return 0; }
	/// number of columns in gridlike structures
	virtual size_t ncols() const { return 0; }
	/// is called when the cursor leaves this inset
	virtual void notifyCursorLeaves(LCursor &) {}

	/// request "external features"
	virtual void validate(LaTeXFeatures &) const {}
	/// Appends \c list with all labels found within this inset.
	virtual void getLabelList(Buffer const &,
				  std::vector<std::string> & /* list */) const {}

	/// describe content if cursor inside
	virtual void infoize(std::ostream &) const {}
	/// describe content if cursor behind
	virtual void infoize2(std::ostream &) const {}

	/// plain ascii output
	virtual int plaintext(Buffer const &, std::ostream & os,
		OutputParams const &) const;
	/// linuxdoc output
	virtual int linuxdoc(Buffer const &, std::ostream & os,
		OutputParams const &) const;
	/// docbook output
	virtual int docbook(Buffer const &, std::ostream & os,
		OutputParams const &) const;

	///
	enum EDITABLE {
		///
		NOT_EDITABLE = 0,
		///
		IS_EDITABLE,
		///
		HIGHLY_EDITABLE
	};
	/// what appears in the minibuffer when opening
	virtual std::string const editMessage() const;
	///
	virtual EDITABLE editable() const;
	/// can we go further down on mouse click?
	virtual bool descendable() const { return false; }
	///
	virtual bool isTextInset() const { return false; }
	/// return true if the inset should be removed automatically
	virtual bool autoDelete() const;

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

	/** returns the Code corresponding to the \c name.
	 *  Eg, translate("branch") == BRANCH_CODE
	 */
	static Code translate(std::string const & name);

	/// returns true the inset can hold an inset of given type
	virtual bool insetAllowed(Code) const { return false; }
	// if this inset has paragraphs should they be output all as default
	// paragraphs with "Standard" layout?
	virtual bool forceDefaultParagraphs(InsetBase const *) const { return false; }
	///
	virtual std::string const & getInsetName() const;
	/// used to toggle insets
	// is the inset open?
	virtual bool isOpen() const { return false; }
	/// open the inset
	virtual void open() {}
	/// close the inset
	virtual void close() {}
	// should this inset be handled like a normal charater
	virtual bool isChar() const { return false; }
	// is this equivalent to a letter?
	virtual bool isLetter() const { return false; }
	// is this equivalent to a space (which is BTW different from
	// a line separator)?
	virtual bool isSpace() const { return false; }
	// should we have a non-filled line before this inset?
	virtual bool display() const { return false; }
	// should we break lines after this inset?
	virtual bool isLineSeparator() const { return false; }
	/// dumps content to lyxerr
	virtual void dump() const;
	///
	virtual void write(Buffer const &, std::ostream &) const {}
	///
	virtual void read(Buffer const &, LyXLex &) {}
	/// returns the number of rows (\n's) of generated tex code.
	virtual int latex(Buffer const &, std::ostream &,
			  OutputParams const &) const { return 0; }
	/// returns true to override begin and end inset in file
	virtual bool directWrite() const;
	///
	virtual bool allowSpellCheck() const { return false; }

	/// if this insets owns text cells (e.g. InsetText) return cell num
	virtual LyXText * getText(int /*num*/) const { return 0; }

	/** Adds a LaTeX snippet to the Preview Loader for transformation
	 *  into a bitmap image. Does not start the laoding process.
	 *
	 *  Most insets have no interest in this capability, so the method
	 *  defaults to empty.
	 */
	virtual void addPreview(lyx::graphics::PreviewLoader &) const {}
protected:
	// the real dispatcher
	virtual void priv_dispatch(LCursor & cur, FuncRequest & cmd);
public:
	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual Code lyxCode() const { return NO_CODE; }

	/// -1: text mode, 1: math mode, 0 undecided
	enum mode_type {UNDECIDED_MODE, TEXT_MODE, MATH_MODE};
	/// return text or mathmode if that is possible to determine
	virtual mode_type currentMode() const { return UNDECIDED_MODE; }

	/// is this inset allowed within a font change?
	virtual bool noFontChange() const { return false; }

	///
	virtual void markErased();
	/// pretty arbitrary
	virtual int width() const { return 10; }
	/// pretty arbitrary
	virtual int ascent() const { return 10; }
	/// pretty arbitrary
	virtual int descent() const { return 10; }
};


/**
 * returns true if pointer argument is valid
 * and points to an editable inset
 */
bool isEditableInset(InsetBase const * inset);


/**
 * returns true if pointer argument is valid
 * and points to a highly editable inset
 */
bool isHighlyEditableInset(InsetBase const * inset);

#endif
