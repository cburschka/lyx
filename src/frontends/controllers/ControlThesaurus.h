// -*- C++ -*-
/**
 * \file ControlThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */


#ifndef CONTROLTHESAURUS_H
#define CONTROLTHESAURUS_H

#include "Dialog.h"
#include "Thesaurus.h"

namespace lyx {
namespace frontend {

/** A controller for Thesaurus dialogs.
 */
class ControlThesaurus : public Controller {
public:
	///
	ControlThesaurus(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	///
	virtual void clearParams();
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return true; }

	/// replace the particular string
	void replace(docstring const & newstr);

	/// get meanings
	Thesaurus::Meanings const & getMeanings(docstring const & str);

	/// the text
	docstring const & text() const { return oldstr_; }

private:
	/// last string looked up
	docstring laststr_;

	/// entries for last string
	Thesaurus::Meanings meanings_;

	/// original string
	docstring oldstr_;

	/// not needed.
	virtual void apply() {}
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLTHESAURUS_H
