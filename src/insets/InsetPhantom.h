// -*- C++ -*-
/**
 * \file InsetPhantom.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_PHANTOM_H
#define INSET_PHANTOM_H

#include "InsetCollapsable.h"


namespace lyx {

class InsetPhantomParams
{
public:
	enum Type {
		Phantom,
		HPhantom,
		VPhantom
	};
	/// \c type defaults to Phantom
	InsetPhantomParams();
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Type type;
};


/////////////////////////////////////////////////////////////////////////
//
// InsetPhantom
//
/////////////////////////////////////////////////////////////////////////

/// The phantom inset
class InsetPhantom : public InsetCollapsable
{
public:
	///
	InsetPhantom(Buffer const &, std::string const &);
	///
	~InsetPhantom();
	///
	static std::string params2string(InsetPhantomParams const &);
	///
	static void string2params(std::string const &, InsetPhantomParams &);
	///
	InsetPhantomParams const & params() const { return params_; }
private:
	///
	docstring editMessage() const;
	///
	docstring name() const;
	///
	DisplayType display() const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(std::ostream &) const;
	///
	void read(Lexer & lex);
	///
	void setButtonLabel();
	/// show the phantom dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	bool allowParagraphCustomization(idx_type = 0) const { return false; }
	///
	bool forcePlainLayout(idx_type = 0) const { return true; }
	///
	bool neverIndent() const { return true; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	/// used by the constructors
	void init();
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	///
	friend class InsetPhantomParams;

	///
	InsetPhantomParams params_;
};


} // namespace lyx

#endif // INSET_PHANTOM_H
