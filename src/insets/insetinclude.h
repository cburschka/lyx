// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	
 *	    Copyright 1997 LyX Team (this file was created this year)
 *
 * ====================================================== */

#ifndef INSET_INCLUDE_H
#define INSET_INCLUDE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

class Buffer;
struct LaTeXFeatures;

// Created by AAS 970521

/**  Used to include files
 */
class InsetInclude: public InsetButton, public boost::noncopyable {
public:
        /// the type of inclusion
        enum IncludeFlags {
		///
		INCLUDE= 0,
		///
		VERB = 1,
		///
		INPUT = 2,
		///
		VERBAST = 3
	};

	struct InsetIncludeParams {
		InsetIncludeParams(InsetCommandParams const & cp = InsetCommandParams(),
			IncludeFlags f = INCLUDE, bool nl = false, Buffer const * b = 0)
			: cparams(cp), flag(f), noload(nl), buffer(b) {}
		InsetCommandParams cparams;
		IncludeFlags flag;
		bool noload;
		Buffer const * buffer;
	};

	///
	InsetInclude(InsetIncludeParams const &);
	///
	InsetInclude(InsetCommandParams const &, Buffer const &);
	///
	~InsetInclude();

	/// get the parameters
	InsetIncludeParams const & params(void) const;

	/// set the parameters
	void setFromParams(InsetIncludeParams const & params);

        ///
        Inset * Clone(Buffer const &) const;
	///
	Inset::Code LyxCode() const { return Inset::INCLUDE_CODE; }
	/// This returns the list of labels on the child buffer
	std::vector<string> const getLabelList() const;
	/// This returns the list of bibkeys on the child buffer
	std::vector< std::pair<string,string> > const getKeys() const;
	///
	void Edit(BufferView *, int x, int y, unsigned int button);
	///
	EDITABLE Editable() const
	{
		return IS_EDITABLE;
	}
        /// With lyx3 we won't overload these 3 methods
        void Write(Buffer const *, std::ostream &) const;
        ///
	void Read(Buffer const *, LyXLex &);
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	void Validate(LaTeXFeatures &) const;
	
        /** Input inserts anything inside a paragraph.
	    Display can give some visual feedback
	*/
	bool display() const;

	/// return the filename stub of the included file 
	string const getRelFileBaseName() const;
 
	/// return true if the included file is not loaded
	bool isIncludeOnly() const;

	/// return true if the file is or got loaded.
	bool loadIfNeeded() const;
 
	/// hide a dialog if about 
	SigC::Signal0<void> hideDialog;
private:
	/// get the text displayed on the button
	string const getScreenLabel() const;
	/// is this a verbatim include ?
	bool isVerbatim() const;
        /// get the filename of the master buffer
        string const getMasterFilename() const;
        /// get the included file name
        string const getFileName() const;

	/// the parameters
	InsetIncludeParams params_;
	///
	string include_label;
};


inline bool InsetInclude::isVerbatim() const
{
	return params_.flag == VERB || params_.flag == VERBAST;
}


inline bool InsetInclude::isIncludeOnly() const
{
	return params_.flag == INCLUDE && params_.noload;
}

#endif
