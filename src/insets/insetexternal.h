// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_EXTERNAL_H
#define INSET_EXTERNAL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetbutton.h"
#include "ExternalTemplate.h"
#include "LString.h"
#include <sigc++/signal_system.h>

///
class InsetExternal : public InsetButton {
public:
	/// hold parameters settable from the GUI
	struct Params {
		Params(string const & f = string(), 
					string const & p = string(), 
					ExternalTemplate const & t = ExternalTemplate())
			: filename(f), parameters(p), templ(t) {}
		/// the filename
		string filename;
		/// the parameters of the current choice
		string parameters;
		/// the current template used
		ExternalTemplate templ;
	};

	InsetExternal();
	///
	virtual ~InsetExternal();
	/// what appears in the minibuffer when opening
	virtual string const editMessage() const;
	///
	virtual void edit(BufferView *, int x, int y, unsigned int button);
	///
	virtual void edit(BufferView * bv, bool front = true);
	///
	virtual EDITABLE editable() const { return IS_EDITABLE; }
	///
	virtual void write(Buffer const *, std::ostream &) const;
	///
	virtual void read(Buffer const *, LyXLex & lex);

	/** returns the number of rows (\n's) of generated tex code.
	 fragile == true means, that the inset should take care about
	 fragile commands by adding a \protect before.
	 If the free_spc (freespacing) variable is set, then this inset
	 is in a free-spacing paragraph.
	 */
	virtual int latex(Buffer const *, std::ostream &, bool fragile,
	                  bool free_spc) const;
	/// write ASCII output to the ostream
	virtual int ascii(Buffer const *, std::ostream &, int linelen) const;
	/// write LinuxDoc output to the ostream
	virtual int linuxdoc(Buffer const *, std::ostream &) const;
	/// write DocBook output to the ostream
	virtual int docBook(Buffer const *, std::ostream &) const;

	/// Updates needed features for this inset.
	virtual void validate(LaTeXFeatures & features) const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual Inset::Code lyxCode() const { return EXTERNAL_CODE; }
 
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const;

	/// returns the text of the button
	virtual string const getScreenLabel() const;

	// The following public members are used from the frontends code

	/// set the parameters from a Params structure
	virtual void setFromParams(Params const &);

	/// update the file represented by the template
	void updateExternal() const;

	/// edit file of this template
	void editExternal() const;

	/// view file of this template
	void viewExternal() const;

	/// return a copy of our current params
	Params params() const;

	/// hide connection
	SigC::Signal0<void> hideDialog;

private:
	/// Write the output for a specific file format
	int write(string const & format, Buffer const *,
		  std::ostream &) const;

	/// Execute this command in the directory of this document
	void executeCommand(string const & s, Buffer const * buf) const;

	/// Substitute meta-variables in this string
	string const doSubstitution(Buffer const *, string const & s) const;

	/// our owning view
	BufferView * view_;

	/// the current params
	Params params_;

	/// A temp filename
	string tempname_;
};

///
bool operator==(InsetExternal::Params const &, InsetExternal::Params const &);
///
bool operator!=(InsetExternal::Params const &, InsetExternal::Params const &);

#endif
