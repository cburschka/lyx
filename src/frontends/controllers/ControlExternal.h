// -*- C++ -*-
/**
 * \file ControlExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLEXTERNAL_H
#define CONTROLEXTERNAL_H


#include "Dialog.h"
#include "insets/insetexternal.h"
#include <boost/scoped_ptr.hpp>


class ControlExternal : public Dialog::Controller {
public:
	///
	ControlExternal(Dialog &);
	///
	virtual void initialiseParams(string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	///
	InsetExternal::Params const & params() const
		{ return inset_->params(); }
	///
	void setParams(InsetExternal::Params const &);

	///
	void editExternal();
	///
	void viewExternal();
	///
	void updateExternal();
	///
	std::vector<string> const getTemplates() const;
	///
	int getTemplateNumber(string const &) const;
	///
	ExternalTemplate getTemplate(int) const;
	///
	string const Browse(string const &) const;
private:
	///
	boost::scoped_ptr<InsetExternal> inset_;
};

#endif // CONTROLEXTERNAL_H
