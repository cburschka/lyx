// -*- C++ -*-
/**
 * \file RadioButtonGroup.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS.
 */


#ifndef RADIOBUTTONGROUP_H
#define RADIOBUTTONGROUP_H

#include "support/types.h"
#include <vector>
#include <utility>
#include "forms_fwd.h"

namespace lyx {
namespace frontend {

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

	/// Register a radio button with its corresponding value.
	void init(FL_OBJECT * ob, size_type value);

	// Set a single active button.
	void set(size_type value) const;
	void set(FL_OBJECT * ob) const;

	// None of the radiobuttons are set.
	void unset() const;

	// Get the active button's value.
	size_type get() const;

	void setEnabled(bool enabled);

private:
	///
	typedef std::pair<FL_OBJECT *, size_type> ButtonValuePair;
	///
	typedef std::vector<ButtonValuePair> ButtonValueMap;
	///
	ButtonValueMap map;
};

} // namespace frontend
} // namespace lyx

#endif // RADIOBUTTONGROUP_H
