// -*- C++ -*-
/**
 * \file FormAboutlyx.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORMABOUTLYX_H
#define FORMABOUTLYX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

#include <boost/scoped_ptr.hpp>

class ControlAboutlyx;
struct FD_aboutlyx;
struct FD_aboutlyx_version;
struct FD_aboutlyx_credits;
struct FD_aboutlyx_license;

/** This class provides an XForms implementation of the FormAboutlyx Dialog.
 */
class FormAboutlyx : public FormCB<ControlAboutlyx, FormDB<FD_aboutlyx> > {
public:
	///
	FormAboutlyx();

private:
	/// not needed.
	virtual void apply() {}
	/// not needed.
	virtual void update() {}
	/// Build the dialog
	virtual void build();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_aboutlyx_version> version_;
	///
	boost::scoped_ptr<FD_aboutlyx_credits> credits_;
	///
	boost::scoped_ptr<FD_aboutlyx_license> license_;
};


#endif // FORMABOUTLYX_H
