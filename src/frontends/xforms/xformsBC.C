/**
 * \file xformsBC.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae <rae@lyx.org>
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "xforms_helpers.h"


xformsBC::xformsBC(string const & cancel, string const & close)
	: ButtonControllerBase(cancel, close),
	  okay_(0), apply_(0), cancel_(0), undo_all_(0), read_only_()
{}


void xformsBC::refresh()
{
	if (okay_) {
		if (bp().buttonStatus(ButtonPolicy::OKAY)) {
			setEnabled(okay_, true);
		} else {
			setEnabled(okay_, false);
		}
	}
	if (apply_) {
		if (bp().buttonStatus(ButtonPolicy::APPLY)) {
			setEnabled(apply_, true);
		} else {
			setEnabled(apply_, false);
		}
	}
	if (undo_all_) {
		if (bp().buttonStatus(ButtonPolicy::UNDO_ALL)) {
			setEnabled(undo_all_, true);
		} else {
			setEnabled(undo_all_, false);
		}
	}
	if (cancel_) {
		if (bp().buttonStatus(ButtonPolicy::CANCEL)) {
			fl_set_object_label(cancel_, cancel_label.c_str());
		} else {
			fl_set_object_label(cancel_, close_label.c_str());
		}
	}
	if (!read_only_.empty()) {
		bool enable = true;
		if (bp().isReadOnly()) enable = false;
 
		for (std::list<FL_OBJECT *>::iterator iter = read_only_.begin();
		     iter != read_only_.end(); ++iter) {
			setEnabled(*iter, enable);
		}
	}
}
