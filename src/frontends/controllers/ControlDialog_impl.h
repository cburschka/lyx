// -*- C++ -*-
/**
 * \file ControlDialog_impl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
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


class ControlDialogBD : public ControlDialog<ControlConnectBD> {
public:
	///
	ControlDialogBD(LyXView &, Dialogs &);
};


class ControlDialogBI : public ControlDialog<ControlConnectBI> {
public:
	///
	ControlDialogBI(LyXView &, Dialogs &);
};


#endif // CONTROLDIALOG_IMPL_H
