
#include <config.h>

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION  
#include "insetbib.h"
#include "combox.h"
#include "buffer.h"
#include "debug.h"
#include "lyx_gui_misc.h"
#include "BufferView.h"
#include "gettext.h"
#include "bibforms.h"
#include "lyxtext.h"
#include "support/filetools.h"

extern BufferView *current_view;

FD_citation_form *citation_form=0;
FD_bibitem_form *bibitem_form=0;
static Combox *bibcombox = 0;

extern void UpdateInset(Inset* inset, bool mark_dirty = true);
void BibitemUpdate(Combox *);
FD_citation_form * create_form_citation_form(void);
FD_bibitem_form * create_form_bibitem_form(void);


extern "C" void bibitem_cb(FL_OBJECT *, long data)
{
	switch (data) {
//       case 0: fl_hide_form(citation_form->citation_form);
//               break;
//       case 1: 
	case 1: // OK, citation
        {
//               InsetCommand *inset = (InsetCommand*)citation_form->vdata;
//               inset->setContents(bibcombox->getline());
//               inset->setOptions(fl_get_input(citation_form->label));
//               fl_hide_form(citation_form->citation_form);
//               UpdateInset(inset);
//               break;
		if(!current_view->currentBuffer()->isReadonly()) {
			InsetCommand *inset = (InsetCommand*)citation_form->vdata;
			inset->setContents(bibcombox->getline());
			inset->setOptions(fl_get_input(citation_form->label));
			fl_hide_form(citation_form->citation_form);
			// shouldn't mark the buffer dirty unless something
			// was actually altered
			UpdateInset(inset);
			break;
		}
		// fall through to Cancel on RO-mode
        }       
//       case 2:  fl_hide_form(bibitem_form->bibitem_form);
	case 0: fl_hide_form(citation_form->citation_form);
                break;
//       case 3:

	case 3: // OK, bibitem
        {
//               InsetCommand *inset = (InsetCommand *)bibitem_form->vdata;
//               inset->setContents(fl_get_input(bibitem_form->key));
//               inset->setOptions(fl_get_input(bibitem_form->label));
//               fl_hide_form(bibitem_form->bibitem_form);
//              // Does look like a hack? It is! (but will change at 0.13)
//               current_view->currentBuffer()->text->RedoParagraph();
//               current_view->currentBuffer()->update(1);
//               break;
		if(!current_view->currentBuffer()->isReadonly()) {
			InsetCommand *inset = (InsetCommand *)bibitem_form->vdata;
			inset->setContents(fl_get_input(bibitem_form->key));
			inset->setOptions(fl_get_input(bibitem_form->label));
			fl_hide_form(bibitem_form->bibitem_form);
			// Does look like a hack? It is! (but will change at 0.13)
			current_view->currentBuffer()->text->RedoParagraph();
			current_view->currentBuffer()->update(1);
			break;
		} // fall through to Cancel on RO-mode
        }
	case 2: // Cancel, bibitem
		fl_hide_form(bibitem_form->bibitem_form); // Cancel, bibitem
		break;
        }
}

FD_citation_form *create_form_citation_form(void)
{
	FL_OBJECT *obj;
	FD_citation_form *fdui = (FD_citation_form *) fl_calloc(1, sizeof(FD_citation_form));

	fdui->citation_form = fl_bgn_form(FL_NO_BOX, 220, 130);
	obj = fl_add_box(FL_UP_BOX,0,0,220,130,"");
	fdui->key = obj = fl_add_text(FL_NORMAL_TEXT,20,10,60,30,_("Key:"));
	  fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	  fl_set_object_lalign(obj,FL_ALIGN_RIGHT);

	bibcombox = new Combox(FL_COMBOX_INPUT);
	bibcombox->add(80,10,130,30, 120);

	obj = fl_add_button(FL_RETURN_BUTTON,20,90,90,30,_("OK"));
	  fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	  fl_set_object_callback(obj,bibitem_cb,1);
	obj = fl_add_button(FL_NORMAL_BUTTON,120,90,90,30,idex(_("Cancel|^[")));
	  fl_set_button_shortcut(obj,scex(_("Cancel|^[")),1);
	  fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	  fl_set_object_callback(obj,bibitem_cb,0);
	fdui->label = obj = fl_add_input(FL_NORMAL_INPUT,80,50,130,30,idex(_("Remark:|#R")));
	  fl_set_input_shortcut(obj,scex(_("Remark:|#R")),1);
	  fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	fl_end_form();

	//fdui->citation_form->fdui = fdui;

	return fdui;
}
/*---------------------------------------*/

