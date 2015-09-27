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
#include "InsetLayout.h"
#include "LayoutEnums.h"
#include "OutputEnums.h"

#include "support/strfwd.h"
#include "support/types.h"

#include <climits>


namespace lyx {

class BiblioInfo;
class Buffer;
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
class InsetCommand;
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
class XHTMLStream;
class otexstream;

namespace graphics { class PreviewLoader; }


/// returns the InsetCode corresponding to the \c name.
/// Eg, insetCode("branch") == BRANCH_CODE
InsetCode insetCode(std::string const & name);
/// returns the Inset name corresponding to the \c InsetCode.
/// Eg, insetName(BRANCH_CODE) == "branch"
std::string insetName(InsetCode);
/// returns the Inset name corresponding to the \c InsetCode.
/// Eg, insetDisplayName(BRANCH_CODE) == _("Branch")
docstring insetDisplayName(InsetCode);
///
static int const TOC_ENTRY_LENGTH = 80;

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
		ENTRY_DIRECTION_LEFT
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
	virtual void setBuffer(Buffer & buffer);
	/// retrieve associated Buffer
	virtual Buffer & buffer();
	virtual Buffer const & buffer() const;
	/// Returns true if buffer_ actually points to a Buffer that has
	/// been loaded into LyX and is still open. Note that this will
	/// always return false for cloned Buffers. If you want to allow
	/// for the case of cloned Buffers, use isBufferValid().
	bool isBufferLoaded() const;
	/// Returns true if this is a loaded buffer or a cloned buffer.
	bool isBufferValid() const;

	/// initialize view for this inset.
	/**
	  * This is typically used after this inset is created interactively.
	  * Intented purpose is to sanitize internal state with regard to current
	  * Buffer. 
	  **/
	virtual void initView() {}
	/// \return true if this inset is labeled.
	virtual bool isLabeled() const { return false; }

	/// identification as math inset
	virtual InsetMath * asInsetMath() { return 0; }
	/// identification as math inset
	virtual InsetMath const * asInsetMath() const { return 0; }
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
	/// is this inset based on the InsetCommand class?
	virtual InsetCommand * asInsetCommand() { return 0; }
	/// is this inset based on the InsetCommand class?
	virtual InsetCommand const * asInsetCommand() const { return 0; }

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
	virtual bool showInsetDialog(BufferView *) const;

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

	/// Allow multiple blanks
	virtual bool isFreeSpacing() const;
	/// Don't eliminate empty paragraphs
	virtual bool allowEmpty() const;
	/// Force inset into LTR environment if surroundings are RTL
	virtual bool forceLTR() const;
	/// whether to include this inset in the strings generated for the TOC
	virtual bool isInToc() const;

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
	/// \c old is the old cursor, the last slice points to this.
	/// \c cur is the new cursor. Use the update flags to cause a redraw.
	virtual bool notifyCursorLeaves(Cursor const & /*old*/, Cursor & /*cur*/)
		{ return false; }
	/// Is called when the cursor enters this inset.
	/// Returns true if cursor is now invalid, e.g. if former 
	/// insets in higher cursor slices of \c old do not exist 
	/// anymore.
	/// \c cur is the new cursor, some slice points to this. Use the update
	/// flags to cause a redraw.
	virtual bool notifyCursorEnters(Cursor & /*cur*/)
		{ return false; }
	/// is called when the mouse enters or leaves this inset
	/// return true if this inset needs a repaint
	virtual bool setMouseHover(BufferView const *, bool) const
		{ return false; }
	/// return true if this inset is hovered (under mouse)
	/// This is by now only used by mathed to draw corners 
	/// (Inset::drawMarkers() and Inset::drawMarkers2()).
	/// Other insets do not have to redefine this function to 
	/// return the correct status of mouseHovered.
	virtual bool mouseHovered(BufferView const *) const { return false; }

	/// request "external features"
	virtual void validate(LaTeXFeatures &) const {}

