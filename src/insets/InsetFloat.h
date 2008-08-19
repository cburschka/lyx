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

#include "InsetCollapsable.h"


namespace lyx {

class InsetFloatParams {
public:
	///
	InsetFloatParams() : wide(false), sideways(false), subfloat(false) {}
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
class InsetFloat : public InsetCollapsable
{
public:
	///
	InsetFloat(Buffer const &, std::string const &);
	///
	~InsetFloat();

	///
	static void string2params(std::string const &, InsetFloatParams &);
	///
	static std::string params2string(InsetFloatParams const &);
	///
	void setWide(bool w, BufferParams const &, bool update_label = true);
	///
	void setSideways(bool s, BufferParams const &, bool update_label = true);
	///
	void setSubfloat(bool s, BufferParams const &, bool update_label = true);
	///
	void setNewLabel(BufferParams const &);
	///
	InsetFloatParams const & params() const { return params_; }
private:
	///
	docstring name() const { return name_; }
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
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring editMessage() const;
	///
	bool insetAllowed(InsetCode) const;
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	bool showInsetDialog(BufferView *) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	// Update the counters of this inset and of its contents
	void updateLabels(ParIterator const &);
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetFloat(*this); }
	///
	docstring getCaption(OutputParams const &) const;
	///
	docstring getCaptionText(OutputParams const &) const;
	///
	InsetFloatParams params_;
	///
	docstring name_;
};


} // namespace lyx

#endif // INSET_FLOAT_H
