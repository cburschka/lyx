// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1997 LyX Team (this file was created this year)
 * 
 *======================================================*/

#ifndef _INSET_URL_H
#define _INSET_URL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "buffer.h"

struct LaTeXFeatures;

/** The url inset  
 */
class InsetUrl: public InsetCommand {
public:
        ///
        enum Url_Flags {
		///
		URL,
		///
		HTML_URL
	};
	
	///
	InsetUrl(): InsetCommand("url"), form(0) { flag = InsetUrl::URL; }
	///
	InsetUrl(string const &);
	///
	InsetUrl(InsetCommand const&);
	///
	InsetUrl(string const &,string const &,string const &);
	///
	~InsetUrl();
        ///
        Inset* Clone() { return new InsetUrl(getCommand()); }
	///
	Inset::Code LyxCode() const { return Inset::URL_CODE; }
	///
	void Validate(LaTeXFeatures &) const;
	///
	void Edit(int, int);
	///
	unsigned char Editable() const {
		return 1;
	}
	///
	char const* EditMessage() {return "Opened Url";}
        ///
	bool Display() const { return false; }
	///
	string getScreenLabel() const;
	///
	InsetUrl::Url_Flags getFlag() { return flag; }
	///
	void setFlag(InsetUrl::Url_Flags f) { flag = f; }
        ///
        void gotoLabel();
	///
	int Latex(FILE *file, signed char fragile);
	///
	int Latex(string &file, signed char fragile);
	///
	int Linuxdoc(string &file);
	///
	int DocBook(string &file);
	///
	static void CloseUrlCB(FL_OBJECT *, long data);
private:
	///
        Url_Flags flag;
	///
        FL_FORM *form;
	///
	FL_OBJECT *url_name;
	///
	FL_OBJECT *name_name;
	///
	FL_OBJECT *radio_html;
};

#endif
