// -*- C++ -*-
/**
 * \file Dialogs_impl.h
 * Copyright 1995-2002 The LyX Team.
 * See the file COPYING.
 * \author Angus Leeming <leeming@lyx.org>
 */

#ifndef DIALOGS_IMPL_H
#define DIALOGS_IMP_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Dialogs.h"
#include "GUI.h"

#include <boost/scoped_ptr.hpp>


class ControlAboutlyx;
class ControlBibitem;
class ControlBibtex;
class ControlCharacter;
class ControlCitation;
class ControlError;
class ControlERT;
class ControlExternal;
class ControlShowFile;
class ControlFloat;
class ControlForks;
class ControlGraphics;
class ControlInclude;
class ControlIndex;
class ControlLog;
class ControlMinipage;
class ControlParagraph;
class ControlPreamble;
class ControlPrint;
class ControlRef;
class ControlSearch;
class ControlSendto;
class ControlSpellchecker;
class ControlTabularCreate;
class ControlTexinfo;
class ControlToc;
class ControlUrl;
class ControlVCLog;

class FormAboutlyx;
class FormBibitem;
class FormBibtex;
class FormCharacter;
class FormCitation;
class FormDocument;
class FormError;
class FormERT;
class FormExternal;
class FormShowFile;
class FormFloat;
class FormForks;
class FormGraphics;
class FormInclude;
class FormIndex;
class FormLog;
class FormMathsPanel;
class FormMinipage;
class FormParagraph;
class FormPreamble;
class FormPreferences;
class FormPrint;
class FormRef;
class FormSearch;
class FormSendto;
class FormSpellchecker;
class FormTabular;
class FormTabularCreate;
class FormTexinfo;
class FormToc;
class FormUrl;
class FormVCLog;

#ifdef HAVE_LIBAIKSAURUS
class ControlThesaurus;
class FormThesaurus;
#endif

class OkCancelPolicy;
class OkApplyCancelPolicy;
class OkCancelReadOnlyPolicy;
class OkApplyCancelReadOnlyPolicy;
class NoRepeatedApplyReadOnlyPolicy;
class xformsBC;


typedef GUI<ControlAboutlyx, FormAboutlyx, OkCancelPolicy, xformsBC>
AboutlyxDialog;

typedef GUI<ControlBibitem, FormBibitem, OkCancelReadOnlyPolicy, xformsBC>
BibitemDialog;

typedef GUI<ControlBibtex, FormBibtex, OkCancelReadOnlyPolicy, xformsBC>
BibtexDialog;

typedef GUI<ControlCharacter, FormCharacter,
	OkApplyCancelReadOnlyPolicy, xformsBC>
CharacterDialog;

typedef GUI<ControlCitation, FormCitation,
	NoRepeatedApplyReadOnlyPolicy, xformsBC>
CitationDialog;

typedef FormDocument DocumentDialog;

typedef GUI<ControlError, FormError, OkCancelPolicy, xformsBC>
ErrorDialog;

typedef GUI<ControlERT, FormERT, NoRepeatedApplyReadOnlyPolicy, xformsBC>
ERTDialog;

typedef GUI<ControlExternal, FormExternal,
	OkApplyCancelReadOnlyPolicy, xformsBC>
ExternalDialog;

typedef GUI<ControlShowFile, FormShowFile, OkCancelPolicy, xformsBC>
FileDialog;

typedef GUI<ControlFloat, FormFloat, NoRepeatedApplyReadOnlyPolicy, xformsBC>
FloatDialog;

typedef GUI<ControlForks, FormForks, OkApplyCancelPolicy, xformsBC>
ForksDialog;

typedef GUI<ControlGraphics, FormGraphics,
	NoRepeatedApplyReadOnlyPolicy, xformsBC>
GraphicsDialog;

typedef GUI<ControlInclude, FormInclude, OkCancelReadOnlyPolicy, xformsBC>
IncludeDialog;

typedef GUI<ControlIndex, FormIndex, NoRepeatedApplyReadOnlyPolicy, xformsBC>
IndexDialog;

typedef GUI<ControlLog, FormLog, OkCancelPolicy, xformsBC>
LogFileDialog;

typedef FormMathsPanel MathPanelDialog;

typedef GUI<ControlMinipage, FormMinipage,
	NoRepeatedApplyReadOnlyPolicy, xformsBC>
MinipageDialog;

typedef GUI<ControlParagraph, FormParagraph,
	OkApplyCancelReadOnlyPolicy, xformsBC>
