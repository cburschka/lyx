/**
 * \file qt2/Dialogs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "Dialogs_impl.h"


void Dialogs::init_pimpl()
{
	pimpl_ = new Impl(lyxview_, *this);
}


Dialogs::~Dialogs()
{
	delete pimpl_;
}


Dialogs::Impl::Impl(LyXView & lv, Dialogs & d)
	: character(lv, d),
	  document(lv, d),
	  file(lv, d),
	  logfile(lv, d),
	  paragraph(lv, d),
	  prefs(lv, d),
	  print(lv, d),
	  search(lv, d),
	  sendto(lv, d),
	  spellchecker(lv, d),
	  texinfo(lv, d),

#ifdef HAVE_LIBAIKSAURUS
	  thesaurus(lv, d),
#endif

	  vclogfile(lv, d)
{}
