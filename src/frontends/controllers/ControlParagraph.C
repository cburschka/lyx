/**
 * \file ControlParagraph.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlParagraph.h"
#include "ButtonController.h"
#include "funcrequest.h"
#include "lyxlex.h"
#include "paragraph.h"
#include "ParagraphParameters.h"
#include "support/std_sstream.h"

using std::istringstream;
using std::ostringstream;


ControlParagraph::ControlParagraph(Dialog & parent)
	: Dialog::Controller(parent), ininset_(false)
{}


bool ControlParagraph::initialiseParams(string const & data)
{
	istringstream is(data);
	LyXLex lex(0,0);
	lex.setStream(is);

	// Set tri-state flag:
	// action == 0: show dialog
	// action == 1: update dialog, accept changes
	// action == 2: update dialog, do not accept changes
	int action = 0;

	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();

		if (token == "show") {
			action = 0;
		} else if (token == "update") {
			lex.next();
			bool const accept = lex.getBool();
			action = accept ? 1 : 2;
		} else if (!token.empty()) {
			// Unrecognised token
			return false;
		}
	}

	ParagraphParameters * tmp = new ParagraphParameters;
	tmp->read(lex);

	// For now, only reset the params on "show".
	// Don't bother checking if the params are different on "update"
	if (action == 0) {
		params_.reset(tmp);
	} else {
		delete tmp;
	}

	// Read the rest of the data irrespective of "show" or "update"
	int nset = 0;
	while (lex.isOK()) {
		lex.next();
		string const token = lex.getString();

		if (token.empty())
			continue;

		int Int = 0;
		if (token == "\\alignpossible" ||
		    token == "\\aligndefault" ||
		    token == "\\ininset") {
			lex.next();
			Int = lex.getInteger();
		} else {
			// Unrecognised token
			return false;
		}

		++nset;

		if (token == "\\alignpossible") {
			alignpossible_ = static_cast<LyXAlignment>(Int);
		} else if (token == "\\aligndefault") {
			aligndefault_ = static_cast<LyXAlignment>(Int);
		} else {
			ininset_ = Int;
		}
	}
	if (nset != 3) {
		return false;
	}

	// If "update", then set the activation status of the button controller
	if (action > 0) {
		bool const accept = action == 1;
		dialog().bc().valid(accept);
	}
	return true;
}


void ControlParagraph::clearParams()
{
	params_.reset();
}


void ControlParagraph::dispatchParams()
{
	ostringstream data;
	params().write(data);
	FuncRequest const fr(LFUN_PARAGRAPH_APPLY, data.str());
	kernel().dispatch(fr);
}


ParagraphParameters & ControlParagraph::params()
{
	BOOST_ASSERT(params_.get());
	return *params_;
}


ParagraphParameters const & ControlParagraph::params() const
{
	BOOST_ASSERT(params_.get());
	return *params_;
}


bool ControlParagraph::inInset() const
{
	return ininset_;
}


LyXAlignment ControlParagraph::alignPossible() const
{
	return alignpossible_;
}


LyXAlignment ControlParagraph::alignDefault() const
{
	return aligndefault_;
}
