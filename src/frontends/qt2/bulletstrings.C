

#include "bulletstrings.h"

char const * bullets_standard[36] = {
		/* standard */
		"\\normalfont\\bfseries{--}", "\\(\\vdash\\)",
		"\\(\\dashv\\)", "\\(\\flat\\)", "\\(\\natural\\)",
		"\\(\\sharp\\)", "\\(\\ast\\)", "\\(\\star\\)",
		"\\(\\bullet\\)", "\\(\\circ\\)", "\\(\\cdot\\)",
		"\\(\\dagger\\)", "\\(\\bigtriangleup\\)",
		"\\(\\bigtriangledown\\)", "\\(\\triangleleft\\)",
		"\\(\\triangleright\\)", "\\(\\lhd\\)", "\\(\\rhd\\)",
		"\\(\\oplus\\)", "\\(\\ominus\\)", "\\(\\otimes\\)",
		"\\(\\oslash\\)", "\\(\\odot\\)", "\\(\\spadesuit\\)",
		"\\(\\diamond\\)", "\\(\\Diamond\\)", "\\(\\Box\\)",
		"\\(\\diamondsuit\\)", "\\(\\heartsuit\\)",
		"\\(\\clubsuit\\)", "\\(\\rightarrow\\)", "\\(\\leadsto\\)",
		"\\(\\rightharpoonup\\)", "\\(\\rightharpoondown\\)",
		"\\(\\Rightarrow\\)", "\\(\\succ\\)"
	};
char const * bullets_amssymb[36] = {
		/* amssymb */
		"\\(\\Rrightarrow\\)", "\\(\\rightarrowtail\\)",
		"\\(\\twoheadrightarrow\\)", "\\(\\rightsquigarrow\\)",
		"\\(\\looparrowright\\)", "\\(\\multimap\\)",
		"\\(\\boxtimes\\)", "\\(\\boxplus\\)", "\\(\\boxminus\\)",
		"\\(\\boxdot\\)", "\\(\\divideontimes\\)", "\\(\\Vvdash\\)",
		"\\(\\lessdot\\)", "\\(\\gtrdot\\)", "\\(\\maltese\\)",
		"\\(\\bigstar\\)", "\\(\\checkmark\\)", "\\(\\Vdash\\)",
		"\\(\\backsim\\)", "\\(\\thicksim\\)",
		"\\(\\centerdot\\)", "\\(\\circleddash\\)",
		"\\(\\circledast\\)", "\\(\\circledcirc\\)",
		"\\(\\vartriangleleft\\)", "\\(\\vartriangleright\\)",
		"\\(\\vartriangle\\)", "\\(\\triangledown\\)",
		"\\(\\lozenge\\)", "\\(\\square\\)", "\\(\\blacktriangleleft\\)",
		"\\(\\blacktriangleright\\)", "\\(\\blacktriangle\\)",
		"\\(\\blacktriangledown\\)", "\\(\\blacklozenge\\)",
		"\\(\\blacksquare\\)"
	};
char const * bullets_psnfss1[36] = {
		/* psnfss1 */
		"\\ding{108}", "\\ding{109}",
		"\\ding{119}", "\\Pisymbol{psy}{197}",
		"\\Pisymbol{psy}{196}", "\\Pisymbol{psy}{183}",
		"\\ding{71}", "\\ding{70}",
		"\\ding{118}", "\\ding{117}",
		"\\Pisymbol{psy}{224}", "\\Pisymbol{psy}{215}",
		"\\ding{111}", "\\ding{112}",
		"\\ding{113}", "\\ding{114}",
		"\\Pisymbol{psy}{68}", "\\Pisymbol{psy}{209}",
		"\\ding{120}", "\\ding{121}",
		"\\ding{122}", "\\ding{110}",
		"\\ding{115}", "\\ding{116}",
		"\\Pisymbol{psy}{42}", "\\ding{67}",
		"\\ding{66}", "\\ding{82}",
		"\\ding{81}", "\\ding{228}",
		"\\ding{162}", "\\ding{163}",
		"\\ding{166}", "\\ding{167}",
		"\\ding{226}", "\\ding{227}"
	};
char const * bullets_psnfss2[36] = {
		/* psnfss2 */
		"\\ding{37}", "\\ding{38}",
		"\\ding{34}", "\\ding{36}",
		"\\ding{39}", "\\ding{40}",
		"\\ding{41}", "\\ding{42}",
		"\\ding{43}", "\\ding{44}",
		"\\ding{45}", "\\ding{47}",
		"\\ding{53}", "\\ding{54}",
		"\\ding{59}", "\\ding{57}",
		"\\ding{62}", "\\ding{61}",
		"\\ding{55}", "\\ding{56}",
		"\\ding{58}", "\\ding{60}",
		"\\ding{63}", "\\ding{64}",
		"\\ding{51}", "\\ding{52}",
		"\\Pisymbol{psy}{170}", "\\Pisymbol{psy}{167}",
		"\\Pisymbol{psy}{168}", "\\Pisymbol{psy}{169}",
		"\\ding{164}", "\\ding{165}",
		"\\ding{171}", "\\ding{168}",
		"\\ding{169}", "\\ding{170}"
	};
char const * bullets_psnfss3[36] = {
		/* psnfss3 */
		"\\ding{65}", "\\ding{76}",
		"\\ding{75}", "\\ding{72}",
		"\\ding{80}", "\\ding{74}",
		"\\ding{78}", "\\ding{77}",
		"\\ding{79}", "\\ding{85}",
		"\\ding{90}", "\\ding{98}",
		"\\ding{83}", "\\ding{84}",
		"\\ding{86}", "\\ding{87}",
		"\\ding{88}", "\\ding{89}",
		"\\ding{92}", "\\ding{91}",
		"\\ding{93}", "\\ding{105}",
		"\\ding{94}", "\\ding{99}",
		"\\ding{103}", "\\ding{104}",
		"\\ding{106}", "\\ding{107}",
		"\\ding{68}", "\\ding{69}",
		"\\ding{100}", "\\ding{101}",
		"\\ding{102}", "\\ding{96}",
		"\\ding{95}", "\\ding{97}"
	};
char const * bullets_psnfss4[36] = {
		/* psnfss4 */
		"\\ding{223}", "\\ding{224}",
		"\\ding{225}", "\\ding{232}",
		"\\ding{229}", "\\ding{230}",
		"\\ding{238}", "\\ding{237}",
		"\\ding{236}", "\\ding{235}",
		"\\ding{234}", "\\ding{233}",
		"\\ding{239}", "\\ding{241}",
		"\\ding{250}", "\\ding{251}",
		"\\ding{49}", "\\ding{50}",
		"\\ding{217}", "\\ding{245}",
		"\\ding{243}", "\\ding{248}",
		"\\ding{252}", "\\ding{253}",
		"\\ding{219}", "\\ding{213}",
		"\\ding{221}", "\\ding{222}",
		"\\ding{220}", "\\ding{212}",
		"\\Pisymbol{psy}{174}", "\\Pisymbol{psy}{222}",
		"\\ding{254}", "\\ding{242}",
		"\\ding{231}", "\\Pisymbol{psy}{45}"
	};
char const ** BulletPanels[6] = {
		bullets_standard, bullets_amssymb,
		bullets_psnfss1, bullets_psnfss2,
		bullets_psnfss3, bullets_psnfss4
};
