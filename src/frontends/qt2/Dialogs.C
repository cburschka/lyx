/**
 * \file qt2/Dialogs.C
 * Copyright 1995 Matthias Ettrich
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs_impl.h"


Dialogs::Dialogs(LyXView & lv)
	: pimpl_(new Impl(lv, *this))
{
	// reduce the number of connections needed in
	// dialogs by a simple connection here.
	hideAll.connect(hideBufferDependent);
}


Dialogs::~Dialogs()
{}


void Dialogs::toggleTooltips()
{}


/// Are the tooltips on or off?
bool Dialogs::tooltipsEnabled()
{
	return false;
}


Dialogs::Impl::Impl(LyXView & lv, Dialogs & d)
	: aboutlyx(lv, d),
	  bibitem(lv, d),
	  bibtex(lv, d),
	  character(lv, d),
	  citation(lv, d),
	  document(lv, d),
	  error(lv, d),
	  ert(lv, d),
	  external(lv, d),
	  file(lv, d),
	  floats(lv, d),
	  graphics(lv, d),
	  include(lv, d),
	  index(lv, d),
	  logfile(lv, d),
	  minipage(lv, d),
	  paragraph(lv, d),
	  preamble(lv, d),
	  print(lv, d),
	  ref(lv, d),
	  search(lv, d),
	  sendto(lv, d),
	  spellchecker(lv, d),
	  tabularcreate(lv, d),
	  texinfo(lv, d),

#ifdef HAVE_LIBAIKSAURUS
	  thesaurus(lv, d),
#endif

	  toc(lv, d),
	  url(lv, d),
	  vclogfile(lv, d),
	  wrap(lv, d)
{}
