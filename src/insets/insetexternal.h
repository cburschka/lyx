// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_EXTERNAL_H
#define INSET_EXTERNAL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetbutton.h"
#include "LString.h"
#include "../lyx.h"
#include <sigc++/signal_system.h>

struct ExternalTemplate;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Object;
using SigC::Connection;
#endif

///
class InsetExternal : public InsetButton, public Object {
public:
	InsetExternal();
	///
	virtual ~InsetExternal();
	/// what appears in the minibuffer when opening
	virtual string const EditMessage() const;
	///
	virtual void Edit(BufferView *, int x, int y, unsigned int button);
	///
	virtual EDITABLE Editable() const { return IS_EDITABLE; }
	///
	virtual void Write(Buffer const *, std::ostream &) const;
	///
	virtual void Read(Buffer const *, LyXLex & lex);
	/** returns the number of rows (\n's) of generated tex code.
	 fragile == true means, that the inset should take care about
	 fragile commands by adding a \protect before.
	 If the free_spc (freespacing) variable is set, then this inset
	 is in a free-spacing paragraph.
	 */
	virtual int Latex(Buffer const *, std::ostream &, bool fragile,
			  bool free_spc) const;
	///
	virtual int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	virtual int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	virtual int DocBook(Buffer const *, std::ostream &) const;
	/// Updates needed features for this inset.
	virtual void Validate(LaTeXFeatures & features) const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual Inset::Code LyxCode() const { return EXTERNAL_CODE; }
  
	///
	virtual Inset * Clone(Buffer const &) const;

	/// returns the text of the button
	virtual string const getScreenLabel() const;

	/// Callback function for the template drop-down
	static void templateCB(FL_OBJECT *, long);

	/// Callback function for the browse button
	static void browseCB(FL_OBJECT *, long);

	/// Callback function for the edit button
	static void editCB(FL_OBJECT *, long);

	/// Callback function for the view button
	static void viewCB(FL_OBJECT *, long);

	/// Callback function for the update production button
	static void updateCB(FL_OBJECT *, long);

	/// Callback function for the ok button
	static void okCB(FL_OBJECT *, long);

	/// Callback function for the cancel button
	static void cancelCB(FL_OBJECT *, long);
private:
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colors have been re-mapped).
	*/
	void redraw();

	/// Write the output for a specific file format
	int write(string const & format, Buffer const *,
		  std::ostream &) const;

	/// Apply the changes
	void doApply(BufferView * bv);

	/// Execute this command in the directory of this document
	void executeCommand(string const & s, Buffer const * buf) const;

	/// Update if automatic
	void automaticUpdate(BufferView const *) const;

	/// Do update
	void doUpdate(BufferView const *) const;
	/// Do edit
	void doEdit(BufferView const *) const;
	/// Do view
	void doView(BufferView const *) const;

	/// Substitute meta-variables in this string
	string const doSubstitution(Buffer const *, string const & s) const;

	/** Get the LyX name of the currently selected
	    template in the choice list
	*/
	string const getCurrentTemplate() const;

	/// Get a certain template from a LyX name
	ExternalTemplate const getTemplate(string const & lyxname) const;

	/**
	  Get the number starting from 1 of a template with a 
	  specific LyX name for the choice list
	 */
	int getTemplateNumber(string const & guiname) const;

	/// Get the LyX name of a template with a given number starting from 1
	string const getTemplateName(int n) const;

	/// Get a string with all the GUI template names separated by |
	string const getTemplateString() const;

	///
	struct Holder {
		InsetExternal * inset;
		BufferView * view;
	};
	///
	Holder holder;

	/// The external inset form
	FD_form_external * form_external;

	///
	string templatename;

	///
	string filename;

	///
	string parameters;

	/// A temp filename
	string tempname;

	/// Redraw connection.
	Connection r_;
};

#endif
