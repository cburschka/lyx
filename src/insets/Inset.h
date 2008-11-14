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

#include "ColorCode.h"
#include "InsetCode.h"
#include "Layout.h"

#include "support/strfwd.h"
#include "support/types.h"


namespace lyx {

class BiblioInfo;
class Buffer;
class BufferParams;
class BufferView;
class Change;
class CompletionList;
class Cursor;
class CursorSlice;
class Dimension;
class DocIterator;
class FuncRequest;
class FuncStatus;
class InsetCollapsable;
class InsetIterator;
class InsetLayout;
class InsetList;
class InsetMath;
class InsetTabular;
class InsetText;
class LaTeXFeatures;
class Lexer;
class MathAtom;
class MetricsInfo;
class OutputParams;
class PainterInfo;
class ParConstIterator;
class ParIterator;
class Text;
class TocList;

namespace graphics { class PreviewLoader; }


/** returns the InsetCode corresponding to the \c name.
*   Eg, insetCode("branch") == BRANCH_CODE
*   Implemented in 'Inset.cpp'.
*/
InsetCode insetCode(std::string const & name);
/// the other way
std::string insetName(InsetCode);

/// Common base class to all insets

// Do not add _any_ (non-static) data members as this would inflate
// everything storing large quantities of insets. Mathed e.g. would
// suffer.

class Inset {
public:
	///
	enum EntryDirection {
		ENTRY_DIRECTION_IGNORE,
		ENTRY_DIRECTION_RIGHT,
		ENTRY_DIRECTION_LEFT,
	};
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

	/// change associated Buffer
	/// FIXME this should go.
	virtual void setBuffer(Buffer & buffer);
	/// retrieve associated Buffer
	virtual Buffer & buffer();
	virtual Buffer const & buffer() const;
	/// This checks whether the Buffer * actually points to an open 
	/// Buffer. It might not if that Buffer has been closed.
	bool isBufferValid() const;

	/// initialize view for this inset.
	/**
	  * This is typically used after this inset is created interactively.
	  * Intented purpose is to sanitize internal state with regard to current
	  * Buffer. The default implementation calls updateLabels(buffer()) is
	  * the inset is labeled.
	  *
	  * \sa isLabeled()
	  **/
	virtual void initView();
	/// \return true if this inset is labeled.
	virtual bool isLabeled() const { return false; }

