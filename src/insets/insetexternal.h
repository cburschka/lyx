// -*- C++ -*-
/**
 * \file insetexternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup Nielsen
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_EXTERNAL_H
#define INSET_EXTERNAL_H

#include "inset.h"
#include "graphics/GraphicsTypes.h"
#include "LString.h"

#include <boost/scoped_ptr.hpp>
#include <boost/signals/trackable.hpp>


class GraphicInset;

///
class InsetExternal : public Inset, public boost::signals::trackable {
public:
	/// hold parameters settable from the GUI
	struct Params {
		Params();
		~Params();
		/// the filename
		string filename;
		/// the current template used
		string templatename;
		/// The name of the tempfile used for manipulations.
		string tempname;
		/// how the inset is displayed by LyX
		grfx::DisplayType display;
		/// The scale of the displayed graphic (If shown).
		unsigned int lyxscale;
	};

	InsetExternal();
	///
	InsetExternal(InsetExternal const &);
	///
	virtual ~InsetExternal();
	///
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// what appears in the minibuffer when opening
	virtual string const editMessage() const;
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
	virtual int latex(Buffer const *, std::ostream &,
			  LatexRunParams const &) const;
	/// write ASCII output to the ostream
	virtual int ascii(Buffer const *, std::ostream &, int linelen) const;
	/// write LinuxDoc output to the ostream
	virtual int linuxdoc(Buffer const *, std::ostream &) const;
	/// write DocBook output to the ostream
	virtual int docbook(Buffer const *, std::ostream &, bool mixcont) const;

	/// Updates needed features for this inset.
	virtual void validate(LaTeXFeatures & features) const;

	/// returns LyX code associated with the inset. Used for TOC, ...)
	virtual Inset::Code lyxCode() const { return EXTERNAL_CODE; }

	///
	virtual Inset * clone() const;

	/// Set the inset parameters.
	virtual void setParams(Params const &, string const & filepath);

	/** update the file represented by the template.
	    If \param external_in_tmpdir == true, then the generated file is
	    place in the buffer's temporary directory.
	*/
	void updateExternal(string const &, Buffer const *,
			    bool external_in_tmpdir) const;

	/// return a copy of our current params
	Params const & params() const;

private:
	/** This method is connected to the graphics loader, so we are
	 *  informed when the image has been loaded.
	 */
	void statusChanged();

	/** Write the output for a specific file format
	    and generate any external data files.
	    If \param external_in_tmpdir == true, then the generated file is
	    place in the buffer's temporary directory.
	*/
	int write(string const & format, Buffer const *, std::ostream &,
		  bool external_in_tmpdir = false) const;

	/// the current params
	Params params_;

	/// The thing that actually draws the image on LyX's screen.
	boost::scoped_ptr<GraphicInset> const renderer_;
};


#include "mailinset.h"

class InsetExternalMailer : public MailInset {
public:
	///
	InsetExternalMailer(InsetExternal & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string() const;
	///
	static void string2params(string const &, InsetExternal::Params &);
	///
	static string const params2string(InsetExternal::Params const &);
private:
	///
	static string const name_;
	///
	InsetExternal & inset_;
};

#endif
