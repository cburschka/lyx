
#include <config.h>

#include "language.h"
#include "gettext.h"

Languages languages;

class LangInit {
public:
	LangInit() {
		if (!init) initL();
		init = true;
	}
private:
	void newLang(string const & l, string const & d, bool ltr) {
		Language lang;
		lang.lang = l;
		lang.display = d;
		lang.leftTOright = ltr;
		languages[l] = lang;
	}
	void initL() {
		newLang("afrikaans", N_("Afrikaans"), false);
		newLang("american", N_("American"), false);
		newLang("arabic", N_("Arabic"), true);
		newLang("austrian", N_("Austrian"), false);
		newLang("bahasa", N_("Bahasa"), false);
		newLang("brazil", N_("Brazil"), false);
		newLang("breton", N_("Breton"), false);
		newLang("catalan", N_("Catalan"), false);
		newLang("croatian", N_("Croatian"), false);
		newLang("czech", N_("Czech"), false);
		newLang("danish", N_("Danish"), false);
		newLang("dutch", N_("Dutch"), false);
		newLang("english", N_("English"), false);
		newLang("esperanto", N_("Esperanto"), false);
		newLang("estonian", N_("Estonian"), false);
		newLang("finnish", N_("Finnish"), false);
		newLang("francais", N_("Francais"), false);
		newLang("french", N_("French"), false);
		newLang("frenchb", N_("Frenchb"), false);
		newLang("galician", N_("Galician"), false);
		newLang("german", N_("German"), false);
		newLang("greek", N_("Greek"), false);
		newLang("hebrew", N_("Hebrew"), true);
		newLang("hungarian", N_("Hungarian"), false);
		newLang("irish", N_("Irish"), false);
		newLang("italian", N_("Italian"), false);
		newLang("lsorbian", N_("Lsorbian"), false);
		newLang("magyar", N_("Magyar"), false);
		newLang("norsk", N_("Norsk"), false);
		newLang("polish", N_("Polish"), false);
		newLang("portuges", N_("Portuges"), false);
		newLang("romanian", N_("Romanian"), false);
		newLang("russian", N_("Russian"), false);
		newLang("scottish", N_("Scottish"), false);
		newLang("spanish", N_("Spanish"), false);
		newLang("slovak", N_("Slovak"), false);
		newLang("slovene", N_("Slovene"), false);
		newLang("swedish", N_("Swedish"), false);
		newLang("turkish", N_("Turkish"), false);
		newLang("usorbian", N_("Usorbian"), false);
		newLang("welsh", N_("Welsh"), false);
	}
	static bool init;
};
static
LangInit langinit;

bool LangInit::init = false;
