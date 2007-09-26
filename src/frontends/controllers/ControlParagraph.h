// -*- C++ -*-
/**
 * \file ControlParagraph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLPARAGRAPH_H
#define CONTROLPARAGRAPH_H

#include "Dialog.h"
#include "Layout.h" // for LyXAlignment
#include "ParagraphParameters.h"

namespace lyx {
namespace frontend {

class ControlParagraph : public Controller
{
public:
	///
	ControlParagraph(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data) { return true; }
	/// clean-up on hide.
	virtual void clearParams() {}
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	ParagraphParameters & params();
	///
	ParagraphParameters const & params() const;
	///
	bool haveMulitParSelection();
	///
	bool canIndent() const;
	///
	LyXAlignment alignPossible() const;
	///
	LyXAlignment alignDefault() const;

private:
	ParagraphParameters multiparsel_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLPARAGRAPH_H
