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
#include "Bullet.h"
#include <boost/array.hpp>

class QBrowseBox;
class QListViewItem;


class BulletsModule : public BulletsModuleBase {
	Q_OBJECT
public:
	BulletsModule(QWidget* parent = 0, const char* name = 0,
		      WFlags fl = 0);
	~BulletsModule();

	///
	void setBullet(int level, const Bullet & bullet);
	///
	Bullet getBullet(int level);
	
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
	/// set custom (text) bullet
	void setCustom();
	/// make level active
	void setActive(QListViewItem*);
	/// set size
	void setSize(int size);
	
private:
	/// get the pixmap that matches font/character
	QPixmap getPixmap(int font, int character);
	/// get the qlistviewitem of level
	QListViewItem *  getItem(int level);
	/// make level active
	void setActive(int level);
	/// set bullet for standard font/character
	void setBullet(int font, int character);
	/// set bullet for custom text
	void setBullet(string text);
	/// pointer to the bullet of the level we're at
	Bullet * activebullet_;
	/// qlistviewitem of the level we're at
	QListViewItem * activeitem_;
	/// store results
	boost::array<Bullet,4> bullets_;
	
	QBrowseBox * standard_;
	QBrowseBox * maths_;
	QBrowseBox * ding1_;
	QBrowseBox * ding2_;
	QBrowseBox * ding3_;
	QBrowseBox * ding4_;
};

#endif // BULLETSMODULE_H
