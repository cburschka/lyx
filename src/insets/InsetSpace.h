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

struct InsetSpaceParams {
	/// The different kinds of spaces we support
	enum Kind {
		/// Normal space ('\ ')
		NORMAL,
		/// Protected (no break) space ('~')
		PROTECTED,
		/// Visible ("open box") space ('\textvisiblespace')
		VISIBLE,
		/// Thin space ('\,')
		THIN,
		/// Medium space ('\:')
		MEDIUM,
		/// Thick space ('\;')
		THICK,
		/// \quad (1em)
		QUAD,
		/// \qquad (2em)
		QQUAD,
		/// \enskip (0.5em unbreakable)
		ENSPACE,
		/// \enspace (0.5em breakable)
		ENSKIP,
		/// Negative thin space ('\negthinspace')
		NEGTHIN,
		/// Negative medium space ('\negmedspace')
		NEGMEDIUM,
		/// Negative thick space ('\negthickspace')
		NEGTHICK,
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
	InsetSpaceParams(bool m = false) : kind(NORMAL), math(m) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Kind kind;
	///
	GlueLength length;
	/**
	 * Whether these params are to be used in mathed.
	 * This determines the set of valid kinds.
	 */
	bool math;
};


///  Used to insert different kinds of spaces
class InsetSpace : public Inset
{
public:
	///
	InsetSpace() : Inset(0) {}
	///
	explicit InsetSpace(InsetSpaceParams const & par);
	///
	InsetSpaceParams const & params() const { return params_; }
	///
	InsetSpaceParams::Kind kind() const;

	///
	static void string2params(std::string const &, InsetSpaceParams &);
	///
	static std::string params2string(InsetSpaceParams const &);
	///
	GlueLength length() const;

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
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void toString(odocstream &) const;
	///
	void forOutliner(docstring &, size_t) const;
	///
	bool hasSettings() const { return true; }
	///
	bool clickable(int, int) const { return true; }
	///
	InsetCode lyxCode() const { return SPACE_CODE; }
	/// does this inset try to use all available space (like \\hfill does)?
	bool isHfill() const;
	/// should this inset be handled like a normal character?
	bool isChar() const { return true; }
	/// is this equivalent to a letter?
	bool isLetter() const { return false; }
	/// is this equivalent to a space (which is BTW different from
	// a line separator)?
	bool isSpace() const { return true; }
	///
	std::string contextMenuName() const;
protected:
	///
	Inset * clone() const { return new InsetSpace(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
public:
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;

private:
	///
	InsetSpaceParams params_;
};


} // namespace lyx

#endif // INSET_SPACE_H
