// -*- C++ -*-
/**
 * \file insetbibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_BIBTEX_H
#define INSET_BIBTEX_H


#include "inset.h"
#include "renderers.h"
#include "support/filename.h"
#include "mailinset.h"
#include <vector>


class InsetBibtexParams {
public:
	InsetBibtexParams();
	bool empty() const;
	void erase();
	void write(Buffer const &, std::ostream &) const;
	void read(Buffer const &, LyXLex &);

	std::vector<lyx::support::FileName> databases;
	string style;
	bool bibtotoc;
};


class InsetBibtex : public InsetOld {
public:
	///
	InsetBibtex();
	InsetBibtex(InsetBibtexParams const &);
	///
	~InsetBibtex();
	///
	std::auto_ptr<InsetBase> clone() const;
	/// small wrapper for the time being
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	///
	string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::BIBTEX_CODE; }
	///
	virtual void metrics(MetricsInfo &, Dimension &) const;
	virtual void draw(PainterInfo & pi, int x, int y) const;
	///
	virtual void write(Buffer const &, std::ostream & os) const;
	virtual void read(Buffer const &, LyXLex & lex);
	///
	bool display() const { return true; }
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	virtual int ascii(Buffer const &, std::ostream &, int linelen) const;
	virtual int linuxdoc(Buffer const &, std::ostream &) const;
	virtual int docbook(Buffer const &, std::ostream &, bool) const;
	///
	void fillWithBibKeys(Buffer const & buffer,
		std::vector<std::pair<string,string> > & keys) const;
	///
	std::vector<string> const getFiles(Buffer const &) const;
	///
	bool addDatabase(string const &);
	bool delDatabase(string const &);
	///
	InsetBibtexParams const & params() const { return params_; }
	void setParams(InsetBibtexParams const &);
private:
	InsetBibtexParams params_;
	mutable bool set_label_;
	mutable unsigned int center_indent_;
	mutable ButtonRenderer button_;
};


class InsetBibtexMailer : public MailInset {
public:
	///
	InsetBibtexMailer(InsetBibtex & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string(Buffer const &) const;
	///
	static void string2params(string const &, Buffer const &,
				  InsetBibtexParams &);
	///
	static string const params2string(InsetBibtexParams const &,
					  Buffer const &);
private:
	///
	InsetBibtex & inset_;
	///
	static string const name_;
};

#endif // INSET_BIBTEX_H
