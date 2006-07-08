// -*- C++ -*-
/**
 * \file qt3/GuiClipboard.h
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

#include "QWorkArea.h"

namespace lyx {
namespace frontend {

/**
 * The Qt3 version of the Clipboard.
 */
class GuiClipboard: public lyx::frontend::Clipboard
{
public:
	GuiClipboard(QWorkArea * work_area)
		: old_work_area_(work_area)
	{
	}

	virtual ~GuiClipboard() {}

	/** Clipboard overloaded methods
	 */
	//@{
	void haveSelection(bool own)
	{
		old_work_area_->haveSelection(own);
	}

	std::string const get() const;

	void put(std::string const & str);
	//@}

private:
	QWorkArea * old_work_area_;
};

} // namespace frontend
} // namespace lyx

#endif // CLIPBOARD_H