ParagraphDialog;

typedef GUI<ControlPreamble, FormPreamble,
	NoRepeatedApplyReadOnlyPolicy, xformsBC>
PreambleDialog;

typedef FormPreferences PreferencesDialog;

typedef GUI<ControlPrint, FormPrint, OkApplyCancelPolicy, xformsBC>
PrintDialog;

typedef GUI<ControlRef, FormRef, NoRepeatedApplyReadOnlyPolicy, xformsBC>
RefDialog;

typedef GUI<ControlSearch, FormSearch, NoRepeatedApplyReadOnlyPolicy, xformsBC>
SearchDialog;

typedef GUI<ControlSendto, FormSendto, OkApplyCancelPolicy, xformsBC>
SendtoDialog;

typedef GUI<ControlSpellchecker, FormSpellchecker,
	NoRepeatedApplyReadOnlyPolicy, xformsBC>
SpellcheckerDialog;

typedef FormTabular TabularDialog;

typedef GUI<ControlTabularCreate, FormTabularCreate,
	OkApplyCancelReadOnlyPolicy, xformsBC>
TabularCreateDialog;

typedef GUI<ControlTexinfo, FormTexinfo, OkCancelPolicy, xformsBC>
TexinfoDialog;

#ifdef HAVE_LIBAIKSAURUS
typedef GUI<ControlThesaurus, FormThesaurus,
	OkApplyCancelReadOnlyPolicy, xformsBC>
ThesaurusDialog;
#endif

typedef GUI<ControlToc, FormToc, OkCancelPolicy, xformsBC>
TocDialog;

typedef GUI<ControlUrl, FormUrl, NoRepeatedApplyReadOnlyPolicy, xformsBC>
UrlDialog;

typedef GUI<ControlVCLog, FormVCLog, OkCancelPolicy, xformsBC>
VCLogFileDialog;

struct Dialogs::Impl {
	Impl(LyXView & lv, Dialogs & d) : lv_(lv), d_(d) {}

	template <typename T>
	T & dialog(boost::scoped_ptr<T> & var)
	{
		if (!var.get())
			var.reset(new T(lv_, d_));
		return *var;
	}
	
        boost::scoped_ptr<AboutlyxDialog>      aboutlyx;
	boost::scoped_ptr<BibitemDialog>       bibitem;
        boost::scoped_ptr<BibtexDialog>        bibtex;
        boost::scoped_ptr<CharacterDialog>     character;
	boost::scoped_ptr<CitationDialog>      citation;
	boost::scoped_ptr<DocumentDialog>      document;
        boost::scoped_ptr<ErrorDialog>         error;
	boost::scoped_ptr<ERTDialog>           ert;
        boost::scoped_ptr<ExternalDialog>      external;
	boost::scoped_ptr<FileDialog>          file;
        boost::scoped_ptr<FloatDialog>         floats;
	boost::scoped_ptr<ForksDialog>         forks;
        boost::scoped_ptr<GraphicsDialog>      graphics;
	boost::scoped_ptr<IncludeDialog>       include;
        boost::scoped_ptr<IndexDialog>         index;
	boost::scoped_ptr<LogFileDialog>       logfile;
	boost::scoped_ptr<MathPanelDialog>     mathpanel;
	boost::scoped_ptr<MinipageDialog>      minipage;
	boost::scoped_ptr<ParagraphDialog>     paragraph;
	boost::scoped_ptr<PreambleDialog>      preamble;
	boost::scoped_ptr<PreferencesDialog>   preferences;
	boost::scoped_ptr<PrintDialog>         print;
	boost::scoped_ptr<RefDialog>           ref;
	boost::scoped_ptr<SearchDialog>        search;
	boost::scoped_ptr<SendtoDialog>        sendto;
	boost::scoped_ptr<SpellcheckerDialog>  spellchecker;
	boost::scoped_ptr<TabularDialog>       tabular;
	boost::scoped_ptr<TabularCreateDialog> tabularcreate;
	boost::scoped_ptr<TexinfoDialog>       texinfo;

#ifdef HAVE_LIBAIKSAURUS
	boost::scoped_ptr<ThesaurusDialog>     thesaurus;
#endif

	boost::scoped_ptr<TocDialog>           toc;
	boost::scoped_ptr<UrlDialog>           url;
	boost::scoped_ptr<VCLogFileDialog>     vclogfile;

private:
	LyXView & lv_;
	Dialogs & d_;
};

#endif // DIALOGS_IMPL_H
