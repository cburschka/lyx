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
#include "insets/insetcommandparams.h"


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
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	///
	InsetCommandParams const & params() const { return params_; }
	///
	void setParams(InsetCommandParams const &);

	/// Browse for a file
	std::string const browse(std::string const &, Type) const;

	/// load a file
	void load(std::string const & file);

	/// test if file exist
	bool fileExists(std::string const & file);
private:
	///
	InsetCommandParams params_;
};

#endif // CONTROLINCLUDE_H
