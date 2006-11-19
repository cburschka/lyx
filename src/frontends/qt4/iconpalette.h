// -*- C++ -*-
/**
 * \file iconpalette.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ICONPALETTE_H
#define ICONPALETTE_H

#include <QWidget>

#include <string>
#include <utility>
#include <vector>

class QPushButton;

namespace lyx {
namespace frontend {

/**
 * For holding an arbitrary set of icons.
 */
class IconPalette : public QWidget {
	Q_OBJECT
public:
	IconPalette(QWidget * parent, char const ** entries);

Q_SIGNALS:
	void button_clicked(const std::string &);

protected Q_SLOTS:
	virtual void clicked();

private:
	typedef std::pair<QPushButton *, std::string> Button;
	std::vector<Button> buttons_;
};


} // namespace frontend
} // namespace lyx

#endif // ICONPALETTE_H
