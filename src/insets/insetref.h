// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 LyX Team (this file was created this year)
 * 
 * ====================================================== */

#ifndef INSET_REF_H
#define INSET_REF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

struct LaTeXFeatures;

/** The reference inset  
 */
class InsetRef : public InsetCommand {
public:
	struct type_info {
		///
		string latex_name;
		///
		string gui_name;
		///
		string short_gui_name;
	};
	static type_info types[];
	///
	static int getType(string const & name);
	///
	static string const & getName(int type);

	///
	InsetRef(InsetCommandParams const &, Buffer const &);
	///
	Inset * Clone(Buffer const & buffer) const {
		return new InsetRef(params(), buffer);
	}
	///
	string const getScreenLabel() const;
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	Inset::Code LyxCode() const { return Inset::REF_CODE; }
	///
	void Edit(BufferView *, int, int, unsigned int);
        ///
	bool display() const { return false; }
	///
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	void Validate(LaTeXFeatures & features) const;
private:
	/// This function escapes 8-bit characters
	string const escape(string const &) const;
	///
	bool isLatex;
};
#endif
