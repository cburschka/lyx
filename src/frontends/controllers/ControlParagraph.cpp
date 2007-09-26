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
	: Controller(parent)
{}


ParagraphParameters & ControlParagraph::params()
{
	return bufferview()->cursor().paragraph().params();
}


ParagraphParameters const & ControlParagraph::params() const
{
	return bufferview()->cursor().paragraph().params();
}


void ControlParagraph::dispatchParams()
{
	bufferview()->updateMetrics(false);
	bufferview()->buffer().changed();
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
	return bufferview()->cursor().paragraph().layout()->alignpossible;
}


LyXAlignment ControlParagraph::alignDefault() const
{
	return bufferview()->cursor().paragraph().layout()->align;
}

} // namespace frontend
} // namespace lyx
