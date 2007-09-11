// -*- C++ -*-
/**
 * \file ControlBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer (with useful hints from Angus Leeming)
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLBOX_H
#define CONTROLBOX_H


#include "Dialog.h"
#include <vector>
#include <boost/scoped_ptr.hpp>

namespace lyx {

class InsetBoxParams;

namespace frontend {

class ControlBox : public Controller {
public:
	///
	ControlBox(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	InsetBoxParams & params() { return *params_.get(); }
	///
	InsetBoxParams const & params() const { return *params_.get(); }
	///
private:
	///
	boost::scoped_ptr<InsetBoxParams> params_;
};

///
void box_gui_tokens(std::vector<std::string> &, std::vector<docstring> &);
///
void box_gui_tokens_special_length(std::vector<std::string> &,
	std::vector<docstring> &);

} // namespace frontend
} // namespace lyx

#endif // CONTROLBOX_H