FD_bibitem_form *create_form_bibitem_form(void)
{
	FL_OBJECT *obj;
	FD_bibitem_form *fdui = (FD_bibitem_form *) fl_calloc(1, sizeof(FD_bibitem_form));

	fdui->bibitem_form = fl_bgn_form(FL_NO_BOX, 220, 130);
	obj = fl_add_box(FL_UP_BOX,0,0,220,130,"");
	fdui->key = obj = fl_add_input(FL_NORMAL_INPUT,80,10,130,30,idex(_("Key:|#K")));
	  fl_set_input_shortcut(obj,scex(_("Key:|#K")),1);
	  fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	obj = fl_add_button(FL_RETURN_BUTTON,20,90,90,30,_("OK"));
	  fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	  fl_set_object_callback(obj,bibitem_cb,3);
	obj = fl_add_button(FL_NORMAL_BUTTON,120,90,90,30,idex(_("Cancel|^[")));
	  fl_set_button_shortcut(obj,scex(_("Cancel|^[")),1);
	  fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	  fl_set_object_callback(obj,bibitem_cb,2);
	fdui->label = obj = fl_add_input(FL_NORMAL_INPUT,80,50,130,30,idex(_("Label:|#L")));
	  fl_set_input_shortcut(obj,scex(_("Label:|#L")),1);
	  fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	fl_end_form();

	//fdui->bibitem_form->fdui = fdui;

	return fdui;
}
/*---------------------------------------*/


InsetCitation::InsetCitation(string const & key, string const & note):
	InsetCommand("cite", key, note)
{

}
InsetCitation::~InsetCitation()
{
	if(citation_form && citation_form->citation_form
	   && citation_form->citation_form->visible
	   && citation_form->vdata == this)
		fl_hide_form(citation_form->citation_form);
}


