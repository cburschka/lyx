// -*- C++ -*-
/**
 * \file Inset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETBASE_H
#define INSETBASE_H

#include "Biblio.h"
#include "Changes.h"
#include "Dimension.h"

#include "support/docstream.h"

#include <memory>
#include <vector>

namespace lyx {

class Buffer;
class BufferParams;
class BufferView;
class ParIterator;
class ParConstIterator;
class CursorSlice;
class InsetIterator;
class FuncRequest;
class FuncStatus;
class InsetLayout;
class InsetMath;
class InsetText;
class LaTeXFeatures;
class Color_color;
class Cursor;
class Lexer;
class Text;
class MetricsInfo;
class Dimension;
class PainterInfo;
class OutputParams;
class TocList;


namespace graphics { class PreviewLoader; }


/// Common base class to all insets

// Do not add _any_ (non-static) data members as this would inflate
// everything storing large quantities of insets. Mathed e.g. would
// suffer.

class Inset {
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
	virtual ~Inset() {}
	/// replicate ourselves
	std::auto_ptr<Inset> clone() const;

	/// identification as math inset
	virtual InsetMath * asInsetMath() { return 0; }
	/// true for 'math' math inset, but not for e.g. mbox
	virtual bool inMathed() const { return false; }
	/// is this inset based on the TextInset class?
	virtual InsetText * asTextInset() { return 0; }
	/// is this inset based on the TextInset class?
	virtual InsetText const * asTextInset() const { return 0; }
	
	/// the real dispatcher
	void dispatch(Cursor & cur, FuncRequest & cmd);
	/**
	 * \returns true if this function made a definitive decision on
	 * whether the inset wants to handle the request \p cmd or not.
	 * The result of this decision is put into \p status.
	 *
	 * Every request that is enabled in this method needs to be handled
	 * in doDispatch(). Normally we have a 1:1 relationship between the
	 * requests handled in getStatus() and doDispatch(), but there are
	 * some exceptions:
	 * - A request that is disabled in getStatus() does not need to
	 *   appear in doDispatch(). It is guaranteed that doDispatch()
	 *   is never called with this request.
	 * - A few requests are en- or disabled in Inset::getStatus().
	 *   These need to be handled in the doDispatch() methods of the
	 *   derived insets, since Inset::doDispatch() has not enough
	 *   information to handle them.
	 * - LFUN_MOUSE_* need not to be handled in getStatus(), because these
	 *   are dispatched directly
	 */
	virtual bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;

	/// cursor enters
	virtual void edit(Cursor & cur, bool left);
	/// cursor enters
	virtual Inset * editXY(Cursor & cur, int x, int y);

	/// compute the size of the object returned in dim
	/// \retval true if metrics changed.
	virtual bool metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;
	/// draw inset selection if necessary
	virtual void drawSelection(PainterInfo &, int, int) const {}
	///
	virtual bool editing(BufferView * bv) const;
	///
	virtual bool showInsetDialog(BufferView *) const { return false; }

	/// draw inset decoration if necessary.
	/// This can use \c drawMarkers() for example.
	virtual void drawDecoration(PainterInfo &, int, int) const {}
	/// draw four angular markers
	void drawMarkers(PainterInfo & pi, int x, int y) const;
	/// draw two angular markers
	void drawMarkers2(PainterInfo & pi, int x, int y) const;
	/// add space for markers
	void metricsMarkers(Dimension & dim, int framesize = 1) const;
	/// add space for markers
	void metricsMarkers2(Dimension & dim, int framesize = 1) const;
	/// last drawn position for 'important' insets
	int xo(BufferView const & bv) const;
	/// last drawn position for 'important' insets
	int yo(BufferView const & bv) const;
	/// set x/y drawing position cache if available
	virtual void setPosCache(PainterInfo const &, int, int) const;
	/// do we cover screen position x/y?
	virtual bool covers(BufferView const & bv, int x, int y) const;
	/// get the screen positions of the cursor (see note in Cursor.cpp)
	virtual void cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const;

	/// is this an inset that can be moved into?
	/// FIXME: merge with editable()
	virtual bool isActive() const { return nargs() > 0; }
	/// Where should we go when we press the up or down cursor key?
	virtual bool idxUpDown(Cursor & cur, bool up) const;
	/// Move one cell to the left
	virtual bool idxLeft(Cursor &) const { return false; }
	/// Move one cell to the right
	virtual bool idxRight(Cursor &) const { return false; }

	/// Move one physical cell up
	virtual bool idxNext(Cursor &) const { return false; }
	/// Move one physical cell down
	virtual bool idxPrev(Cursor &) const { return false; }

	/// Target pos when we enter the inset from the left by pressing "Right"
	virtual bool idxFirst(Cursor &) const { return false; }
	/// Target pos when we enter the inset from the right by pressing "Left"
	virtual bool idxLast(Cursor &) const { return false; }

	/// Delete a cell and move cursor
	virtual bool idxDelete(idx_type &) { return false; }
	/// pulls cell after pressing erase
	virtual void idxGlue(idx_type) {}
	/// returns list of cell indices that are "between" from and to for
	/// selection purposes
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
	//  returns true if cursor is now invalid.
	virtual bool notifyCursorLeaves(Cursor &) { return false; }
	/// is called when the mouse enter or leave this inset
	/// return true if this inset needs repaint
	virtual bool setMouseHover(bool) { return false; }
	/// return true if this inset is hovered (under mouse)
	/// This is by now only used by mathed to draw corners 
	/// (Inset::drawMarkers() and Inset::drawMarkers2()).
	/// Other insets do not have to redefine this function to 
	/// return the correct status of mouseHovered.
	virtual bool mouseHovered() const { return false; }

	/// request "external features"
	virtual void validate(LaTeXFeatures &) const {}
	/// Appends \c list with all labels found within this inset.
	virtual void getLabelList(Buffer const &,
				  std::vector<docstring> & /* list */) const {}

	/// describe content if cursor inside
	virtual void infoize(odocstream &) const {}
	/// describe content if cursor behind
	virtual void infoize2(odocstream &) const {}

	enum {
		PLAINTEXT_NEWLINE = 10000
	};

	/// plain text output in ucs4 encoding
	/// return the number of characters; in case of multiple lines of
	/// output, add PLAINTEXT_NEWLINE to the number of chars in the last line
	virtual int plaintext(Buffer const &, odocstream &,
			      OutputParams const &) const = 0;
	/// docbook output
	virtual int docbook(Buffer const &, odocstream & os,
			    OutputParams const &) const;
	/// the string that is passed to the TOC
	virtual void textString(Buffer const &, odocstream &) const {}

	/** This enum indicates by which means the inset can be modified:
	- NOT_EDITABLE: the inset's content cannot be modified at all
	  (e.g. printindex, insetspecialchar)
	- IS_EDITABLE: content can be edited via dialog (e.g. bibtex, index, url)
	- HIGHLY_EDITABLE: content can be edited on screen (normally means that
	  insettext is contained, e.g. collapsables, tabular) */
	// FIXME: This has not yet been fully implemented to math insets
	enum EDITABLE {
		///
		NOT_EDITABLE = 0,
		///
		IS_EDITABLE,
		///
		HIGHLY_EDITABLE
	};
	/// what appears in the minibuffer when opening
	virtual docstring const editMessage() const;
	///
	virtual EDITABLE editable() const;
	/// can we go further down on mouse click?
	virtual bool descendable() const { return false; }
	/// does this contain text that can be change track marked in DVI?
	virtual bool canTrackChanges() const { return false; }
	/// return true if the inset should be removed automatically
	virtual bool autoDelete() const;

	/** This is not quite the correct place for this enum. I think
	    the correct would be to let each subclass of Inset declare
	    its own enum code. Actually the notion of an Inset::Code
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
		MATHMACROARG_CODE,
		///
		NOMENCL_CODE, // 45
		///
		NOMENCL_PRINT_CODE,
		///
		PAGEBREAK_CODE,
		///
		LISTINGS_CODE
	};

	/** returns the Code corresponding to the \c name.
	 *  Eg, translate("branch") == BRANCH_CODE
	 */
	static Code translate(std::string const & name);

	/// returns true if the inset can hold an inset of given type
	virtual bool insetAllowed(Code) const { return false; }
	/// if this inset has paragraphs should they be output all as default
	/// paragraphs with the default layout of the text class?
	virtual bool forceDefaultParagraphs(idx_type) const { return false; }
	/// Is the width forced to some value?
	virtual bool hasFixedWidth() const { return false; }

	///
	virtual docstring name() const { return from_ascii("unknown"); }
	///
	virtual InsetLayout const & getLayout(BufferParams const & bp) const;
	/// used to toggle insets
	/// is the inset open?
	/// should this inset be handled like a normal charater
	virtual bool isChar() const { return false; }
	/// is this equivalent to a letter?
	virtual bool isLetter() const { return false; }
	/// is this equivalent to a space (which is BTW different from
	/// a line separator)?
	virtual bool isSpace() const { return false; }

	enum DisplayType {
		Inline = 0,
		AlignLeft,
		AlignCenter,
		AlignRight
	};

	/// should we have a non-filled line before this inset?
	virtual DisplayType display() const { return Inline; }
	/// should we break lines after this inset?
	virtual bool isLineSeparator() const { return false; }
	/// should paragraph indendation be ommitted in any case?
	virtual bool neverIndent(Buffer const &) const { return false; }
	/// dumps content to lyxerr
	virtual void dump() const;
	/// write inset in .lyx format
	virtual void write(Buffer const &, std::ostream &) const {}
	/// read inset in .lyx format
	virtual void read(Buffer const &, Lexer &) {}
	/** Export the inset to LaTeX.
	 *  Don't use a temporary stringstream if the final output is
	 *  supposed to go to a file.
	 *  \sa Buffer::writeLaTeXSource for the reason.
	 *  \return the number of rows (\n's) of generated LaTeX code.
	 */
	virtual int latex(Buffer const &, odocstream &,
			  OutputParams const &) const { return 0; }
	/// returns true to override begin and end inset in file
	virtual bool directWrite() const;
	///
	virtual bool allowSpellCheck() const { return false; }

	/// if this insets owns text cells (e.g. InsetText) return cell num
	virtual Text * getText(int /*num*/) const { return 0; }

	/** Adds a LaTeX snippet to the Preview Loader for transformation
	 *  into a bitmap image. Does not start the laoding process.
	 *
	 *  Most insets have no interest in this capability, so the method
	 *  defaults to empty.
	 */
	virtual void addPreview(graphics::PreviewLoader &) const {}
	/// Add an entry to the TocList
	/// pit is the ParConstIterator of the paragraph containing the inset
	virtual void addToToc(TocList &, Buffer const &, ParConstIterator const &) const {}
	/// Fill keys with BibTeX information
	virtual void fillWithBibKeys(Buffer const &,
		BiblioInfo &, InsetIterator const &) const { return; }
	/// Update the counters of this inset and of its contents
	virtual void updateLabels(Buffer const &, ParIterator const &) {}


