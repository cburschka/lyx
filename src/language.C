
#include <config.h>

#include "language.h"
#include "gettext.h"
#include "encoding.h"

Languages languages;
Language const * default_language;
Language ignore_lang("ignore", "Ignore", false, 0);
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
};


void LangInit::initL()
{
	// Use this style of initialization to lower compilation times.
	// Same method is used in LyXAction.C (Lgb)

	lang_item items[] = {
	{ "afrikaans", N_("Afrikaans"), false, &iso8859_1 },
	{ "american", N_("American"), false, &iso8859_1 },
	{ "arabic", N_("Arabic"), true, &iso8859_6 },
	{ "austrian", N_("Austrian"), false, &iso8859_1 },
	{ "bahasa", N_("Bahasa"), false, &iso8859_1 },
	{ "brazil", N_("Brazil"), false, &iso8859_1 },
	{ "breton", N_("Breton"), false, &iso8859_1 },
	{ "catalan", N_("Catalan"), false, &iso8859_1 },
	{ "croatian", N_("Croatian"), false, &iso8859_2 },
	{ "czech", N_("Czech"), false, &iso8859_2 },
	{ "danish", N_("Danish"), false, &iso8859_1 },
	{ "dutch", N_("Dutch"), false, &iso8859_1 },
	{ "english", N_("English"), false, &iso8859_1 },
	{ "esperanto", N_("Esperanto"), false, &iso8859_3 },
	{ "estonian", N_("Estonian"), false, &iso8859_4 },
	{ "finnish", N_("Finnish"), false, &iso8859_1 },
	{ "francais", N_("Francais"), false, &iso8859_1 },
	{ "french", N_("French"), false, &iso8859_1 },
	{ "frenchb", N_("Frenchb"), false, &iso8859_1 },
	{ "galician", N_("Galician"), false, &iso8859_1 },
	{ "german", N_("German"), false, &iso8859_1 },
	{ "greek", N_("Greek"), false, &iso8859_7 },
	{ "hebrew", N_("Hebrew"), true, &cp1255 },
	/* According to Zrubecz Laszlo <zrubi@k2.jozsef.kando.hu>,
	   "magyar" is better. I kept it here in case we want
	   to  provide aliasing of languages. (JMarc) 
	*/
	//{ "hungarian", N_("Hungarian"), false, &iso8859_2 },
	{ "irish", N_("Irish"), false, &iso8859_1 },
	{ "italian", N_("Italian"), false, &iso8859_1 },
	{ "lsorbian", N_("Lsorbian"), false, &iso8859_2 },
	{ "magyar", N_("Magyar"), false, &iso8859_2 },
	{ "norsk", N_("Norsk"), false, &iso8859_1 },
	{ "polish", N_("Polish"), false, &iso8859_2 },
	{ "portuges", N_("Portuges"), false, &iso8859_1 },
	{ "romanian", N_("Romanian"), false, &iso8859_2 },
	{ "russian", N_("Russian"), false, &koi8 },
	{ "scottish", N_("Scottish"), false, &iso8859_1 },
	{ "spanish", N_("Spanish"), false, &iso8859_1 },
	{ "slovak", N_("Slovak"), false, &iso8859_2 },
	{ "slovene", N_("Slovene"), false, &iso8859_2 },
	{ "swedish", N_("Swedish"), false, &iso8859_1 },
	{ "turkish", N_("Turkish"), false, &iso8859_9 },
	{ "usorbian", N_("Usorbian"), false, &iso8859_2 },
	{ "welsh", N_("Welsh"), false, &iso8859_1 },
	{ 0, 0, false, 0 }
	};

	int i = 0;
	while (items[i].lang) {
		languages[items[i].lang] =
			Language(items[i].lang, items[i].display, 
				 items[i].rtl, items[i].encoding);
		++i;
	}
	
	default_language = &languages["american"];
}


static
LangInit langinit;

bool LangInit::init = false;
