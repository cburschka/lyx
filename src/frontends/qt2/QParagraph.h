// -*- C++ -*-
/**
 * \file QParagraph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QPARAGRAPH_H
#define QPARAGRAPH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"
#include <vector>

class ControlParagraph;
class QParagraphDialog;


class QParagraph
	: public Qt2CB<ControlParagraph, Qt2DB<QParagraphDialog> >
{
public:
	friend class QParagraphDialog;

	QParagraph();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	std::vector<string> units_;
};

#endif // QPARAGRAPH_H
