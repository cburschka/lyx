// -*- C++ -*-
/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995-2001 the LyX Team.
 *
 * ====================================================== */

#ifndef INSET_H
#define INSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "LString.h"
#include "commandtags.h"
#include "frontends/mouse_state.h"
#include "WordLangTuple.h"
#include "LColor.h"

class LyXFont;
class BufferView;
class Buffer;
class Painter;
class LyXText;
class LyXLex;
class Paragraph;
class LyXCursor;

struct LaTeXFeatures;

namespace grfx {
	class PreviewLoader;
}

/// Insets
class Inset {
public:
	/** This is not quite the correct place for this enum. I think
	    the correct would be to let each subclass of Inset declare
	    its own enum code. Actually the notion of an Inset::Code
	    should be avoided, but I am not sure how this could be done
	    in a cleaner way. */
	enum Code {
		///
		NO_CODE,
		///
		TOC_CODE,  // do these insets really need a code? (ale)
		///
		QUOTE_CODE,
		///
		MARK_CODE,
		///
		REF_CODE, // 5
		///
		URL_CODE,
		///
		HTMLURL_CODE,
		///
		SEPARATOR_CODE,
		///
		ENDING_CODE,
		///
		LABEL_CODE, // 10
		///
		NOTE_CODE,
		///
		ACCENT_CODE,
		///
		MATH_CODE,
		///
		INDEX_CODE,
		///
		INCLUDE_CODE, // 15
		///
		GRAPHICS_CODE,
		///
		PARENT_CODE,
		///
		BIBTEX_CODE,
		///
		TEXT_CODE,
		///
		ERT_CODE, // 20
		///
		FOOT_CODE,
		///
		MARGIN_CODE,
		///
		FLOAT_CODE,
		///
		MINIPAGE_CODE,
		///
		SPECIALCHAR_CODE, // 25
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
		INDEX_PRINT_CODE
	};

	///
	enum {
		///
		TEXT_TO_INSET_OFFSET = 2
	};

	///
	enum EDITABLE {
		///
		NOT_EDITABLE = 0,
		///
		IS_EDITABLE,
		///
		HIGHLY_EDITABLE
	};

	///
	Inset();
	///
	Inset(Inset const & in, bool same_id = false);
	///
	virtual ~Inset() {}
	///
	virtual int ascent(BufferView *, LyXFont const &) const = 0;
	///
	virtual int descent(BufferView *, LyXFont const &) const = 0;
	///
	virtual int width(BufferView *, LyXFont const &) const = 0;
	///
	virtual void draw(BufferView *, LyXFont const &,
			  int baseline, float & x, bool cleared) const = 0;
	/// update the inset representation
	virtual void update(BufferView *, LyXFont const &, bool = false)
		{}
	/// what appears in the minibuffer when opening
	virtual string const editMessage() const;
	///
	virtual void edit(BufferView *, int x, int y, mouse_button::state button);
	///
	virtual void edit(BufferView *, bool front = true);
	///
	virtual EDITABLE editable() const;
	/// This is called when the user clicks inside an inset
	virtual void insetButtonPress(BufferView *, int, int, mouse_button::state) {}
	/// This is called when the user releases the button inside an inset
	// the bool return is used to see if we opened a dialog so that we can
	// check this from an outer inset and open the dialog of the
	// outer inset if that one has one!
	virtual bool insetButtonRelease(BufferView *, int, int, mouse_button::state)
		{ return editable() == IS_EDITABLE; }
	/// This is called when the user moves the mouse inside an inset
	virtual void insetMotionNotify(BufferView *, int , int, mouse_button::state) {}
	///
	virtual bool isTextInset() const { return false; }
	///
	virtual bool doClearArea() const { return true; }
	///
	virtual bool autoDelete() const;
	/// returns true the inset can hold an inset of given type
	virtual bool insetAllowed(Inset::Code) const { return false; }
	/// wrapper around the above
	bool insetAllowed(Inset * in) const;
	///
	virtual void write(Buffer const *, std::ostream &) const = 0;
	///
	virtual void read(Buffer const *, LyXLex & lex) = 0;
	/** returns the number of rows (\n's) of generated tex code.
	    fragile == true means, that the inset should take care about
	    fragile commands by adding a \protect before.
	    If the free_spc (freespacing) variable is set, then this inset
	    is in a free-spacing paragraph.
	*/
	virtual int latex(Buffer const *, std::ostream &, bool fragile,
			  bool free_spc) const = 0;
	///
	virtual int ascii(Buffer const *,
			  std::ostream &, int linelen = 0) const = 0;
	///
	virtual int linuxdoc(Buffer const *, std::ostream &) const = 0;
	///
	virtual int docbook(Buffer const *, std::ostream &, bool) const = 0;
	/// Updates needed features for this inset.
	virtual void validate(LaTeXFeatures & features) const;
	///
	virtual bool deletable() const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual Inset::Code lyxCode() const { return NO_CODE; }

