// -*- C++ -*-
/**
 * \file insetminipage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETMINIPAGE_H
#define INSETMINIPAGE_H

#include "insetcollapsable.h"
#include "lyxlength.h"

/** The minipage inset

*/
class InsetMinipage : public InsetCollapsable {
public:
	///
	enum Position {
		top,
		center,
		bottom
	};
	///
	enum InnerPosition {
		inner_center,
		inner_top,
		inner_bottom,
		inner_stretch
	};

	///
	struct Params {
		///
		Params();
		///
		Position pos;
		///
		InnerPosition inner_pos;
		///
		LyXLength height;
		///
		LyXLength width;
		///
		void write(std::ostream & os) const;
		///
		void read(LyXLex & lex);
	};

	///
	InsetMinipage(BufferParams const &);
	///
	InsetMinipage(InsetMinipage const &);
	///
	~InsetMinipage();
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::MINIPAGE_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	std::string const editMessage() const;
	///
	bool insetAllowed(InsetOld::Code) const;
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	bool showInsetDialog(BufferView *) const;
	///
	int latexTextWidth(BufferView *) const;
	///
	void params(Params const & p) { params_ = p; }
	///
	Params const & params() const { return params_; }
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
private:
	///
	Params params_;
};


#include "mailinset.h"


class InsetMinipageMailer : public MailInset {
public:
	///
	InsetMinipageMailer(InsetMinipage & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &, InsetMinipage::Params &);
	///
	static std::string const params2string(InsetMinipage::Params const &);
private:
	///
	static std::string const name_;
	///
	InsetMinipage & inset_;
};

#endif