	/// Validate LFUN_INSET_MODIFY argument.
	virtual bool validateModifyArgument(docstring const &) const { return true; }

	/// describe content if cursor inside
	virtual void infoize(odocstream &) const {}
	/// describe content if cursor behind
	virtual void infoize2(odocstream &) const {}

	enum { PLAINTEXT_NEWLINE = 10000 };

	/// plain text output in ucs4 encoding
	/// return the number of characters; in case of multiple lines of
	/// output, add PLAINTEXT_NEWLINE to the number of chars in the last line
	virtual int plaintext(odocstringstream &, OutputParams const &,
	                      size_t max_length = INT_MAX) const = 0;
	/// docbook output
	virtual int docbook(odocstream & os, OutputParams const &) const;
	/// XHTML output
	/// the inset is expected to write XHTML to the XHTMLStream
	/// \return any "deferred" material that should be written outside the
	/// normal stream, and which will in fact be written after the current
	/// paragraph closes. this is appropriate e.g. for floats.
	virtual docstring xhtml(XHTMLStream & xs, OutputParams const &) const;

	/// Writes a string representation of the inset to the odocstream.
	/// This one should be called when you want the whole contents of
	/// the inset.
	virtual void toString(odocstream &) const {}
	/// Appends a potentially abbreviated version of the inset to
	/// \param str. Intended for use by the TOC.
	virtual void forOutliner(docstring & str,
	                    size_t maxlen = TOC_ENTRY_LENGTH) const;

	/// can the contents of the inset be edited on screen ?
	// true for InsetCollapsables (not ButtonOnly) (not InsetInfo), InsetText
	virtual bool editable() const;
	/// has the Inset settings that can be modified in a dialog ?
	virtual bool hasSettings() const;
	/// can we go further down on mouse click?
	// true for InsetCaption, InsetCollapsables (not ButtonOnly), InsetTabular
	virtual bool descendable(BufferView const &) const { return false; }
	/// is this an inset that can be moved into?
	/// FIXME: merge with editable()
	// true for InsetTabular & InsetText
	virtual bool isActive() const { return nargs() > 0; }
	/// can we click at the specified position ?
	virtual bool clickable(int, int) const { return false; }
	/// Move one cell backwards
	virtual bool allowsCaptionVariation(std::string const &) const { return false; }

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
	virtual bool allowParagraphCustomization(idx_type = 0) const
		{ return true; }
	/// Is the width forced to some value?
	virtual bool hasFixedWidth() const { return false; }
	/// if this inset has paragraphs should they be forced to use a
	/// local font language switch?
	virtual bool forceLocalFontSwitch() const { return false; }

	/// Is the content of this inset part of the output document?
	virtual bool producesOutput() const { return true; }

	/// \return Tool tip for this inset.
	/// This default implementation returns an empty string.
	virtual docstring toolTip(BufferView const & bv, int x, int y) const;
	
	/// \return Context menu identifier. This function determines
	/// whose Inset's menu should be shown for the given position.
	virtual std::string contextMenu(BufferView const & bv, int x, int y) const;

	/// \return Context menu identifier for this inset.
	/// This default implementation returns an empty string.
	virtual std::string contextMenuName() const;


	virtual docstring layoutName() const;
	///
	virtual InsetLayout const & getLayout() const;
	///
	virtual bool isPassThru() const { return getLayout().isPassThru(); }
	/// Is this inset's layout defined in the document's textclass?
	bool undefined() const;
	/// should this inset be handled like a normal character?
	/// (a character can be a letter or punctuation)
	virtual bool isChar() const { return false; }
	/// is this equivalent to a letter?
	/// (a letter is a character that is considered part of a word)
	virtual bool isLetter() const { return false; }
	/// is this equivalent to a space (which is BTW different from
	/// a line separator)?
	virtual bool isSpace() const { return false; }
	/// does this inset try to use all available space (like \\hfill does)?
	virtual bool isHfill() const { return false; }

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
	 */
	virtual void latex(otexstream &, OutputParams const &) const {}
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
	virtual void addPreview(DocIterator const &,
		graphics::PreviewLoader &) const {}