public:
	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual Code lyxCode() const { return NO_CODE; }

	/// -1: text mode, 1: math mode, 0 undecided
	enum mode_type {UNDECIDED_MODE, TEXT_MODE, MATH_MODE};
	/// return text or mathmode if that is possible to determine
	virtual mode_type currentMode() const { return UNDECIDED_MODE; }
	/// returns whether this inset is allowed in other insets of given mode
	virtual bool allowedIn(mode_type) const { return true; }
	/**
	 * Is this inset allowed within a font change?
	 *
	 * FIXME: noFontChange means currently that the font change is closed
	 * in LaTeX before the inset, and that the contents of the inset
	 * will be in default font. This should be changed so that the inset
	 * changes the font again.
	 */
	virtual bool noFontChange() const { return false; }

	/// set the change for the entire inset
	virtual void setChange(Change const &) {}
	/// accept the changes within the inset
	virtual void acceptChanges(BufferParams const &) {};
	/// reject the changes within the inset
	virtual void rejectChanges(BufferParams const &) {};

	/// inset width.
	int width() const { return dim_.wid; }
	/// inset ascent.
	int ascent() const { return dim_.asc; }
	/// inset descent.
	int descent() const { return dim_.des; }
	///
	int scroll() const { return 0; }
	///
	virtual Color_color backgroundColor() const;
	///
	enum CollapseStatus {
		Collapsed,
		Open
	};
	///
	virtual void setStatus(Cursor &, CollapseStatus) {}
	//
	enum { TEXT_TO_INSET_OFFSET = 4 };

