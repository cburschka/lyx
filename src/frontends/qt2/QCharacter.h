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
 

class ControlCharacter;
class QCharacterDialog;

class QCharacter :
	public Qt2CB<ControlCharacter, Qt2DB<QCharacterDialog> > 
{
	friend class QCharacterDialog;
 
public: 
	QCharacter(ControlCharacter &);

private: 
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
 
	std::vector<character::FamilyPair> family;
	std::vector<character::SeriesPair> series;
	std::vector<character::ShapePair>  shape;
	std::vector<character::SizePair>   size;
	std::vector<character::BarPair>    bar;
	std::vector<character::ColorPair>  color;
	std::vector<string> language;
};

#endif // QCHARACTER_H