	virtual std::vector<string> const getLabelList() const {
		return std::vector<string>();
	}

	///
	virtual Inset * clone(Buffer const &, bool same_ids = false) const = 0;

	/// returns true to override begin and end inset in file
	virtual bool directWrite() const;

	/// Returns true if the inset should be centered alone
	virtual bool display() const { return false; }
	/// Changes the display state of the inset
	virtual void display(bool) {}
	///
	/// returns true if this inset needs a row on it's own
	///
	virtual bool needFullRow() const { return false; }
	///
	void setInsetName(string const & s) { name_ = s; }
	///
	string const & getInsetName() const { return name_; }
	///
	void setOwner(Inset * inset) { owner_ = inset; }
	///
	Inset * owner() const { return owner_; }
	///
	void parOwner(Paragraph * par) { par_owner_ = par; }
	///
	Paragraph * parOwner() const {return par_owner_; }
	///
	void setBackgroundColor(LColor::color);
	///
	LColor::color backgroundColor() const;
	///
	int x() const { return top_x; }
	///
	int y() const { return top_baseline; }
	//
	// because we could have fake text insets and have to call this
	// inside them without cast!!!
	///
	virtual LyXText * getLyXText(BufferView const *,
				     bool const recursive = false) const;
	///
	virtual void deleteLyXText(BufferView *, bool = true) const {}
	///
	virtual void resizeLyXText(BufferView *, bool /*force*/= false) const {}
	/// returns the actuall scroll-value
	virtual int scroll(bool recursive=true) const {
		if (!recursive || !owner_)
			return scx;
		return 0;
	}
	/// try to get a paragraph pointer from it's id if we have a
	/// paragraph to give back!
	virtual Paragraph * getParFromID(int /* id */) const {
		return 0;
	}
	/// try to get a inset pointer from it's id if we have
	/// an inset to give back!
	virtual Inset * getInsetFromID(int /* id */) const {
		return 0;
	}
	/// if this insets owns paragraphs (f.ex. InsetText) then it
	/// should return it's very first one!
	virtual Paragraph * firstParagraph() const {
		return 0;
	}

	///
	virtual Paragraph * getFirstParagraph(int /*num*/) const {
		return 0;
	}

	/// return the cursor if we own one otherwise giv'em just the
	/// BufferView cursor to work with.
	virtual LyXCursor const & cursor(BufferView * bview) const;
	/// id functions
	int id() const;
	///
	void id(int id_arg);

	/// used to toggle insets
	// is the inset open?
	virtual bool isOpen() const { return false; }
	/// open the inset
	virtual void open(BufferView *) {}
	/// close the inset
	virtual void close(BufferView *) const {}
	/// check if the font of the char we want inserting is correct
	/// and modify it if it is not.
	virtual bool checkInsertChar(LyXFont &);
	/// we need this here because collapsed insets are only EDITABLE
	virtual void setFont(BufferView *, LyXFont const &,
			 bool toggleall = false, bool selectall = false);
	///
	// needed for spellchecking text
	///
	virtual bool allowSpellcheck() { return false; }

