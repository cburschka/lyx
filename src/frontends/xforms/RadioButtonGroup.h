// -*- C++ -*-
/**
 * \file RadioButtonGroup.h
 * Copyright 2000 Baruch Even
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS
 */


#ifndef RADIOBUTTONGROUP_H
#define RADIOBUTTONGROUP_H

#ifdef __GNUG__
#pragma interface
#endif

#include "support/types.h"
#include <vector>
#include <utility>
#include "forms_fwd.h"

/** This class simplifies interaction with a group of radio buttons:
 *  one, and only one, can be selected.
 *  The idea is that you register a bunch of radio buttons with
 *  an accompanying value. Then you can get or set the active button with a
 *  single function call.
 *  It is necessary to also group a family of radio buttons in the
 *  corresponding .fd file in order to unset the previously chosen button
 *  when a new one is selected.
 */
class RadioButtonGroup {
public:
	///
	typedef lyx::size_type size_type;

	/// Register a radio button with its corresponding value.
	void init(FL_OBJECT * ob, size_type value);

	// Set a single active button.
	void set(size_type value);
	void set(FL_OBJECT * ob);

	// Get the active button's value.
	size_type get() const;

private:
	///
	typedef std::pair<FL_OBJECT *, size_type> ButtonValuePair;
	///
	typedef std::vector<ButtonValuePair> ButtonValueMap;
	///
	ButtonValueMap map;
};

#endif
