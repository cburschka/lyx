// -*- C++ -*-
/**
 * \file BulletsModule.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QBULLETSMODULE_H
#define QBULLETSMODULE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ui/BulletsModuleBase.h"
#include "LString.h"

class QBrowseBox;

class BulletsModule : public BulletsModuleBase {
	Q_OBJECT
public:
	BulletsModule(QWidget* parent = 0, const char* name = 0,
		      WFlags fl = 0);
	~BulletsModule();
public slots:
   void checkThis(int,int);
   void setLevel1();
private:
	QBrowseBox * standard;
	QBrowseBox * maths;
	QBrowseBox * ding1;
	QBrowseBox * ding2;
	QBrowseBox * ding3;
	QBrowseBox * ding4;
};

#endif // BULLETSMODULE_H
