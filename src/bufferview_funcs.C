#include <config.h>

#include "bufferview_funcs.h"
#include "LyXView.h"
#include "BufferView.h"
#include "lyxparagraph.h"
#include "lyxfont.h"
#include "lyx_gui_misc.h"
#include "lyxtext.h"
#include "minibuffer.h"
#include "buffer.h"
#include "support/lstrings.h"
#include "lyx_cb.h"
#include "layout_forms.h"

extern FD_form_paragraph * fd_form_paragraph;
extern FD_form_character * fd_form_character;


void Foot(BufferView * bv)
{
	if (!bv->available()) 
		return;
	
	bv->owner()->getMiniBuffer()
		->Set(_("Inserting Footnote..."));
	bv->hideCursor();
	bv->update(-2);
	bv->text->InsertFootnoteEnvironment(LyXParagraph::FOOTNOTE);
	bv->update(1);
}


void Emph(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setEmph(LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}


void Bold(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setSeries(LyXFont::BOLD_SERIES);
	ToggleAndShow(bv, font);
}


void Noun(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setNoun(LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}


void Margin(BufferView * bv)
{
	if (bv->available()) {
		bv->owner()->getMiniBuffer()->Set(_("Inserting margin note..."));
		bv->hideCursor();
		bv->update(-2);
		bv->text->InsertFootnoteEnvironment(LyXParagraph::MARGIN);
		bv->update(1);
	}
}


void Lang(BufferView * bv, string const & l)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	Languages::iterator lit = languages.find(l);
	if (lit != languages.end()) {
		font.setLanguage(&(*lit).second);
		ToggleAndShow(bv, font);
	} else
		WriteAlert(_("Error! unknown language"),l);
}


void Melt(BufferView * bv)
{
	if (!bv->available()) return;
	
	bv->owner()->getMiniBuffer()->Set(_("Melt"));
	bv->hideCursor();
	bv->beforeChange();
	bv->update(-2);
	bv->text->MeltFootnoteEnvironment();
	bv->update(1);
}


void Tex(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setLatex (LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}


// Change environment depth.
// if decInc >= 0, increment depth
// if decInc <  0, decrement depth
void changeDepth(BufferView * bv, int decInc)
{
	if (!bv->available()) return;
	
	bv->hideCursor();
	bv->update(-2);
	if (decInc >= 0)
		bv->text->IncDepth();
	else
		bv->text->DecDepth();
	bv->update(1);
	bv->owner()->getMiniBuffer()
		->Set(_("Changed environment depth"
			" (in possible range, maybe not)"));
}


void Free(BufferView * bv)
{
	ToggleAndShow(bv, UserFreeFont());
}


// How should this actually work? Should it prohibit input in all BufferViews,
// or just in the current one? If "just the current one", then it should be
// placed in BufferView. If "all BufferViews" then LyXGUI (I think) should
// run "ProhibitInput" on all LyXViews which will run prohibitInput on all
// BufferViews. Or is it perhaps just the (input in) BufferViews in the
// current LyxView that should be prohibited (Lgb) (This applies to
// "AllowInput" as well.)
void ProhibitInput(BufferView * bv)
{
	bv->hideCursor();

	static Cursor cursor;
	static bool cursor_undefined = true;
   
	if (cursor_undefined){
		cursor = XCreateFontCursor(fl_display, XC_watch);
		XFlush(fl_display);
		cursor_undefined = false;
	}
   
	/* set the cursor to the watch for all forms and the canvas */ 
	XDefineCursor(fl_display, bv->owner()->getForm()->window, 
		      cursor);
	if (fd_form_paragraph->form_paragraph->visible)
		XDefineCursor(fl_display,
			      fd_form_paragraph->form_paragraph->window,
			      cursor);
	if (fd_form_character->form_character->visible)
		XDefineCursor(fl_display,
			      fd_form_character->form_character->window,
			      cursor);

	XFlush(fl_display);
	fl_deactivate_all_forms();
}


void AllowInput(BufferView * bv)
{
	/* reset the cursor from the watch for all forms and the canvas */
   
	XUndefineCursor(fl_display, bv->owner()->getForm()->window);
	if (fd_form_paragraph->form_paragraph->visible)
		XUndefineCursor(fl_display,
				fd_form_paragraph->form_paragraph->window);
	if (fd_form_character->form_character->visible)
		XUndefineCursor(fl_display,
				fd_form_character->form_character->window);

	XFlush(fl_display);
	fl_activate_all_forms();
}


void Code(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::TYPEWRITER_FAMILY); // no good
	ToggleAndShow(bv, font);
}


void Sans(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::SANS_FAMILY);
	ToggleAndShow(bv, font);
}


void Roman(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setFamily(LyXFont::ROMAN_FAMILY);
	ToggleAndShow(bv, font);
}


void StyleReset(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_INHERIT, ignore_language);
	ToggleAndShow(bv, font);
}


void Underline(BufferView * bv)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setUnderbar(LyXFont::TOGGLE);
	ToggleAndShow(bv, font);
}


void FontSize(BufferView * bv, string const & size)
{
	LyXFont font(LyXFont::ALL_IGNORE);
	font.setGUISize(size);
	ToggleAndShow(bv, font);
}


/* -------> Returns the current font and depth by printing a message. In the
 * future perhaps we could try to implement a callback to the button-bar.
 * That is, `light' the bold button when the font is currently bold, etc.
 */
string CurrentState(BufferView * bv)
{
	string state;
	if (bv->available()) { 
		// I think we should only show changes from the default
		// font. (Asger)
		Buffer * buffer = bv->buffer();
		LyXFont font = bv->text->real_current_font;
		LyXFont const & defaultfont =
			textclasslist
			.TextClass(buffer->params.textclass)
			.defaultfont();
		font.reduce(defaultfont);
		state = _("Font: ") + font.stateText(&buffer->params);
		// The paragraph depth
		int depth = bv->text->GetDepth();
		if (depth > 0) 
			state += string(_(", Depth: ")) + tostr(depth);
		// The paragraph spacing, but only if different from
		// buffer spacing.
		if (!bv->text->cursor.par->spacing.isDefault()) {
			Spacing::Space cur_space =
				bv->text->cursor.par->spacing.getSpace();
			state += _(", Spacing: ");
			switch (cur_space) {
			case Spacing::Single:
				state += _("Single");
				break;
			case Spacing::Onehalf:
				state += _("Onehalf");
				break;
			case Spacing::Double:
				state += _("Double");
				break;
			case Spacing::Other:
				state += _("Other (");
				state += tostr(bv->text->cursor.par->spacing.getValue());
				state += ")";
				break;
			case Spacing::Default:
				// should never happen, do nothing
				break;
			}
		}
	}
	return state;
}


/* -------> Does the actual toggle job of the XxxCB() calls above.
 * Also shows the current font state.
 */
void ToggleAndShow(BufferView * bv, LyXFont const & font)
{
	if (bv->available()) { 
		bv->hideCursor();
		bv->update(-2);
		if (bv->the_locking_inset)
			bv->the_locking_inset->SetFont(bv, font, toggleall);
		else
			bv->text->ToggleFree(font, toggleall);
		bv->update(1);

		if (font.language() != ignore_language ||
		    font.latex() != LyXFont::IGNORE) {
			LyXText * text = bv->text;
			LyXCursor & cursor = text->cursor;
			text->ComputeBidiTables(cursor.row);
			if (cursor.boundary != 
			    text->IsBoundary(cursor.par, cursor.pos,
					     text->real_current_font) )
				text->SetCursor(cursor.par, cursor.pos,
						false, !cursor.boundary);
		}
	}
}
