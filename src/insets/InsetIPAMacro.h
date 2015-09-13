// -*- C++ -*-
/**
 * \file InsetIPAMacro.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_IPAMACRO_H
#define INSET_IPAMACRO_H


#include "Inset.h"
#include "InsetCollapsable.h"


namespace lyx {

class LaTeXFeatures;

class InsetIPADecoParams
{
public:
	enum Type {
		Toptiebar,
		Bottomtiebar
	};
	///
	InsetIPADecoParams();
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Type type;
};

/////////////////////////////////////////////////////////////////////////
//
// InsetIPADeco
//
/////////////////////////////////////////////////////////////////////////

/// Used to insert IPA decorations
class InsetIPADeco : public InsetCollapsable
{
public:
	///
	InsetIPADeco(Buffer *, std::string const &);
	///
	~InsetIPADeco();
	///
	static std::string params2string(InsetIPADecoParams const &);
	///
	static void string2params(std::string const &, InsetIPADecoParams &);
	///
	InsetIPADecoParams const & params() const { return params_; }
private:
	///
	InsetCode lyxCode() const { return IPADECO_CODE; }
	///
	docstring layoutName() const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	bool neverIndent() const { return true; }
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
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	void validate(LaTeXFeatures & features) const;
	///
	bool allowSpellCheck() const { return false; }
	///
	bool insetAllowed(InsetCode code) const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	Inset * clone() const { return new InsetIPADeco(*this); }
	/// used by the constructors
	void init();
	///
	friend class InsetIPADecoParams;

	///
	InsetIPADecoParams params_;
};


/////////////////////////////////////////////////////////////////////////
//
// InsetIPAChar
//
/////////////////////////////////////////////////////////////////////////

///  Used to insert special IPA chars that are not available in unicode
class InsetIPAChar : public Inset {
public:

	/// The different kinds of special chars we support
	enum Kind {
		/// falling tone mark
		TONE_FALLING,
		/// rising tone mark
		TONE_RISING,
		/// high-rising tone mark
		TONE_HIGH_RISING,
		/// low-rising tone mark
		TONE_LOW_RISING,
		/// high rising-falling tone mark
		TONE_HIGH_RISING_FALLING
	};

	///
	InsetIPAChar() : Inset(0), kind_(TONE_FALLING) {}
	///
	explicit InsetIPAChar(Kind k);
	///
	Kind kind() const;
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
	void toString(odocstream &) const;
	///
	void forOutliner(docstring &, size_t) const;
	///
	InsetCode lyxCode() const { return IPACHAR_CODE; }
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }
	///
	void validate(LaTeXFeatures &) const;

	/// should this inset be handled like a normal character?
	bool isChar() const { return true; }
	/// is this equivalent to a letter?
	bool isLetter() const { return true; }
private:
	Inset * clone() const { return new InsetIPAChar(*this); }

	/// And which kind is this?
	Kind kind_;
};


} // namespace lyx

#endif
