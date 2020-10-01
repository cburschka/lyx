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

struct TexString;


class InsetFloatParams
{
public:
	///
	InsetFloatParams() : type("senseless"), placement("document"), alignment("document"),
		wide(false), sideways(false), subfloat(false) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	std::string type;
	///
	std::string placement;
	///
	std::string alignment;
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
	InsetFloat(Buffer * buffer, std::string const & params_str);
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
	bool allowsCaptionVariation(std::string const &) const override;
	///
	LyXAlignment contentAlignment() const override;
	///
	bool forceParDirectionSwitch() const override { return true; }
private:
	///
	void setCaptionType(std::string const & type) override;
	///
	docstring layoutName() const override;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	void write(std::ostream & os) const override;
	///
	void read(Lexer & lex) override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return FLOAT_CODE; }
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	bool insetAllowed(InsetCode) const override;
	/** returns false if, when outputting LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool inheritFont() const override { return false; }
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	bool hasSubCaptions(ParIterator const & it) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	Inset * clone() const override { return new InsetFloat(*this); }
	/// Is the content of this inset part of the immediate (visible) text sequence?
	bool isPartOfTextSequence() const override { return false; }
	///
	TexString getCaption(OutputParams const &) const;
	///
	std::string getAlignment() const;
	///
	OutputParams::CtObject CtObject(OutputParams const &) const override { return OutputParams::CT_OMITOBJECT; }

	InsetFloatParams params_;
};


} // namespace lyx

#endif // INSET_FLOAT_H