	// should this inset be handled like a normal charater
	virtual bool isChar() const { return false; }
	// is this equivalent to a letter?
	virtual bool isLetter() const { return false; }
	// is this equivalent to a space (which is BTW different from
	// a line separator)?
	virtual bool isSpace() const { return false; }
	// should we break lines after this inset?
	virtual bool isLineSeparator() const { return false; }
	// if this inset has paragraphs should they be output all as default
	// paragraphs with "Standard" layout?
	virtual bool forceDefaultParagraphs(Inset const *) const;
	/** returns true if, when outputing LaTeX, font changes should
            be closed before generating this inset. This is needed for
            insets that may contain several paragraphs */
	virtual bool noFontChange() const { return false; }
	//
	virtual void getDrawFont(LyXFont &) const {}
	/* needed for widths which are % of something
	   returns the value of \textwidth in this inset. Most of the
	   time this is the width of the workarea, but if there is a
	   minipage somewhere, it will be the width of this minipage */
	virtual int latexTextWidth(BufferView *) const;

	/** Adds a LaTeX snippet to the Preview Loader for transformation
	 *  into a bitmap image. Does not start the laoding process.
	 *
	 *  Most insets have no interest in this capability, so the method
	 *  defaults to empty.
	 */
	virtual void addPreview(grfx::PreviewLoader &) const {}

	/** Find the PreviewLoader, add a LaTeX snippet to it and
	 *  start the loading process.
	 *
	 *  Most insets have no interest in this capability, so the method
	 *  defaults to empty.
	 */
	virtual void generatePreview() const {}

protected:
	///
	mutable int top_x;
	///
	mutable bool topx_set; /* have we already drawn ourself! */
	///
	mutable int top_baseline;
	///
	mutable int scx;
	///
	unsigned int id_;
	///
	static unsigned int inset_id;

private:
	///
	Inset * owner_;
	/// the paragraph in which this inset has been inserted
	Paragraph * par_owner_;
	///
	string name_;
	///
	LColor::color background_color_;
};


inline
bool Inset::insetAllowed(Inset * in) const
{
	return insetAllowed(in->lyxCode());
}


inline
bool Inset::checkInsertChar(LyXFont &)
{
	return false;
}

//  Updatable Insets. These insets can be locked and receive
//  directly user interaction. Currently used only for mathed.
//  Note that all pure methods from Inset class are pure here too.
//  [Alejandro 080596]

/** Extracted from Matthias notes:
 *
 * An inset can simple call LockInset in it's edit call and *ONLY*
 * in it's edit call.
 *
 * Unlocking is either done by LyX or the inset itself with a
 * UnlockInset-call
 *
 * During the lock, all button and keyboard events will be modified
 * and send to the inset through the following inset-features. Note that
 * Inset::insetUnlock will be called from inside UnlockInset. It is meant
 * to contain the code for restoring the menus and things like this.
 *
 * If a inset wishes any redraw and/or update it just has to call
 * updateInset(this).
 *
 * It's is completly irrelevant, where the inset is. UpdateInset will
 * find it in any paragraph in any buffer.
 * Of course the_locking_inset and the insets in the current paragraph/buffer
 *  are checked first, so no performance problem should occur.
 */
