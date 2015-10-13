// -*- C++ -*-
/**
 * \file InsetFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FLOAT_H
#define INSET_FLOAT_H

#include "InsetCaptionable.h"


namespace lyx {

class InsetFloatParams
{
public:
	///
	InsetFloatParams() : type("senseless"), wide(false), sideways(false), subfloat(false) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	std::string type;
	///
	std::string placement;
	/// span columns
	bool wide;
	///
	bool sideways;
	///
	bool subfloat;
};



/////////////////////////////////////////////////////////////////////////
//
// InsetFloat
//
/////////////////////////////////////////////////////////////////////////

/// Used for "floating" objects like tables, figures etc.
class InsetFloat : public InsetCaptionable
{
public:
	InsetFloat(Buffer * buffer, std::string params_str);
	///
	static void string2params(std::string const &, InsetFloatParams &);
	///
	static std::string params2string(InsetFloatParams const &);
	///
	void setWide(bool w, bool update_label = true);
	///
	void setSideways(bool s, bool update_label = true);
	///
	void setSubfloat(bool s, bool update_label = true);
	///
	void setNewLabel();
	///
	InsetFloatParams const & params() const { return params_; }
	///
	bool allowsCaptionVariation(std::string const &) const;
private:
	///
	void setCaptionType(std::string const & type);
	///
	docstring layoutName() const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return FLOAT_CODE; }
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
	bool insetAllowed(InsetCode) const;
	/** returns false if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool inheritFont() const { return false; }
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	bool hasSubCaptions(ParIterator const & it) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetFloat(*this); }
	///
	docstring getCaption(OutputParams const &) const;
	///
	void getCaption(otexstream & os, OutputParams const & runparams) const;

	InsetFloatParams params_;
};


} // namespace lyx

#endif // INSET_FLOAT_H
