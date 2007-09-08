// -*- C++ -*-
/**
 * \file ControlInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLINCLUDE_H
#define CONTROLINCLUDE_H


#include "Dialog.h"
#include "insets/InsetCommandParams.h"

#include "support/docstring.h"

namespace lyx {
namespace frontend {

/** A controller for the Include file dialog.
 */
class ControlInclude : public Controller {
public:
	///
	enum Type {
		///
		INPUT,
		///
		VERBATIM,
		///
		INCLUDE,
		///
		LISTINGS,
	};
	///
	ControlInclude(Dialog &);

	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	/// clean-up on hide.
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }

	///
	InsetCommandParams const & params() const { return params_; }
	///
	void setParams(InsetCommandParams const &);

	/// Browse for a file
	docstring const browse(docstring const &, Type) const;

	/// edit the child document, .lyx file will be opened in lyx
	/// other formats will be edited by external applications.
	void edit(std::string const & file);

	/// test if file exist
	bool fileExists(std::string const & file);
private:
	///
	InsetCommandParams params_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLINCLUDE_H
