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
#include <vector>

class QBrowseBox;
class QLineEdit;


class BulletsModule : public BulletsModuleBase {
	Q_OBJECT
public:
	BulletsModule(QWidget* parent = 0, const char* name = 0,
		      WFlags fl = 0);
	~BulletsModule();

protected slots:
	///
	void standard(int row, int col);
	///
	void maths(int row, int col);
	///
	void ding1(int row, int col);
	///
	void ding2(int row, int col);
	///
	void ding3(int row, int col);
	///
	void ding4(int row, int col);
	///
	void setlevel1();
	///
	void setlevel2();
	///
	void setlevel3();
	///
	void setlevel4();
	
	
private:
	QLineEdit * le_;

	QBrowseBox * standard_;
	QBrowseBox * maths_;
	QBrowseBox * ding1_;
	QBrowseBox * ding2_;
	QBrowseBox * ding3_;
	QBrowseBox * ding4_;

};

#endif // BULLETSMODULE_H
