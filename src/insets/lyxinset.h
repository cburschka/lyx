// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995-2000 the LyX Team.
 *
 * ====================================================== */

#ifndef LYXINSET_H
#define LYXINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>

#include <X11/Xlib.h>

#include "gettext.h"
#include "lyxfont.h"
#include "lyxlex.h"

class BufferView;
class Buffer;
class Painter;
class LyXText;

struct LaTeXFeatures;


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
		LOF_CODE, // 2
		///
		LOT_CODE,
		///
		LOA_CODE,
		///
		QUOTE_CODE, // 5
		///
		MARK_CODE,
		///
		REF_CODE,
		///
		URL_CODE,
		///
		HTMLURL_CODE,
		///
		SEPARATOR_CODE, // 10
		///
		ENDING_CODE,
		///
		LABEL_CODE,
		///
		IGNORE_CODE,
		///
		ACCENT_CODE,
		///
		MATH_CODE, // 15
		///
		INDEX_CODE,
		///
		INCLUDE_CODE,
		///
		GRAPHICS_CODE,
		///
		PARENT_CODE,
		///
		BIBTEX_CODE, // 20
		///
		TEXT_CODE,
		///
		ERT_CODE,
		///
		FOOT_CODE,
		///
		MARGIN_CODE,
		///
		FLOAT_CODE, // 25
		///
		MINIPAGE_CODE,
		///
		SPECIALCHAR_CODE,
		///
		TABULAR_CODE,
		///
		EXTERNAL_CODE,
		///
		THEOREM_CODE, // 30
		///
		CAPTION_CODE
	};

	///
	enum { TEXT_TO_INSET_OFFSET = 2 };

	enum EDITABLE {
	    NOT_EDITABLE = 0,
	    IS_EDITABLE,
	    HIGHLY_EDITABLE
	};

	///
	Inset() { owner_ = 0; top_x = top_baseline = 0; scx = 0; }
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
	///
	virtual LyXFont ConvertFont(LyXFont const & font) const;
	/// what appears in the minibuffer when opening
	virtual const char * EditMessage() const;
	///
	virtual void Edit(BufferView *, int x, int y, unsigned int button);
	///
	virtual EDITABLE Editable() const;
	/// This is called when the user clicks inside an inset
	virtual void InsetButtonPress(BufferView *, int, int, int) {}
	/// This is called when the user releases the button inside an inset
	virtual void InsetButtonRelease(BufferView *, int, int, int) {}
	/// This is caleld when the user moves the mouse inside an inset
	virtual void InsetMotionNotify(BufferView *, int , int , int) {}
	///
	virtual bool IsTextInset() const { return false; }
	///
	virtual bool doClearArea() const { return true; }
	///
	virtual bool AutoDelete() const;
	///
	virtual void Write(Buffer const *, std::ostream &) const = 0;
	///
	virtual void Read(Buffer const *, LyXLex & lex) = 0;
	/** returns the number of rows (\n's) of generated tex code.
	 fragile == true means, that the inset should take care about
	 fragile commands by adding a \protect before.
	 If the free_spc (freespacing) variable is set, then this inset
	 is in a free-spacing paragraph.
	 */
	virtual int Latex(Buffer const *, std::ostream &, bool fragile,
			  bool free_spc) const = 0;
	///
	virtual int Ascii(Buffer const *, std::ostream &) const = 0;
	///
	virtual int Linuxdoc(Buffer const *, std::ostream &) const = 0;
	///
	virtual int DocBook(Buffer const *, std::ostream &) const = 0;
	/// Updates needed features for this inset.
	virtual void Validate(LaTeXFeatures & features) const;
	///
	virtual bool Deletable() const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual Inset::Code LyxCode() const { return NO_CODE; }
  
	virtual std::vector<string> getLabelList() const {
		return std::vector<string>();
	}

	///
	virtual Inset * Clone() const = 0;

	/// returns true to override begin and end inset in file
	virtual bool DirectWrite() const;

	/// Returns true if the inset should be centered alone
	virtual bool display() const { return false; }
	/// Changes the display state of the inset
	virtual void display(bool) {}
	///
	/// returns true if this inset needs a row on it's own
	///
	virtual bool needFullRow() const { return false; }
	///
	virtual bool InsertInsetAllowed(Inset *) const { return false; }
	///
	virtual void setInsetName(const char * s) { name = s; }
	///
	virtual string getInsetName() const { return name; }
	///
	virtual void setOwner(Inset * inset) { owner_ = inset; }
	///
	virtual Inset * owner() const { return owner_; }
	///
	int x() const { return top_x; }
	///
	int y() const { return top_baseline; }
	///
	/// because we could have fake text insets and have to call this
	/// inside them without cast!!!
	virtual LyXText * getLyXText(BufferView *) const;
	virtual void deleteLyXText(BufferView *, bool =true) const {}
	virtual void resizeLyXText(BufferView *) const {}
	// returns the actuall scroll-value
	int  scroll() const { return scx; }

