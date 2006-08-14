// -*- C++ -*-
/**
 * \file BulletsModule.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBULLETSMODULE_H
#define QBULLETSMODULE_H


#include "ui/BulletsUi.h"
#include "Bullet.h"
#include <boost/array.hpp>

#include <QWidget>

class BulletsModule : public QWidget, public Ui::BulletsUi {
	Q_OBJECT
public:

	BulletsModule(QWidget * parent = 0, const char * name = 0, Qt::WFlags fl = 0);

	~BulletsModule();

	/// set a bullet
	void setBullet(int level, Bullet const & bullet);
	/// get bullet setting
	Bullet const & getBullet(int level) const;
	/// update 1st level
	void init();

Q_SIGNALS:
	void changed();

protected Q_SLOTS:

	void on_bulletsizeCO_activated(int level);
	void on_customCB_toggled(bool);
	void on_customLE_textEdited(const QString &);
	void bulletSelected(QListWidgetItem *, QListWidgetItem*);
	void showLevel(int);

private:
	void selectItem(int font, int character, bool select);
	void setupPanel(QListWidget * lw, QString panelname, std::string fname);

	/// store results
	boost::array<Bullet, 4> bullets_;
	int current_font_;
	int current_char_;
};

#endif // BULLETSMODULE_H