	/** Classifies the unicode characters appearing in a math inset
	 *  depending on whether they are to be translated as latex
	 *  math/text commands or used as math symbols without translation.
	 *
	 *  Only math insets have interest in this classification, so the
	 *  method defaults to empty.
	 */
	virtual void initUnicodeMath() const {}

	/// Add an entry to the TocList
	/// Pass a DocIterator that points at the paragraph containing
	/// the inset
	///
	/// \param output_active : is the inset active or is it in an inactive
	/// branch or a note?
	///
	/// \param utype : is the toc being generated for use by the output
	/// routines?
	virtual void addToToc(DocIterator const & /* di */,
						  bool /* output_active */,
						  UpdateType /* utype*/) const {}
	/// Collect BibTeX information
	virtual void collectBibKeys(InsetIterator const &) const {}
	/// Update the counters of this inset and of its contents.
	/// The boolean indicates whether we are preparing for output, e.g.,
	/// of XHTML.
	virtual void updateBuffer(ParIterator const &, UpdateType) {}

	/// Updates the inset's dialog
	virtual Buffer const * updateFrontend() const;

public:
	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual InsetCode lyxCode() const { return NO_CODE; }

	/// -1: text mode, 1: math mode, 0 undecided
	enum mode_type {UNDECIDED_MODE, TEXT_MODE, MATH_MODE};
	/// return text or mathmode if that is possible to determine
	virtual mode_type currentMode() const { return UNDECIDED_MODE; }
	/// returns whether changing mode during latex export is forbidden
	virtual bool lockedMode() const { return false; }
	/// returns whether only ascii chars are allowed during latex export
	virtual bool asciiOnly() const { return false; }
	/// returns whether this inset is allowed in other insets of given mode
	virtual bool allowedIn(mode_type) const { return true; }
	/**
	 * The font is inherited from the parent for LaTeX export if this
	 * method returns true. No open font changes are closed in front of
	 * the inset for LaTeX export, and the font is inherited for all other
	 * exports as well as on screen.
	 * If this method returns false all open font changes are closed in
	 * front of the inset for LaTeX export. The default font is used
	 * inside the inset for all exports and on screen.
	 */
	virtual bool inheritFont() const { return true; }
	/**
	 * If this method returns true all explicitly set font attributes
	 * are reset during editing operations.
	 * For copy/paste operations the language is never changed, since
	 * the language of a given text never changes if the text is
	 * formatted differently, while other font attribues like size may
	 * need to change if the text is copied from one environment to
	 * another one.
	 * If this method returns false no font attribute is reset.
	 * The default implementation returns true if the resetFont layout
	 * tag is set and otherwise the negation of inheritFont(),
	 * since inherited inset font attributes do not need to be changed,
	 * and non-inherited ones need to be set explicitly.
	 */
	virtual bool resetFontEdit() const;

	/// set the change for the entire inset
	virtual void setChange(Change const &) {}
	/// accept the changes within the inset
	virtual void acceptChanges() {}
	/// reject the changes within the inset
	virtual void rejectChanges() {}

	///
	virtual Dimension const dimension(BufferView const &) const;
	///
	virtual ColorCode backgroundColor(PainterInfo const &) const;
	///
	virtual ColorCode labelColor() const;
	//
	enum { TEXT_TO_INSET_OFFSET = 4 };

protected:
	/// Constructors
	Inset(Buffer * buf) : buffer_(buf) {}
	Inset(Inset const &) : buffer_(0) {}

	/// replicate ourselves
	friend class InsetList;
	friend class MathAtom;
	virtual Inset * clone() const = 0;

	/** The real dispatcher.
	 *  Gets normally called from Cursor::dispatch(). Cursor::dispatch()
	 *  assumes the common case of 'LFUN handled, need update'.
	 *  This has to be overriden by calling Cursor::undispatched() or
	 *  Cursor::noScreenUpdate() if appropriate.
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
