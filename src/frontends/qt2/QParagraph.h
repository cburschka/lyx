// -*- C++ -*-
/**
 * \file QParagraph.h
 * Copyright 2001 LyX Team
 * see the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QPARAGRAPH_H
#define QPARAGRAPH_H

#include <config.h>
#include "Qt2Base.h"

class ControlParagraph;
class QParagraphDialog;
class Dialogs;

class QParagraph :
	public Qt2CB<ControlParagraph, Qt2DB<QParagraphDialog> >
{
	friend class QParagraphDialog;

public:
	QParagraph(ControlParagraph &, Dialogs &);

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
