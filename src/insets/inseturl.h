// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997-2000 The LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_URL_H
#define INSET_URL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "buffer.h"
#include "form_url.h"

struct LaTeXFeatures;

/** The url inset  
 */
class InsetUrl : public InsetCommand {
public:
        ///
        enum Url_Flags {
		///
		URL,
		///
		HTML_URL
	};
	
	///
	InsetUrl() : InsetCommand("url"), fd_form_url(0) {
		flag = InsetUrl::URL;
	}
	///
	explicit
	InsetUrl(string const &);
	///
	explicit
	InsetUrl(InsetCommand const &);
	///
	InsetUrl(string const &, string const &, string const &);
	///
	~InsetUrl();
        ///
        Inset * Clone() const { return new InsetUrl(getCommand()); }
	///
	Inset::Code LyxCode() const { return Inset::URL_CODE; }
	///
	void Validate(LaTeXFeatures &) const;
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE Editable() const {
		return IS_EDITABLE;
	}
	///
	char const * EditMessage() const;
        ///
	bool display() const { return false; }
	///
	string getScreenLabel() const;
	///
	InsetUrl::Url_Flags getFlag() const { return flag; }
	///
	void setFlag(InsetUrl::Url_Flags f) { flag = f; }
        ///
        void gotoLabel();
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	static void CloseUrlCB(FL_OBJECT *, long data);
private:
	///
	struct Holder {
		InsetUrl * inset;
		BufferView * view;
	};
	///
	Holder holder;
	
	///
        Url_Flags flag;
	///
        FD_form_url * fd_form_url;
};

#endif
