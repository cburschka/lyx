// -*- C++ -*-
/**
 * \file ControlToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLTOC_H
#define CONTROLTOC_H


#include "ControlCommand.h"
#include "TocBackend.h"

#include <vector>

namespace lyx {
namespace frontend {

/** A controller for TOC dialogs.
 */
class ControlToc : public ControlCommand {
public:
	///
	ControlToc(Dialog &);
	///
	virtual ~ControlToc() {}

	/// \c ControlCommand inherited method.
	virtual bool initialiseParams(std::string const & data);

	///
	TocList const & tocs() const;

	/// Goto this paragraph id
	void goTo(TocItem const &);

	/// Return the list of types available
	std::vector<docstring> const & typeNames() const
	{ return type_names_; }

	///
	int selectedType() { return selected_type_; }

	/// Return the first TocItem before the cursor
	TocIterator const getCurrentTocItem(size_t type) const;

	/// Apply the selected outlining operation
	void outlineUp();
	///
	void outlineDown();
	///
	void outlineIn();
	///
	void outlineOut();
	/// Test if outlining operation is possible
	bool canOutline(size_t type) const;
	///
	void updateBackend();

private:
	std::vector<std::string> types_;
	std::vector<docstring> type_names_;
	int selected_type_;

	/// Return the guiname from a given cmdName of the TOC param
	docstring const getGuiName(std::string const & type) const;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLTOC_H
