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

Languages languages;
Language const * default_language;
Language ignore_lang("ignore", "Ignore", false, 0, "ignore");
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
	{ "afrikaans", N_("Afrikaans"), false, &iso8859_1, "af_ZA" },
	{ "american", N_("American"), false, &iso8859_1, "en_US" },
	{ "arabic", N_("Arabic"), true, &iso8859_6, "ar_SA" },
	{ "austrian", N_("Austrian"), false, &iso8859_1, "de_AU" },
	{ "bahasa", N_("Bahasa"), false, &iso8859_1, "in_ID" },
	{ "brazil", N_("Brazil"), false, &iso8859_1, "pt_BR" },
	{ "breton", N_("Breton"), false, &iso8859_1, "br_FR" },
	{ "british", N_("British"), false, &iso8859_1, "en" },
	{ "canadian", N_("Canadian"), false, &iso8859_1, "en_CA" },
	{ "catalan", N_("Catalan"), false, &iso8859_1, "ca_ES" },
	{ "croatian", N_("Croatian"), false, &iso8859_2, "hr" },
	{ "czech", N_("Czech"), false, &iso8859_2, "cs_CZ" },
	{ "danish", N_("Danish"), false, &iso8859_1, "da_DK" },
	{ "default", N_("Document wide language"), false, &iso8859_1, "" },
	{ "dutch", N_("Dutch"), false, &iso8859_1, "nl" },
	// changed from en_EN (Garst)
	{ "esperanto", N_("Esperanto"), false, &iso8859_3, "eo" },
	// and what country code should esperanto have?? (Garst)
	{ "estonian", N_("Estonian"), false, &iso8859_4, "et_EE" },
	{ "finnish", N_("Finnish"), false, &iso8859_1, "fi" },
	{ "frenchb", N_("French"), false, &iso8859_1, "fr" },
	{ "frenchc", N_("French Canadien"), false, &iso8859_1, "fr_CA" },
	{ "french", N_("French (GUTenberg)"), false, &iso8859_1, "fr" },
	{ "galician", N_("Galician"), false, &iso8859_1, "gl_ES" },
	/*There are two Galicia's, one in Spain, one in E.Europe. Because of
	the font encoding, I am assuming this is the one in Spain. (Garst)
	*/
	{ "german", N_("German"), false, &iso8859_1, "de" },
	{ "greek", N_("Greek"), false, &iso8859_7, "el_GR" },
	{ "hebrew", N_("Hebrew"), true, &cp1255, "he_IL" },
	/* According to Zrubecz Laszlo <zrubi@k2.jozsef.kando.hu>,
	   "magyar" is better. I kept it here in case we want
	   to  provide aliasing of languages. (JMarc) 
	*/
	//{ "hungarian", N_("Hungarian"), false, &iso8859_2, "" },
	{ "irish", N_("Irish"), false, &iso8859_1, "ga_IE" },
	{ "italian", N_("Italian"), false, &iso8859_1, "it" },
	{ "lsorbian", N_("Lsorbian"), false, &iso8859_2, "" },
	// no ISO listing for lsorbian (Garst)
	{ "magyar", N_("Magyar"), false, &iso8859_2, "hu" },
	{ "norsk", N_("Norsk"), false, &iso8859_1, "no" },
	{ "polish", N_("Polish"), false, &iso8859_2, "pl" },
	{ "portuges", N_("Portuges"), false, &iso8859_1, "pt" },
	{ "romanian", N_("Romanian"), false, &iso8859_2, "ro" },
	{ "russian", N_("Russian"), false, &koi8, "ru" },
	{ "scottish", N_("Scottish"), false, &iso8859_1, "gd_GB" },
	{ "spanish", N_("Spanish"), false, &iso8859_1, "es" },
	{ "slovak", N_("Slovak"), false, &iso8859_2, "sk_SL" },
	{ "slovene", N_("Slovene"), false, &iso8859_2, "sl_SI" },
	{ "swedish", N_("Swedish"), false, &iso8859_1, "sv_SE" },
	{ "turkish", N_("Turkish"), false, &iso8859_9, "tr" },
	{ "usorbian", N_("Usorbian"), false, &iso8859_2, "" },
	// no ISO listing for usorbian (Garst)
	{ "welsh", N_("Welsh"), false, &iso8859_1, "cy_GB" },
	{ 0, 0, false, 0, 0 }
	};

	int i = 0;
	while (items[i].lang) {
		languages[items[i].lang] =
			Language(items[i].lang, items[i].display, 
				 items[i].rtl, items[i].encoding,
				 items[i].code);
		++i;
	}
	
	default_language = &languages["default"];
}


static
LangInit langinit;

bool LangInit::init = false;
