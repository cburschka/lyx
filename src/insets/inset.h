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
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETOLD_H
#define INSETOLD_H

#include "insetbase.h"
#include "dimension.h"
#include "ParagraphList_fwd.h"


class Buffer;
class LColor_color;
class FuncRequest;
class OutputParams;
class LyXCursor;
class LyXFont;
class LyXLex;
class LyXText;
class Painter;
class Paragraph;
class UpdatableInset;

namespace lyx {
namespace graphics {
	class PreviewLoader;
}
}

/// Insets
class InsetOld : public InsetBase {
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
		MINIPAGE_CODE,
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
		CHARSTYLE_CODE
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
	InsetOld();
	///
	InsetOld(InsetOld const & in);
	///
	int ascent() const;
	///
	int descent() const;
	///
	int width() const;
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
	/// returns true the inset can hold an inset of given type
	virtual bool insetAllowed(InsetOld::Code) const { return false; }
	/// wrapper around the above
	bool insetAllowed(InsetOld * in) const;
	///
	virtual void write(Buffer const &, std::ostream &) const = 0;
	///
	virtual void read(Buffer const &, LyXLex & lex) = 0;
	/// returns the number of rows (\n's) of generated tex code.
	virtual int latex(Buffer const &, std::ostream &,
			  OutputParams const &) const = 0;
	///
	virtual int plaintext(Buffer const &, std::ostream &,
			  OutputParams const &) const = 0;
	///
	virtual int linuxdoc(Buffer const &, std::ostream &,
			     OutputParams const &) const = 0;
	///
	virtual int docbook(Buffer const &, std::ostream &,
			    OutputParams const &) const = 0;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual InsetOld::Code lyxCode() const { return NO_CODE; }

	/// returns true to override begin and end inset in file
	virtual bool directWrite() const;

	///
	void setInsetName(std::string const & s) { name_ = s; }
	///
	std::string const & getInsetName() const { return name_; }
	///
	void setOwner(UpdatableInset * inset) { owner_ = inset; }
	///
	UpdatableInset * owner() const { return owner_; }
	///
	void setBackgroundColor(LColor_color);
	///
	LColor_color backgroundColor() const;
	///
	int x() const { return xo_; }
	///
	int y() const { return yo_; }
	/// returns the actual scroll-value
	virtual int scroll(bool recursive = true) const;

	/// if this insets owns paragraphs (f.ex. InsetText) then it
	/// should return it's very first one!
	virtual ParagraphList * getParagraphs(int /*num*/) const { return 0; }
	///
	virtual LyXText * getText(int /*num*/) const { return 0; }
	///
	virtual int numParagraphs() const { return 0; }

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
	// if this inset has paragraphs should they be output all as default
	// paragraphs with "Standard" layout?
	virtual bool forceDefaultParagraphs(InsetOld const *) const;
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

	/// does this inset allows spellchecking?
	virtual bool allowSpellCheck() const { return true; }

	/** Adds a LaTeX snippet to the Preview Loader for transformation
	 *  into a bitmap image. Does not start the laoding process.
	 *
	 *  Most insets have no interest in this capability, so the method
	 *  defaults to empty.
	 */
	virtual void addPreview(lyx::graphics::PreviewLoader &) const {}
protected:
	///
	mutable int xo_;
	///
	mutable int yo_;
	///
	mutable int scx;
	///
	mutable Dimension dim_;

private:
	///
	UpdatableInset * owner_;
	///
	std::string name_;
	/** We store the LColor::color value as an int to get LColor.h out
	 *  of the header file.
	 */
	int background_color_;
};


/** \c InsetOld_code is a wrapper for InsetOld::Code.
 *  It can be forward-declared and passed as a function argument without
 *  having to expose inset.h.
 */
class InsetOld_code {
	InsetOld::Code val_;
public:
	InsetOld_code(InsetOld::Code val) : val_(val) {}
	operator InsetOld::Code() const { return val_; }
};


/**
 * returns true if pointer argument is valid
 * and points to an editable inset
 */
bool isEditableInset(InsetOld const * inset);


/**
 * returns true if pointer argument is valid
 * and points to a highly editable inset
 */
bool isHighlyEditableInset(InsetOld const * inset);

#endif
