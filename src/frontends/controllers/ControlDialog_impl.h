// -*- C++ -*-
/*
 * \file ControlDialog_impl.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * Instatiate the two possible instantiations of the ControlDialog template
 * class, thus reducing file dependencies enormously.
 */

#ifndef CONTROLDIALOG_IMPL_H
#define CONTROLDIALOG_IMPL_H

#include "ControlDialog.h"
#include "ControlConnections.h"

class Dialogs;
class LyXView;


class ControlDialogBD : public ControlDialog<ControlConnectBD>
{
public:
	///
	ControlDialogBD(LyXView &, Dialogs &);
};


class ControlDialogBI : public ControlDialog<ControlConnectBI>
{
public:
	///
	ControlDialogBI(LyXView &, Dialogs &);
};


#endif // CONTROLDIALOG_IMPL_H