protected:
	Inset();

	/** The real dispatcher.
	 *  Gets normally called from Cursor::dispatch(). Cursor::dispatch()
	 *  assumes the common case of 'LFUN handled, need update'.
	 *  This has to be overriden by calling Cursor::undispatched() or
	 *  Cursor::noUpdate() if appropriate.
	 *  If you need to call the dispatch method of some inset directly
	 *  you may have to explicitly request an update at that place. Don't
	 *  do it in doDispatch(), since that causes nested updates when
	 *  called from Cursor::dispatch(), and these can lead to crashes.
	 *  \sa getStatus
	 */
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);

	/// Cached dimensions of the inset.
	mutable Dimension dim_;
private:
	virtual std::auto_ptr<Inset> doClone() const = 0;
};


/**
 * returns true if pointer argument is valid
 * and points to an editable inset
 */
bool isEditableInset(Inset const * inset);


/**
 * returns true if pointer argument is valid
 * and points to a highly editable inset
 */
bool isHighlyEditableInset(Inset const * inset);

/** \c Inset_code is a wrapper for Inset::Code.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose Inset.h.
 */
class Inset_code {
	Inset::Code val_;
public:
	Inset_code(Inset::Code val) : val_(val) {}
	operator Inset::Code() const { return val_; }
};



} // namespace lyx

#endif
