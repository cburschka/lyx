// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 the LyX Team.
 *
 * ====================================================== */

#ifndef LYXINSET_H
#define LYXINSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "gettext.h"
#include "lyxfont.h"
#include "lyxlex.h"
#include "lyxscreen.h"


class Buffer;
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
		LOF_CODE,
		///
		LOT_CODE,
		///
		LOA_CODE,
		///
		QUOTE_CODE,
		///
		MARK_CODE,
		///
		REF_CODE,
		///
		URL_CODE,
		///
		HTMLURL_CODE,
		///
		SEPARATOR_CODE,
		///
		ENDING_CODE,
		///
		LABEL_CODE,
		///
		IGNORE_CODE,
		///
		ACCENT_CODE,
		///
		MATH_CODE,
		///
		INDEX_CODE,
		///
		INCLUDE_CODE,
		///
		GRAPHICS_CODE,
		///
		PARENT_CODE,
		///
		BIBTEX_CODE
	};

	///
	virtual ~Inset() {}
	///
	virtual int Ascent(LyXFont const & font) const = 0;
	///
	virtual int Descent(LyXFont const & font) const = 0;
	///
	virtual int Width(LyXFont const & font) const = 0;
	///
	virtual LyXFont ConvertFont(LyXFont font);
	///
	virtual void Draw(LyXFont font, LyXScreen & scr,
			  int baseline, float & x) = 0;
	/// what appears in the minibuffer when opening
	virtual char const * EditMessage() const {return _("Opened inset");}
	///
	virtual void Edit(int, int);
	///
	virtual unsigned char Editable() const;
	///
	virtual bool AutoDelete() const;
	///
	virtual void Write(ostream &) = 0;
	///
	virtual void Read(LyXLex & lex) = 0;
	/** returns the number of rows (\n's) of generated tex code.
	 fragile != 0 means, that the inset should take care about
	 fragile commands by adding a \protect before.
	 */
	virtual int Latex(ostream &, signed char fragile) = 0;
	///
	virtual int Latex(string & file, signed char fragile) = 0;
	///
	virtual int Linuxdoc(string & /*file*/) = 0;
	///
	virtual int DocBook(string & /*file*/) = 0;
	/// Updates needed features for this inset.
	virtual void Validate(LaTeXFeatures & features) const;
	///
	virtual bool Deletable() const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual Inset::Code LyxCode() const = 0;
  
	/// Get the label that appears at screen
	virtual string getLabel(int) const {
		return string();
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
	virtual int GetNumberOfLabels() const {
		return 0;
	}

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
	///
	//virtual ~UpdatableInset() {}
	///
	virtual unsigned char Editable() const;
   
	/// may call ToggleLockedInsetCursor
	virtual void ToggleInsetCursor();
	///
	virtual void GetCursorPos(int &, int &) const {}
	///
	virtual void InsetButtonPress(int x, int y, int button);
	///
	virtual void InsetButtonRelease(int x, int y, int button);
	
	///
	virtual void InsetKeyPress(XKeyEvent * ev);
	///
	virtual void InsetMotionNotify(int x, int y, int state);
	///
	virtual void InsetUnlock();
   
	///  An updatable inset could handle lyx editing commands
	virtual bool LocalDispatch(int, char const *) { return false; };
	//
	bool isCursorVisible() const { return cursor_visible; }
protected:
	///
	bool cursor_visible;
};
#endif
