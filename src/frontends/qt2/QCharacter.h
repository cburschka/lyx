// -*- C++ -*-
/**
 * \file QCharacter.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven, leuven@fee.uva.nl
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QCHARACTER_H
#define QCHARACTER_H

#include <config.h>
#include <vector>

#include "LString.h"
#include "Qt2Base.h"
#include "controllers/character.h"
#include "controllers/frnt_lang.h"


class ControlCharacter;
class QCharacterDialog;
class Dialogs;

class QCharacter :
	public Qt2CB<ControlCharacter, Qt2DB<QCharacterDialog> >
{
	friend class QCharacterDialog;

public:
	QCharacter(ControlCharacter &, Dialogs &);

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