	/// identification as math inset
	virtual InsetMath * asInsetMath() { return 0; }
	/// true for 'math' math inset, but not for e.g. mbox
	virtual bool inMathed() const { return false; }
	/// is this inset based on the InsetText class?
	virtual InsetText * asInsetText() { return 0; }
	/// is this inset based on the InsetText class?
	virtual InsetText const * asInsetText() const { return 0; }
	/// is this inset based on the InsetCollapsable class?
	virtual InsetCollapsable * asInsetCollapsable() { return 0; }
	/// is this inset based on the InsetCollapsable class?
	virtual InsetCollapsable const * asInsetCollapsable() const { return 0; }
	/// is this inset based on the InsetTabular class?
	virtual InsetTabular * asInsetTabular() { return 0; }
	/// is this inset based on the InsetTabular class?
	virtual InsetTabular const * asInsetTabular() const { return 0; }

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
	virtual void edit(Cursor & cur, bool front, 
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	/// cursor enters
	virtual Inset * editXY(Cursor & cur, int x, int y);

	/// compute the size of the object returned in dim
	/// \retval true if metrics changed.
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;
	/// draw inset selection if necessary
	virtual void drawSelection(PainterInfo &, int, int) const {}
	/// draw inset background if the inset has an own background and a
	/// selection is drawn by drawSelection.
	virtual void drawBackground(PainterInfo &, int, int) const {}
	///
	virtual bool editing(BufferView const * bv) const;
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
	///
	void setDimCache(MetricsInfo const &, Dimension const &) const;
	/// do we cover screen position x/y?
	virtual bool covers(BufferView const & bv, int x, int y) const;
	/// get the screen positions of the cursor (see note in Cursor.cpp)
	virtual void cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const;

	///
	virtual bool isFreeSpacing() const { return false; }
	///
	virtual bool allowEmpty() const { return false; }
	/// Force inset into LTR environment if surroundings are RTL?
	virtual bool forceLTR() const { return false; }

	/// is this an inset that can be moved into?
	/// FIXME: merge with editable()
	virtual bool isActive() const { return nargs() > 0; }
	/// Where should we go when we press the up or down cursor key?
	virtual bool idxUpDown(Cursor & cur, bool up) const;
	/// Move one cell backwards
	virtual bool idxBackward(Cursor &) const { return false; }
	/// Move one cell forward
	virtual bool idxForward(Cursor &) const { return false; }

	/// Move to the next cell
	virtual bool idxNext(Cursor &) const { return false; }
	/// Move to the previous cell
	virtual bool idxPrev(Cursor &) const { return false; }

	/// Target pos when we enter the inset while moving forward
	virtual bool idxFirst(Cursor &) const { return false; }
	/// Target pos when we enter the inset while moving backwards
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
	/// cell index corresponding to row and column;
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
	/// Is called when the cursor leaves this inset.
	/// Returns true if cursor is now invalid, e.g. if former 
	/// insets in higher cursor slices of \c old do not exist 
	/// anymore.
	/// \c old is the old cursor, i.e. there is a slice pointing to this.
	/// \c cur is the new cursor. Use the update flags to cause a redraw.
	virtual bool notifyCursorLeaves(Cursor const & /*old*/, Cursor & /*cur*/)
		{ return false; }
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

	/// describe content if cursor inside
	virtual void infoize(odocstream &) const {}
	/// describe content if cursor behind
	virtual void infoize2(odocstream &) const {}

	enum { PLAINTEXT_NEWLINE = 10000 };

	/// plain text output in ucs4 encoding
	/// return the number of characters; in case of multiple lines of
	/// output, add PLAINTEXT_NEWLINE to the number of chars in the last line
	virtual int plaintext(odocstream &, OutputParams const &) const = 0;
	/// docbook output
	virtual int docbook(odocstream & os, OutputParams const &) const;
	/// the string that is passed to the TOC
	virtual void tocString(odocstream &) const {}

	/** This enum indicates by which means the inset can be modified:
	- NOT_EDITABLE: the inset's content cannot be modified at all
	  (e.g. printindex, insetspecialchar)
	- IS_EDITABLE: content can be edited via dialog (e.g. bibtex, index, href)
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
	virtual docstring editMessage() const;
	///
	virtual EDITABLE editable() const;
	/// can we go further down on mouse click?
	virtual bool descendable() const { return false; }
	/// does this contain text that can be change track marked in DVI?
	virtual bool canTrackChanges() const { return false; }
	/// return true if the inset should be removed automatically
	virtual bool autoDelete() const;

	/// Returns true if the inset supports completions.
	virtual bool completionSupported(Cursor const &) const { return false; }
	/// Returns true if the inset supports inline completions at the
	/// cursor position. In this case the completion might be stored
	/// in the BufferView's inlineCompletion property.
	virtual bool inlineCompletionSupported(Cursor const & /*cur*/) const 
		{ return false; }
	/// Return true if the inline completion should be automatic.
	virtual bool automaticInlineCompletion() const { return true; }
	/// Return true if the popup completion should be automatic.
	virtual bool automaticPopupCompletion() const { return true; }
	/// Return true if the cursor should indicate a completion.
	virtual bool showCompletionCursor() const { return true; }
	/// Returns completion suggestions at cursor position. Return an
	/// null pointer if no completion is a available or possible.
	/// The caller is responsible to free the returned object!
	virtual CompletionList const * createCompletionList(Cursor const &) const 
		{ return 0; }
	/// Returns the completion prefix to filter the suggestions for completion.
	/// This is only called if completionList returned a non-null list.
	virtual docstring completionPrefix(Cursor const &) const;
	/// Do a completion at the cursor position. Return true on success.
	/// The completion does not contain the prefix. If finished is true, the
	/// completion is final. If finished is false, completion might only be
	/// a partial completion.
	virtual bool insertCompletion(Cursor & /*cur*/, 
		docstring const & /*completion*/, bool /*finished*/) 
		{ return false; }
	/// Get the completion inset position and size
	virtual void completionPosAndDim(Cursor const &, int & /*x*/, int & /*y*/, 
		Dimension & /*dim*/) const {}

