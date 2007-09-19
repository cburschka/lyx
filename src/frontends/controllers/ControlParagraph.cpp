/**
 * \file ControlParagraph.cpp
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
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "Lexer.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"

#include <sstream>

using std::istringstream;
using std::ostringstream;
using std::string;

namespace lyx {
namespace frontend {

ControlParagraph::ControlParagraph(Dialog & parent)
	: Controller(parent), ininset_(false)
{}


bool ControlParagraph::initialiseParams(string const & data)
{
	istringstream is(data);
	Lexer lex(0,0);
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
			if (lex) {
				action = accept ? 1 : 2;
			} else {
				// Unrecognised update option
				return false;
			}
		} else if (!token.empty()) {
			// Unrecognised token
			return false;
		}
	}

	ParagraphParameters tmp;
	tmp.read(lex);

	// For now, only reset the params on "show".
	// Don't bother checking if the params are different on "update"
	if (action == 0)
		params_ = tmp;

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
		dialog().setButtonsValid(accept);
	}
	return true;
}


void ControlParagraph::clearParams()
{
	params_ = ParagraphParameters();
}


void ControlParagraph::dispatchParams()
{
	ostringstream data;
	params().write(data);
	FuncRequest const fr(LFUN_PARAGRAPH_PARAMS_APPLY, data.str());
	dispatch(fr);
}


bool ControlParagraph::haveMulitParSelection()
{
	Cursor cur = bufferview()->cursor();
	return cur.selection() && cur.selBegin().pit() != cur.selEnd().pit();
}

	
bool ControlParagraph::canIndent() const
{
	return buffer().params().paragraph_separation ==
		BufferParams::PARSEP_INDENT;
}


LyXAlignment ControlParagraph::alignPossible() const
{
	return alignpossible_;
}


LyXAlignment ControlParagraph::alignDefault() const
{
	return aligndefault_;
}

} // namespace frontend
} // namespace lyx
