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

namespace lyx {

class ParagraphParameters;

namespace frontend {

class ControlParagraph : public Dialog::Controller {
public:
	///
	ControlParagraph(Dialog &);
	///
	virtual bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	virtual void clearParams();
	///
	virtual void dispatchParams();
	///
	virtual bool isBufferDependent() const { return true; }
	///
	ParagraphParameters & params();
	///
	ParagraphParameters const & params() const;
	///
	bool const haveMulitParSelection();
	///
	bool inInset() const;
	///
	bool canIndent() const;
	///
	LyXAlignment alignPossible() const;
	///
	LyXAlignment alignDefault() const;

private:
	///
	boost::scoped_ptr<ParagraphParameters> params_;
	///
	bool ininset_;
	///
	LyXAlignment alignpossible_;
	///
	LyXAlignment aligndefault_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLPARAGRAPH_H
