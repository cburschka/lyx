/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *          Copyright 2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <gdk--/font.h>
#include <gdk/gdktypes.h>
#include <gdk/gdkx.h>
#include <glib.h>

#include "gnome_helpers.h"

string get_font_name(Gdk_Font const & font)
{
	gchar * name = get_font_name(font.gdkobj());
	string name_str(name);
	g_free(name);
	return name_str;
}


gchar *
get_font_name (const GdkFont * font)
{
	Atom font_atom, atom;
	bool status = false;

#ifdef E_FONT_VERBOSE
	gint i;
	g_print ("Extracting X font info\n");
#endif

	font_atom = gdk_atom_intern ("FONT", FALSE);

	if (font->type == GDK_FONT_FONTSET) {
		XFontStruct **font_structs;
		gint num_fonts;
		gchar **font_names;

		num_fonts = XFontsOfFontSet (
				static_cast<XFontSet>(GDK_FONT_XFONT (font))
				, &font_structs, &font_names);
#ifdef E_FONT_VERBOSE
		g_print ("Fonts of fontset:\n");
		for (i = 0; i < num_fonts; i++) g_print ("  %s\n", font_names[i]);
#endif
		status = XGetFontProperty (font_structs[0], font_atom, &atom);
	} else {
		status = XGetFontProperty (
				static_cast<XFontStruct*>(GDK_FONT_XFONT (font))
				, font_atom, &atom);
	}

	if (status) {
		return gdk_atom_name (atom);
	}

	return NULL;
}

#if USE_UNUSED_STUFF_FROM_E_FONT
/*
 * Splits full X font name into pieces, overwriting hyphens
 */

static void
split_name (gchar * c[], gchar * name)
{
	gchar *p;
	gint i;

	p = name;
	if (*p == '-') p++;

	for (i = 0; i < 12; i++) {
		c[i] = p;
		/* Skip text */
		while (*p && (*p != '-')) p++;
		/* Replace hyphen with '\0' */
		if (*p) *p++ = '\0';
	}

	c[i] = p;
}

/*
 * Find light and bold variants of a font, ideally using the provided
 * weight for the light variant, and a weight 2 shades darker than it
 * for the bold variant. If there isn't something 2 shades darker, use
 * something 3 or more shades darker if it exists, or 1 shade darker
 * if that's all there is. If there is nothing darker than the provided
 * weight, but there are lighter fonts, then use the darker one for
 * bold and a lighter one for light.
 */

static gboolean
find_variants (gchar **namelist, gint length, gchar *weight,
	       gchar **lightname, gchar **boldname)
{
	static GHashTable *wh = NULL;
	/* Standard, Found, Bold, Light */
	gint sw, fw, bw, lw;
	gchar s[32];
	gchar *f, *b, *l;
	gchar *p;
	gint i;

	if (!wh) {
		wh = g_hash_table_new (g_str_hash, g_str_equal);
		g_hash_table_insert (wh, "light", GINT_TO_POINTER (1));
		g_hash_table_insert (wh, "book", GINT_TO_POINTER (2));
		g_hash_table_insert (wh, "regular", GINT_TO_POINTER (2));
		g_hash_table_insert (wh, "medium", GINT_TO_POINTER (3));
		g_hash_table_insert (wh, "demibold", GINT_TO_POINTER (5));
		g_hash_table_insert (wh, "bold", GINT_TO_POINTER (6));
		g_hash_table_insert (wh, "black", GINT_TO_POINTER (8));
	}

	g_snprintf (s, 32, weight);
	g_strdown (s);
	sw = GPOINTER_TO_INT (g_hash_table_lookup (wh, s));
	if (sw == 0) return FALSE;

	fw = 0; lw = 0; bw = 32;
	f = NULL; l = NULL; b = NULL;
	*lightname = NULL; *boldname = NULL;

	for (i = 0; i < length; i++) {
		p = namelist[i];
		if (*p) p++;
		while (*p && (*p != '-')) p++;
		if (*p) p++;
		while (*p && (*p != '-')) p++;
		if (*p) p++;
		f = p;
		while (*p && (*p != '-')) p++;
		if (*p) *p = '\0';
		g_strdown (f);
		fw = GPOINTER_TO_INT (g_hash_table_lookup (wh, f));
		if (fw) {
			if (fw > sw) {
				if ((fw - 2 == sw) ||
				    ((fw > bw) && (bw == sw + 1)) ||
				    ((fw < bw) && (fw - 2 > sw))) {
					bw = fw;
					b = f;
				}
			} else if (fw < sw) {
				if ((fw + 2 == sw) ||
				    ((fw < lw) && (lw == sw - 1)) ||
				    ((fw > lw) && (fw + 2 < sw))) {
					lw = fw;
					l = f;
				}
			}
		}
	}

	if (b) {
		*lightname = weight;
		*boldname = b;
		return TRUE;
	} else if (l) {
		*lightname = l;
		*boldname = weight;
		return TRUE;
	}
	return FALSE;
}
#endif

#ifdef E_FONT_VERBOSE
/*
 * Return newly allocated full name
 */

static void
e_font_print_gdk_font_name (const GdkFont * font)
{
	Atom font_atom, atom;
	Bool status;

	font_atom = gdk_atom_intern ("FONT", FALSE);

	g_print ("-------- start of font description --------\n");

	if (font == NULL) {
		g_print ("GdkFont is NULL\n");
	} else if (font->type == GDK_FONT_FONTSET) {
		XFontStruct **font_structs;
		gint num_fonts;
		gchar **font_names;
		gint i;

		num_fonts = XFontsOfFontSet (GDK_FONT_XFONT (font), &font_structs, &font_names);

		g_print ("Gdk Fontset, locale: %s\n", XLocaleOfFontSet (GDK_FONT_XFONT (font)));
		for (i = 0; i < num_fonts; i++) {
			g_print ("    %s\n", font_names[i]);
		}
	} else {
		gchar * name;
		status = XGetFontProperty (GDK_FONT_XFONT (font), font_atom, &atom);
		name = gdk_atom_name (atom);
		g_print ("GdkFont: %s\n", name);
		if (name) g_free (name);
	}

	g_print ("-------- end of font description --------\n");
}
#endif
