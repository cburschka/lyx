// -*- C++ -*-
/**
 * \file ControlInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLINCLUDE_H
#define CONTROLINCLUDE_H


#include "Dialog.h"
#include "insets/insetinclude.h" // InsetIncludeParams


/** A controller for the Include file dialog.
 */
class ControlInclude : public Dialog::Controller {
public:
	///
	enum Type {
		///
		INPUT,
		///
		VERBATIM,
		///
		INCLUDE
	};
	///
	ControlInclude(Dialog &);

	///
	virtual bool initialiseParams(string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	///
	InsetInclude::Params const & params() const
		{ return inset_->params(); }
	///
	void setParams(InsetInclude::Params const &);

	/// Browse for a file
	string const Browse(string const &, Type);

	/// load a file
	void load(string const & file);

	/// test if file exist
	bool fileExists(string const & file);
private:
	///
	boost::scoped_ptr<InsetInclude> inset_;
};

#endif // CONTROLINCLUDE_H
