// -*- C++ -*-
/**
 * \file ControlRef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLREF_H
#define CONTROLREF_H


#include "ControlCommand.h"
#include <vector>

namespace lyx {
namespace frontend {

/** A controller for the Ref Dialog.
 */
class ControlRef : public ControlCommand {
public:
	///
	ControlRef(Dialog &);
	///
	std::vector<std::string> const getLabelList(std::string const &) const;
	///
	void gotoRef(std::string const &);
	///
	void gotoBookmark();
	///
	std::vector<std::string> const getBufferList() const;
	///
	int getBufferNum() const;
	///
	std::string const getBufferName(int num) const;
private:
	/** disconnect from the inset when the Apply button is pressed.
	 Allows easy insertion of multiple references. */
	virtual bool disconnectOnApply() const { return true; }
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLREF_H
