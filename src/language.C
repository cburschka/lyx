/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "language.h"
#include "gettext.h"
#include "encoding.h"
#include "lyxrc.h"
#include "lyx_gui_misc.h"

Languages languages;
Language const * default_language;
Language ignore_lang("ignore", "ignore", "Ignore", false, 0, "ignore");
Language const * ignore_language = &ignore_lang;

///
class LangInit {
public:
	///
	LangInit() {
		if (!init) initL();
		init = true;
	}
private:
	///
	void initL();
	///
	static bool init;
};


struct lang_item {
	char const * lang;
	char const * babel;
	char const * display;
	bool rtl;
	Encoding const * encoding;
	char const * code;
};


void LangInit::initL()
{
	// Use this style of initialization to lower compilation times.
	// Same method is used in LyXAction.C (Lgb)

	lang_item items[] = {
	{ "afrikaans", "afrikaans", N_("Afrikaans"), false, &iso8859_1, "af_ZA" },
	{ "american", "american", N_("American"), false, &iso8859_1, "en_US" },
	{ "arabic", "arabic", N_("Arabic"), true, &iso8859_6, "ar_SA" },
	{ "austrian", "austrian", N_("Austrian"), false, &iso8859_1, "de_AU" },
	{ "bahasa", "bahasa", N_("Bahasa"), false, &iso8859_1, "in_ID" },
	{ "brazil", "brazil", N_("Brazil"), false, &iso8859_1, "pt_BR" },
	{ "breton", "breton", N_("Breton"), false, &iso8859_1, "br_FR" },
	{ "british", "british", N_("British"), false, &iso8859_1, "en_GB" },
	{ "canadian", "canadian", N_("Canadian"), false, &iso8859_1, "en_CA" },
	{ "canadien", "frenchb", N_("French Canadian"), false, &iso8859_1, "fr_CA" },
	{ "catalan", "catalan", N_("Catalan"), false, &iso8859_1, "ca_ES" },
	{ "croatian", "croatian", N_("Croatian"), false, &iso8859_2, "hr" },
	{ "czech", "czech", N_("Czech"), false, &iso8859_2, "cs_CZ" },
	{ "danish", "danish", N_("Danish"), false, &iso8859_1, "da_DK" },
#ifdef DO_USE_DEFAULT_LANGUAGE
	{ "default", "default", N_("Document wide language"), false, &iso8859_1, "" },
#endif
	{ "dutch", "dutch", N_("Dutch"), false, &iso8859_1, "nl" },
	{ "english", "english", N_("English"), false, &iso8859_1, "en" },
	{ "esperanto", "esperanto", N_("Esperanto"), false, &iso8859_3, "eo" },
	// and what country code should esperanto have?? (Garst)
	{ "estonian", "estonian", N_("Estonian"), false, &iso8859_4, "et_EE" },
	{ "finnish", "finnish", N_("Finnish"), false, &iso8859_1, "fi" },
	{ "frenchb", "frenchb", N_("French"), false, &iso8859_1, "fr" },
	{ "french", "french", N_("French (GUTenberg)"), false, &iso8859_1, "fr" },
	{ "galician", "galician", N_("Galician"), false, &iso8859_1, "gl_ES" },
	/*There are two Galicia's, one in Spain, one in E.Europe. Because of
	the font encoding, I am assuming this is the one in Spain. (Garst)
	*/
	{ "german", "german", N_("German"), false, &iso8859_1, "de" },
	{ "greek", "greek", N_("Greek"), false, &iso8859_7, "el_GR" },
	{ "hebrew", "hebrew", N_("Hebrew"), true, &cp1255, "he_IL" },
	/* According to Zrubecz Laszlo <zrubi@k2.jozsef.kando.hu>,
	   "magyar" is better. I kept it here in case we want
	   to  provide aliasing of languages. (JMarc) 
	*/
	//{ "hungarian", "hungarian", N_("Hungarian"), false, &iso8859_2, "" },
	{ "irish", "irish", N_("Irish"), false, &iso8859_1, "ga_IE" },
	{ "italian", "italian", N_("Italian"), false, &iso8859_1, "it" },
	{ "lsorbian", "lsorbian", N_("Lsorbian"), false, &iso8859_2, "" },
	// no ISO listing for lsorbian (Garst)
	{ "magyar", "magyar", N_("Magyar"), false, &iso8859_2, "hu" },
	{ "norsk", "norsk", N_("Norsk"), false, &iso8859_1, "no" },
	{ "polish", "polish", N_("Polish"), false, &iso8859_2, "pl" },
	{ "portuges", "portuges", N_("Portuges"), false, &iso8859_1, "pt" },
	{ "romanian", "romanian", N_("Romanian"), false, &iso8859_2, "ro" },
	{ "russian", "russian", N_("Russian"), false, &koi8, "ru" },
	{ "scottish", "scottish", N_("Scottish"), false, &iso8859_1, "gd_GB" },
	{ "spanish", "spanish", N_("Spanish"), false, &iso8859_1, "es" },
	{ "slovak", "slovak", N_("Slovak"), false, &iso8859_2, "sk_SL" },
	{ "slovene", "slovene", N_("Slovene"), false, &iso8859_2, "sl_SI" },
	{ "swedish", "swedish", N_("Swedish"), false, &iso8859_1, "sv_SE" },
	{ "turkish", "turkish", N_("Turkish"), false, &iso8859_9, "tr" },
	{ "usorbian", "usorbian", N_("Usorbian"), false, &iso8859_2, "" },
	// no ISO listing for usorbian (Garst)
	{ "welsh", "welsh", N_("Welsh"), false, &iso8859_1, "cy_GB" },
	{ 0, 0, 0, false, 0, 0 }
	};

	int i = 0;
	while (items[i].lang) {
		languages[items[i].lang] =
			Language(items[i].lang, items[i].babel,
				 items[i].display, items[i].rtl,
				 items[i].encoding, items[i].code);
		++i;
	}
	if (languages.find(lyxrc.default_language) == languages.end()) {
	    string l1 = _("Default language \"") + lyxrc.default_language +
		_("\" not found!");
	    WriteAlert(l1.c_str(), _("Using \"english\" instead!"),"");
	    default_language = &languages["english"];
	} else
	    default_language = &languages[lyxrc.default_language];
}


static
LangInit langinit;

bool LangInit::init = false;