protected:
	///
	mutable int top_x;
	mutable int top_baseline;
	mutable int scx;

private:
	///
	Inset * owner_;
	///
	string name;
};


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
  * Inset::InsetUnlock will be called from inside UnlockInset. It is meant
  * to contain the code for restoring the menus and things like this.
  * 
  * If a inset wishes any redraw and/or update it just has to call
  * UpdateInset(this).
  * 
  * It's is completly irrelevant, where the inset is. UpdateInset will
  * find it in any paragraph in any buffer. 
  * Of course the_locking_inset and the insets in the current paragraph/buffer
  *  are checked first, so no performance problem should occur.
  */
class UpdatableInset: public Inset {
public:
	/** Dispatch result codes
	    Now that nested updatable insets are allowed, the local dispatch
	    becomes a bit complex, just two possible results (boolean)
	    are not enough. 
	 
	    DISPATCHED   = the inset catched the action
	    DISPATCHED_NOUPDATE = the inset catched the action and no update
                                  is needed here to redraw the inset
	    FINISHED     = the inset must be unlocked as a result
	                   of the action
	    UNDISPATCHED = the action was not catched, it should be
	                   dispatched by lower level insets
	*/ 
	enum RESULT {
	    UNDISPATCHED = 0,
	    DISPATCHED,
	    DISPATCHED_NOUPDATE,
	    FINISHED
	};
    
	/// To convert old binary dispatch results
	RESULT DISPATCH_RESULT(bool b) {
		return b ? DISPATCHED : FINISHED;
	}

	///
	UpdatableInset() {}
	///
	virtual EDITABLE Editable() const;
   
	/// may call ToggleLockedInsetCursor
	virtual void ToggleInsetCursor(BufferView *);
	///
	virtual void ShowInsetCursor(BufferView *);
	///
	virtual void HideInsetCursor(BufferView *);
	///
	virtual void GetCursorPos(BufferView *, int &, int &) const {}
	///
	virtual void InsetButtonPress(BufferView *, int x, int y, int button);
	///
	virtual void InsetButtonRelease(BufferView *,
					int x, int y, int button);
	///
	virtual void InsetKeyPress(XKeyEvent * ev);
	///
	virtual void InsetMotionNotify(BufferView *, int x, int y, int state);
	///
	virtual void InsetUnlock(BufferView *);
	///
	virtual void Edit(BufferView *, int x, int y, unsigned int button);
	///
	virtual void draw(BufferView *, LyXFont const &,
			  int baseline, float & x, bool cleared) const;
	///
	virtual void SetFont(BufferView *, LyXFont const &,
			     bool toggleall = false);
	///
	virtual bool InsertInset(BufferView *, Inset *) { return false; }
	///
	virtual bool InsertInsetAllowed(Inset *) const { return true; }
	///
	virtual UpdatableInset * GetLockingInset() { return this; }
	///
	virtual UpdatableInset * GetFirstLockingInsetOfType(Inset::Code c)
		{ return (c == LyxCode()) ? this : 0; }
	///
	virtual int InsetInInsetY() { return 0; }
	///
	virtual bool UpdateInsetInInset(BufferView *, Inset *)
		{ return false; }
	///
	virtual bool LockInsetInInset(BufferView *, UpdatableInset *)
		{ return false; }
	///
	virtual bool UnlockInsetInInset(BufferView *, UpdatableInset *,
					bool /*lr*/ = false)
		{ return false; }
	///  An updatable inset could handle lyx editing commands
	virtual RESULT LocalDispatch(BufferView *, int, string const &);
	///
	virtual bool isCursorVisible() const { return cursor_visible; }
	///
	virtual int getMaxWidth(Painter & pain, UpdatableInset const *) const;
	///
	int scroll() const { return scx; }

protected:
	///
	mutable bool cursor_visible;

	// scrolls to absolute position in bufferview-workwidth * sx units
	void scroll(BufferView *, float sx) const;
	// scrolls offset pixels
	void scroll(BufferView *, int offset) const;
};
#endif
