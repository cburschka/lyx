// -*- C++ -*-
/**
 * \file FormAboutlyx.h
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 * \author Angus Leeming <a.leeming@.ac.uk>
 */

#ifndef FORMABOUTLYX_H
#define FORMABOUTLYX_H

#include <vector>
#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif


#include "FormBase.h"

class ControlAboutlyx;
struct FD_form_aboutlyx;
struct FD_form_tab_version;
struct FD_form_tab_credits;
struct FD_form_tab_license;

/** This class provides an XForms implementation of the FormAboutlyx Dialog.
 */
class FormAboutlyx : public FormCB<ControlAboutlyx, FormDB<FD_form_aboutlyx> > {
public:
	///
	FormAboutlyx(ControlAboutlyx &);

private:
	/// not needed.
	virtual void apply() {}
	/// not needed.
	virtual void update() {}
	/// Build the dialog
	virtual void build();

	/// Fdesign generated method
	FD_form_aboutlyx * build_aboutlyx();
	///
	FD_form_tab_version * build_tab_version();
	///
	FD_form_tab_credits * build_tab_credits();
	///
	FD_form_tab_license * build_tab_license();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_tab_version> version_;
	///
	boost::scoped_ptr<FD_form_tab_credits> credits_;
	///
	boost::scoped_ptr<FD_form_tab_license> license_;
};

#endif // FORMABOUTLYX_H

