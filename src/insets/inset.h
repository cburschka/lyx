// -*- C++ -*-
/**
 * \file inset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_H
#define INSET_H


#include <vector>
#include "LString.h"
#include "LColor.h"
#include "insetbase.h"
#include "frontends/mouse_state.h"
#include "support/types.h"

class LyXFont;
class BufferView;
class Buffer;
class Painter;
class LyXText;
class LyXLex;
class Paragraph;
class LyXCursor;
class FuncRequest;
class WordLangTuple;

struct LaTeXFeatures;

namespace grfx {
	class PreviewLoader;
}

/// Insets
class Inset : public InsetBase {
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
		BIBITEM_CODE,
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
		WRAP_CODE,
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
		INDEX_PRINT_CODE,
		///
		OPTARG_CODE
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
	typedef dispatch_result RESULT;

	///
	Inset();
	///
	Inset(Inset const & in, bool same_id = false);
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
	///
	virtual bool isTextInset() const { return false; }
	///
	virtual bool doClearArea() const { return true; }
	/// return true if the inset should be removed automatically
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
	Paragraph * parOwner() const { return par_owner_; }
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

	/// try to get a inset pointer from it's id if we have
	/// an inset to give back!
	virtual Inset * getInsetFromID(int /*id*/) const { return 0; }
	/// if this insets owns paragraphs (f.ex. InsetText) then it
	/// should return it's very first one!
	virtual Paragraph * firstParagraph() const { return 0; }
	///
	virtual Paragraph * getFirstParagraph(int /*num*/) const { return 0; }

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
	virtual bool allowSpellcheck() const { return false; }

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

	/// mark the inset contents as erased (for change tracking)
	virtual void markErased() {}
 
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
	///
	virtual void cache(BufferView *) const {}
	///
	virtual BufferView * view() const { return 0; }

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