class UpdatableInset : public Inset {
public:
	/** Dispatch result codes
	    Now that nested updatable insets are allowed, the local dispatch
	    becomes a bit complex, just two possible results (boolean)
	    are not enough.

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
	enum RESULT {
		UNDISPATCHED = 0,
		DISPATCHED,
		DISPATCHED_NOUPDATE,
		FINISHED,
		FINISHED_RIGHT,
		FINISHED_UP,
		FINISHED_DOWN
	};

	/// To convert old binary dispatch results
	RESULT DISPATCH_RESULT(bool b) {
		return b ? DISPATCHED : FINISHED;
	}

	///
	UpdatableInset();
	///
	UpdatableInset(UpdatableInset const & in, bool same_id = false);

	/// check if the font of the char we want inserting is correct
	/// and modify it if it is not.
	virtual bool checkInsertChar(LyXFont &);
	///
	virtual EDITABLE editable() const;

	///
	virtual void toggleInsetCursor(BufferView *);
	///
	virtual void showInsetCursor(BufferView *, bool show = true);
	///
	virtual void hideInsetCursor(BufferView *);
	///
	virtual void fitInsetCursor(BufferView *) const;
	///
	virtual void getCursorPos(BufferView *, int &, int &) const {}
	///
	virtual void insetButtonPress(BufferView *, int x, int y, mouse_button::state button);
	///
	// the bool return is used to see if we opened a dialog so that we can
	// check this from an outer inset and open the dialog of the outer inset
	// if that one has one!
	///
	virtual bool insetButtonRelease(BufferView *,
					int x, int y, mouse_button::state button);
	///
	virtual void insetMotionNotify(BufferView *, int x, int y, mouse_button::state state);
	///
	virtual void insetUnlock(BufferView *);
	///
	virtual void edit(BufferView *, int x, int y, mouse_button::state button);
	///
	virtual void edit(BufferView *, bool front = true);
	///
	virtual void draw(BufferView *, LyXFont const &,
			  int baseline, float & x, bool cleared) const;
	///
	virtual bool insertInset(BufferView *, Inset *) { return false; }
	///
	virtual UpdatableInset * getLockingInset() const {
		return const_cast<UpdatableInset *>(this);
	}
	///
	virtual UpdatableInset * getFirstLockingInsetOfType(Inset::Code c)
		{ return (c == lyxCode()) ? this : 0; }
	///
	virtual int insetInInsetY() const { return 0; }
	///
	virtual bool updateInsetInInset(BufferView *, Inset *)
		{ return false; }
	///
	virtual bool lockInsetInInset(BufferView *, UpdatableInset *)
		{ return false; }
	///
	virtual bool unlockInsetInInset(BufferView *, UpdatableInset *,
					bool /*lr*/ = false)
		{ return false; }
	///  An updatable inset could handle lyx editing commands
	virtual RESULT localDispatch(BufferView *, kb_action, string const &);
	///
	bool isCursorVisible() const { return cursor_visible_; }
	///
	virtual int getMaxWidth(BufferView * bv, UpdatableInset const *) const;
	///
	int scroll(bool recursive = true) const {
		// We need this method to not clobber the real method in Inset
		return Inset::scroll(recursive);
	}
	///
	virtual bool showInsetDialog(BufferView *) const { return false; }
	///
	virtual void nodraw(bool b) const {
		block_drawing_ = b;
	}
	///
	virtual bool nodraw() const {
		return block_drawing_;
	}
	///
	// needed for spellchecking text
	///
	virtual bool allowSpellcheck() { return false; }
	///
	virtual WordLangTuple selectNextWordToSpellcheck(BufferView *, float & value) const;
	///
	virtual void selectSelectedWord(BufferView *) { return; }
	///
	virtual void toggleSelection(BufferView *, bool /*kill_selection*/) {
		return;
	}
	///
	// needed for search/replace functionality
	///
	virtual bool searchForward(BufferView *, string const &,
				   bool = true, bool = false);
	///
	virtual bool searchBackward(BufferView *, string const &,
				    bool = true, bool = false);

protected:
	///
	void toggleCursorVisible() const {
		cursor_visible_ = !cursor_visible_;
	}
	///
	void setCursorVisible(bool b) const {
		cursor_visible_ = b;
	}
	/// scrolls to absolute position in bufferview-workwidth * sx units
	void scroll(BufferView *, float sx) const;
	/// scrolls offset pixels
	void scroll(BufferView *, int offset) const;

private:
	///
	mutable bool cursor_visible_;
	///
	mutable bool block_drawing_;
};

inline
bool UpdatableInset::checkInsertChar(LyXFont &)
{
	return true;
}

/**
 * returns true if pointer argument is valid
 * and points to an editable inset
 */
inline bool isEditableInset(Inset * i)
{
	return i && i->editable();
}

/**
 * returns true if pointer argument is valid
 * and points to a highly editable inset
 */
inline bool isHighlyEditableInset(Inset * i)
{
	return i && i->editable() == Inset::HIGHLY_EDITABLE;
}

#endif
