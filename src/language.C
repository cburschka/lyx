
#include <config.h>

#include "language.h"
#include "gettext.h"

Languages languages;
Language const * default_language;
Language ignore_lang("ignore", "Ignore", false);
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
	void newLang(char const * l, char const * d, bool rtl) {
		Language lang(l, d, rtl);
		//lang.lang = l;
		//lang.display = d;
		//lang.RightToLeft = rtl;
		languages[l] = lang;
	}
	///
	void initL();
	///
	static bool init;
};


struct lang_item {
	char const * lang;
	char const * display;
	bool rtl;
};


void LangInit::initL()
{
	// Use this style of initialization to lower compilation times.
	// Same method is used in LyXAction.C (Lgb)
	
	lang_item items[] = {
	{ "afrikaans", N_("Afrikaans"), false },
	{ "american", N_("American"), false },
	{ "arabic", N_("Arabic"), true },
	{ "austrian", N_("Austrian"), false },
	{ "bahasa", N_("Bahasa"), false },
	{ "brazil", N_("Brazil"), false },
	{ "breton", N_("Breton"), false },
	{ "catalan", N_("Catalan"), false },
	{ "croatian", N_("Croatian"), false },
	{ "czech", N_("Czech"), false },
	{ "danish", N_("Danish"), false },
	{ "dutch", N_("Dutch"), false },
	{ "english", N_("English"), false },
	{ "esperanto", N_("Esperanto"), false },
	{ "estonian", N_("Estonian"), false },
	{ "finnish", N_("Finnish"), false },
	{ "francais", N_("Francais"), false },
	{ "french", N_("French"), false },
	{ "frenchb", N_("Frenchb"), false },
	{ "galician", N_("Galician"), false },
	{ "german", N_("German"), false },
	{ "greek", N_("Greek"), false },
	{ "hebrew", N_("Hebrew"), true },
	{ "hungarian", N_("Hungarian"), false },
	{ "irish", N_("Irish"), false },
	{ "italian", N_("Italian"), false },
	{ "lsorbian", N_("Lsorbian"), false },
	{ "magyar", N_("Magyar"), false },
	{ "norsk", N_("Norsk"), false },
	{ "polish", N_("Polish"), false },
	{ "portuges", N_("Portuges"), false },
	{ "romanian", N_("Romanian"), false },
	{ "russian", N_("Russian"), false },
	{ "scottish", N_("Scottish"), false },
	{ "spanish", N_("Spanish"), false },
	{ "slovak", N_("Slovak"), false },
	{ "slovene", N_("Slovene"), false },
	{ "swedish", N_("Swedish"), false },
	{ "turkish", N_("Turkish"), false },
	{ "usorbian", N_("Usorbian"), false },
	{ "welsh", N_("Welsh"), false },
	{ 0, 0, false }
	};

	int i = 0;
	while (items[i].lang) {
		newLang(items[i].lang, items[i].display, items[i].rtl);
		++i;
	}
	
	default_language = &languages["american"];
}


static
LangInit langinit;

bool LangInit::init = false;
