// -*- C++ -*-
/**
 * \file xforms/GuiClipboard.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "frontends/Clipboard.h"

#include "XWorkArea.h"

namespace lyx {
namespace frontend {

/**
 * The XForms version of the Clipboard.
 */
class GuiClipboard: public lyx::frontend::Clipboard
{
public:
	GuiClipboard(XWorkArea * work_area)
		: old_work_area_(work_area)
	{
	}

	virtual ~GuiClipboard() {}

	/** ClipBoard overloaded methods
	 */
	//@{
	void haveSelection(bool own)
	{
		old_work_area_->haveSelection(own);
	}

	std::string const get() const
	{
		return old_work_area_->getClipboard();
	}

	void put(std::string const & str)
	{
		old_work_area_->putClipboard(str);
	}
	//@}

private:
	XWorkArea * old_work_area_;
};

} // namespace frontend
} // namespace lyx

#endif // CLIPBOARD_H
