// -*- C++ -*-
/**
 * \file QCharacter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCHARACTER_H
#define QCHARACTER_H


#include "QDialogView.h"
#include "controllers/character.h"
#include "controllers/frnt_lang.h"

#include <vector>


class ControlCharacter;
class QCharacterDialog;


class QCharacter
	: public QController<ControlCharacter, QView<QCharacterDialog> >
{
public:
	friend class QCharacterDialog;

	QCharacter(Dialog &);
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	std::vector<frnt::FamilyPair> family;
	std::vector<frnt::SeriesPair> series;
	std::vector<frnt::ShapePair>  shape;
	std::vector<frnt::SizePair>   size;
	std::vector<frnt::BarPair>    bar;
	std::vector<frnt::ColorPair>  color;
	std::vector<frnt::LanguagePair> language;
};

#endif // QCHARACTER_H
