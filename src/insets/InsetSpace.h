// -*- C++ -*-
/**
 * \file InsetSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 * \author Jean-Marc Lasgouttes
 * \author Lars Gullik Bjønnes
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_SPACE_H
#define INSET_SPACE_H

#include "Inset.h"
#include "Length.h"


namespace lyx {

class LaTeXFeatures;

class InsetSpaceParams {
public:
	/// The different kinds of spaces we support
	enum Kind {
		/// Normal space ('\ ')
		NORMAL,
		/// Protected (no break) space ('~')
		PROTECTED,
		/// Thin space ('\,')
		THIN,
		/// \quad (1em)
		QUAD,
		/// \qquad (2em)
		QQUAD,
		/// \enspace (0.5em unbreakable)
		ENSPACE,
		/// \enspace (0.5em breakable)
		ENSKIP,
		/// Negative thin space ('\negthinspace')
		NEGTHIN,
		/// rubber length
		HFILL,
		/// \hspace*{\fill}
		HFILL_PROTECTED,
		/// rubber length, filled with dots
		DOTFILL,
		/// rubber length, filled with a rule
		HRULEFILL,
		/// rubber length, filled with a left arrow
		LEFTARROWFILL,
		/// rubber length, filled with a right arrow
		RIGHTARROWFILL,
		// rubber length, filled with an up brace
		UPBRACEFILL,
		// rubber length, filled with a down brace
		DOWNBRACEFILL,
		/// \hspace{length}
		CUSTOM,
		/// \hspace*{length}
		CUSTOM_PROTECTED
	};
	///
	InsetSpaceParams() : kind(NORMAL), length(Length()) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Kind kind;
	///
	Length length;
};


///  Used to insert different kinds of spaces
class InsetSpace : public Inset
{
public:
	///
	InsetSpace() {}
	///
	explicit InsetSpace(InsetSpaceParams const & par);
	///
	InsetSpaceParams params() const { return params_; }
	///
	InsetSpaceParams::Kind kind() const;
	///
	~InsetSpace();

	///
	static void string2params(std::string const &, InsetSpaceParams &);
	///
	static std::string params2string(InsetSpaceParams const &);
	///
	Length length() const;

private:
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	/// Will not be used when lyxf3
	void read(Lexer & lex);
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// the string that is passed to the TOC
	void textString(odocstream &) const;
	///
	void edit(Cursor & cur, bool front,
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetCode lyxCode() const { return SPACE_CODE; }
	/// is this an expandible space (rubber length)?
	bool isStretchableSpace() const;

	// should this inset be handled like a normal charater
	bool isChar() const { return true; }
	/// is this equivalent to a letter?
	bool isLetter() const { return false; }
	/// is this equivalent to a space (which is BTW different from
	// a line separator)?
	bool isSpace() const { return true; }
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	///
	Inset * clone() const { return new InsetSpace(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;

	///
	InsetSpaceParams params_;
};


} // namespace lyx

#endif // INSET_SPACE_H