void InsetCitation::Edit(int, int)
{
	if(current_view->currentBuffer()->isReadonly())
		WarnReadonly();

	if (!citation_form) {
		citation_form = create_form_citation_form();
		fl_set_form_atclose(citation_form->citation_form, 
				    CancelCloseBoxCB, 0);
	}
	citation_form->vdata = this;

	BibitemUpdate(bibcombox);
	if (!bibcombox->select_text(getContents().c_str()))
		bibcombox->addline(getContents().c_str());
	    
	fl_set_input(citation_form->label, getOptions().c_str());
	if (citation_form->citation_form->visible) {
		fl_raise_form(citation_form->citation_form);
	} else {
		fl_show_form(citation_form->citation_form,FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Citation"));
	}   
}


string InsetCitation::getScreenLabel() const
{
	string temp("[");

	temp += contents;

	if (!options.empty()) {
		temp += "," + options;
	}

	return temp + ']';
}


InsetBibKey::InsetBibKey(string const & key, string const & label):
	InsetCommand("bibitem", key, label)
{
	counter = 1;
	if (key.empty())
		contents = ' ';
}


InsetBibKey::InsetBibKey(InsetBibKey const *b):
	InsetCommand("bibitem", b->contents, b->options)
{
	counter = b->counter;
}

InsetBibKey::~InsetBibKey()
{
	if(bibitem_form && bibitem_form->bibitem_form
	   && bibitem_form->bibitem_form->visible)
				fl_hide_form(bibitem_form->bibitem_form);
}

void InsetBibKey::setCounter(int c) 
{ 
	counter = c; 
    
	if (contents.empty())
		contents += tostr(counter);
}


// I'm sorry but this is still necessary because \bibitem is used also
// as a LyX 2.x command, and lyxlex is not enough smart to understand
// real LaTeX commands. Yes, that could be fixed, but would be a waste 
// of time cause LyX3 won't use lyxlex anyway.  (ale)
void InsetBibKey::Write(FILE *file)
{
	string s;
	if (!options.empty()) {
		s += '[';
		s += options + ']';
	}
	s += '{';
	s += contents + '}';
	fprintf(file,"\\bibitem %s\n", s.c_str());
}


string InsetBibKey::getScreenLabel() const
{
	if (!options.empty())
		return options;
    
	return tostr(counter);
}


/*
  The value in "Key:" isn't allways set right after a few bibkey insets have
  been added/removed.  Perhaps the wrong object is deleted/used somewhere upwards?
  (Joacim 1998-03-04)
*/
void InsetBibKey::Edit(int, int)
{
	if(current_view->currentBuffer()->isReadonly())
		WarnReadonly();
	
	if (!bibitem_form) {
		bibitem_form = create_form_bibitem_form();
		fl_set_form_atclose(bibitem_form->bibitem_form, 
				    CancelCloseBoxCB, 0);
	}
	bibitem_form->vdata = this;
	// InsetBibtex uses the same form, with different labels
	fl_set_object_label(bibitem_form->key, idex(_("Key:|#K")));
	fl_set_button_shortcut(bibitem_form->key,scex(_("Key:|#K")),1);
	fl_set_object_label(bibitem_form->label, idex(_("Label:|#L")));
	fl_set_button_shortcut(bibitem_form->label,scex(_("Label:|#L")),1);
	fl_set_input(bibitem_form->key, getContents().c_str());
	fl_set_input(bibitem_form->label, getOptions().c_str());
	if (bibitem_form->bibitem_form->visible) {
		fl_raise_form(bibitem_form->bibitem_form);
	} else {
		fl_show_form(bibitem_form->bibitem_form,FL_PLACE_MOUSE,
			     FL_FULLBORDER,
			     _("Bibliography item"));
	}   
}



InsetBibtex::InsetBibtex(string const & dbase, string const & style,
			 Buffer *o)
	:InsetCommand("BibTeX", dbase, style), owner(o)
{
}


InsetBibtex::~InsetBibtex()
{
}


string InsetBibtex::getScreenLabel() const
{
	return _("BibTeX Generated References");
}


int InsetBibtex::Latex(FILE *file, signed char /*fragile*/)
{
	string bib;
	signed char dummy = 0;
	int result = Latex(bib, dummy);
	fprintf(file, "%s", bib.c_str());
	return result;
}


int InsetBibtex::Latex(string &file, signed char /*fragile*/)
{
	// this looks like an horrible hack and it is :) The problem
	// is that owner is not initialized correctly when the bib
	// inset is cut and pasted. Such hacks will not be needed
	// later (JMarc)
	if (!owner) {
		owner = current_view->currentBuffer();
	}
	// If we generate in a temp dir, we might need to give an
	// absolute path there. This is a bit complicated since we can
	// have a comma-separated list of bibliographies
	string db_in, adb, db_out;
	db_in = getContents();
	db_in=split(db_in, adb, ',');
	while(!adb.empty()) {
		if (!owner->niceFile &&
		    IsFileReadable(MakeAbsPath(adb,owner->filepath)+".bib")) 
			adb = MakeAbsPath(adb,owner->filepath);
		db_out += adb;
		db_out += ',';
		db_in=split(db_in, adb,',');
	}
	db_out = strip(db_out, ',');
	// Idem, but simpler
	string style;
	if (!owner->niceFile 
	    && IsFileReadable(MakeAbsPath(getOptions(), owner->filepath)
			      + ".bst")) 
		style = MakeAbsPath(getOptions(), owner->filepath);
	else
		style = getOptions();

	file += "\\bibliographystyle{";
	file += style;
	file += "}\n";
	file += "\\bibliography{";
	file += db_out;
	file += "}\n";
	return 2;
}

// This method returns a comma separated list of Bibtex entries
string InsetBibtex::getKeys()
{
	// This hack is copied from InsetBibtex::Latex.
	// Is it still needed? Probably yes.
	if (!owner) {
		owner = current_view->currentBuffer();
	}

	// We need to create absolute path names for bibliographies
	// First look for bib-file in same directory as document,
	// then in all directories listed in environment variable 
	// BIBINPUTS
	string bibfiles, linebuf, tmp, keys;
	bibfiles = getContents();
	bibfiles=split(bibfiles, tmp, ',');
	while(!tmp.empty()) {
		if (IsFileReadable(MakeAbsPath(tmp,owner->filepath)+".bib"))
			tmp = MakeAbsPath(tmp,owner->filepath)+".bib";
		else {
			tmp = FileOpenSearch(GetEnvPath("BIBINPUTS"),tmp,"bib");
			if (tmp.empty())
				tmp = FileOpenSearch(GetEnvPath("BIBINPUT"),
						     tmp, "bib");
		}
		// If we didn't find a matching file name just fail silently
		if (!tmp.empty()) {
      
			// This is a _very_ simple parser for Bibtex database files.
			// All it does is to look for lines starting in @ and not
 			// being @preamble and @string entries.
			// It does NOT do any syntax checking!
			FilePtr file(tmp,FilePtr::read);
 			char c;

			// On some systems where feof() is a macro,
			// the () after file is needed (JMarc)
			while (! feof(file())) {
				c = fgetc(file);

				// At end of each line check if line begins with '@'
				if ( c == '\n') {
					if (prefixIs(linebuf, "@") ) {
						linebuf = subst(linebuf,
								'{', '(');
						linebuf = split(linebuf,
								tmp, '(');
						tmp = lowercase(tmp);
	    					if (!prefixIs(tmp, "@string") && !prefixIs(tmp, "@preamble") ) {
							linebuf = split(linebuf, tmp,',');
							if (!tmp.empty())
								keys += strip(tmp) + ",";
						}
					}
					linebuf.clear();
				} else {
					linebuf += c;
				}
			}
		}
		// Get next file name
    		bibfiles=split(bibfiles, tmp, ',');
	}
  	return keys;
}

// BibTeX should have its own dialog. This is provisional.
void InsetBibtex::Edit(int, int)
{
	if (!bibitem_form) {
		bibitem_form = create_form_bibitem_form();
		fl_set_form_atclose(bibitem_form->bibitem_form, 
				    CancelCloseBoxCB, 0);
	}

	bibitem_form->vdata = this;
	fl_set_object_label(bibitem_form->key, _("Database:"));
	fl_set_object_label(bibitem_form->label, _("Style:  "));
	fl_set_input(bibitem_form->key, getContents().c_str());
	fl_set_input(bibitem_form->label, getOptions().c_str());
	if (bibitem_form->bibitem_form->visible) {
		fl_raise_form(bibitem_form->bibitem_form);
	} else {
		fl_show_form(bibitem_form->bibitem_form,FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("BibTeX"));
	}   
}


bool InsetBibtex::addDatabase(string const &db)
{
	if (!contains(contents, db.c_str())) {
		if (!contents.empty()) 
			contents += ',';
		contents += db;
		return true;
	}
	return false;
}


bool InsetBibtex::delDatabase(string const &db)
{
	if (contains(contents, db.c_str())) {
		string bd = db;
		int n = tokenPos(contents, ',', bd);
		if (n > 0) {
			// Weird code, would someone care to explain this?(Lgb)
			string tmp(",");
			tmp += bd;
			contents = subst(contents, tmp.c_str(), ",");
		} else if (n==0)
			contents = split(contents, bd, ',');
		else 
			return false;
	}
	return true;
}


// This function should be in LyXView when multiframe works ale970302
void BibitemUpdate(Combox* combox)
{
	combox->clear();
    
	if (!current_view->available())
		return;

	string tmp, bibkeys = current_view->currentBuffer()->getBibkeyList(',');
	bibkeys=split(bibkeys, tmp,',');
	while (!tmp.empty()) {
	  combox->addto(tmp.c_str());
	  bibkeys=split(bibkeys, tmp,',');
	}
}



// ale070405 This function maybe shouldn't be here. We'll fix this at 0.13.
int bibitemMaxWidth(const class LyXFont &font)
{
	int w = 0;
	// Does look like a hack? It is! (but will change at 0.13)
	LyXParagraph *par = current_view->currentBuffer()->paragraph;
    
	while (par) {
		if (par->bibkey) {
			int wx = par->bibkey->Width(font);
			if (wx>w) w = wx;
		}
		par = par->next;
	}
	return w;
}


// ale070405 
string bibitemWidthest()
{
	int w = 0;
	// Does look like a hack? It is! (but will change at 0.13)
	LyXParagraph *par = current_view->currentBuffer()->paragraph;
	InsetBibKey *bkey=0;
	LyXFont font;
      
	while (par) {
		if (par->bibkey) {
			int wx = par->bibkey->Width(font);
			if (wx>w) {
				w = wx;
				bkey = par->bibkey;
			}
		}
		par = par->next;
	}
    
	if (bkey && !bkey->getScreenLabel().empty())
		return bkey->getScreenLabel();
    
	return "99";
}