	/// returns true if the inset can hold an inset of given type
	virtual bool insetAllowed(InsetCode) const { return false; }
	/// should this inset use the empty layout by default rather than 
	/// the standard layout? (default: only if that is forced.)
	virtual bool usePlainLayout() const { return forcePlainLayout(); }
	/// if this inset has paragraphs should they be forced to use the
	/// empty layout?
	virtual bool forcePlainLayout(idx_type = 0) const { return false; }
	/// if this inset has paragraphs should the user be allowed to
	/// customize alignment, etc?
	virtual bool allowParagraphCustomization(idx_type = 0) const { return true; }
	/// Is the width forced to some value?
	virtual bool hasFixedWidth() const { return false; }

	/// Is the content of this inset part of the output document?
	virtual bool producesOutput() const { return true; }

	/// \return Tool tip for this inset.
	/// This default implementation returns an empty string.
	virtual docstring toolTip(BufferView const & bv, int x, int y) const;
	
	/// \return Context menu identifier for this inset.
	/// This default implementation returns an empty string.
	virtual docstring contextMenu(BufferView const & bv, int x, int y) const;

	// FIXME This should really disappear in favor of 
	//	docstring name() const { return from_ascii(insetName(lyxCode()))); }
	// There's no reason to be using different names in different places.
	// But to do this we would need to change the file format, since the names
	// used there don't correspond to what is used here. 
	///
	virtual docstring name() const;
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
	/// is this an expandible space (rubber length)?
	virtual bool isStretchableSpace() const { return false; }

	enum DisplayType {
		Inline = 0,
		AlignLeft,
		AlignCenter,
		AlignRight
	};

	/// should we have a non-filled line before this inset?
	virtual DisplayType display() const { return Inline; }
	///
	virtual LyXAlignment contentAlignment() const { return LYX_ALIGN_NONE; }
	/// should we break lines after this inset?
	virtual bool isLineSeparator() const { return false; }
	/// should paragraph indendation be ommitted in any case?
	virtual bool neverIndent() const { return false; }
	/// dumps content to lyxerr
	virtual void dump() const;
	/// write inset in .lyx format
	virtual void write(std::ostream &) const {}
	/// read inset in .lyx format
	virtual void read(Lexer &) {}
	/** Export the inset to LaTeX.
	 *  Don't use a temporary stringstream if the final output is
	 *  supposed to go to a file.
	 *  \sa Buffer::writeLaTeXSource for the reason.
	 *  \return the number of rows (\n's) of generated LaTeX code.
	 */
	virtual int latex(odocstream &, OutputParams const &) const { return 0; }
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

	/** Classifies the unicode characters appearing in a math inset
	 *  depending on whether they are to be translated as latex
	 *  math/text commands or used as math symbols without translation.
	 *
	 *  Only math insets have interest in this classification, so the
	 *  method defaults to empty.
	 */
	virtual void initUnicodeMath() const {}

	/// Add an entry to the TocList
	/// pit is the ParConstIterator of the paragraph containing the inset
	virtual void addToToc(DocIterator const &) {}
	/// Fill keys with BibTeX information
	virtual void fillWithBibKeys(BiblioInfo &, InsetIterator const &) const {}
	/// Update the counters of this inset and of its contents
	virtual void updateLabels(ParIterator const &) {}

	/// Updates the inset's dialog
	virtual Buffer const * updateFrontend() const;

public:
	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual InsetCode lyxCode() const { return NO_CODE; }

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

	///
	virtual Dimension const dimension(BufferView const &) const;
	///
	int scroll() const { return 0; }
	///
	virtual ColorCode backgroundColor() const;
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
	/// Constructors
	Inset() : buffer_(0) {}
	Inset(Inset const &) : buffer_(0) {}

	/// replicate ourselves
	friend class InsetList;
	friend class MathAtom;
	virtual Inset * clone() const = 0;

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

	Buffer * buffer_;
};

} // namespace lyx

#endif
