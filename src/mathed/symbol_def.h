// This -*- C++ -*- file was created automatically.
// Don't change it!  [asierra18jan96]
// Why? [andre]

#ifndef SYMBOL_DEF
#define SYMBOL_DEF 

enum Math_Symbols_enum {
// Accents
	LM_acute          = '\'',
	LM_grave          = '`',
	LM_hat            = '^',
	LM_tilde          = '~',
	LM_dot            = '.',
	LM_bar            = '-',

/// Symbols that don't exist in X11 symbol font but that we fake
	LM_not            = 10,

// Symbols that do exist in X11 symbol font
	LM_uparrow        = 0xad,
	LM_downarrow      = 0xaf,
	LM_Leftrightarrow = 0xdb,
	LM_Leftarrow      = 0xdc,
	LM_Uparrow        = 0xdd,
	LM_Rightarrow     = 0xde,
	LM_Downarrow      = 0xdf,
	LM_langle         = 0xe1,
	LM_lceil          = 0xe9,
	LM_lfloor         = 0xeb,
	LM_rangle         = 0xf1,
	LM_int            = 0xf2,
	LM_rceil          = 0xf9,
	LM_rfloor         = 0xfb,
		
/// Symbols that don't exist in X11 symbol font
	LM_NoFont          = 256,

/// needed as decoration
	LM_Vert,
	LM_updownarrow,
	LM_Updownarrow,

/// decorations
	LM_widehat,
	LM_widetilde,
	LM_underline,
	LM_overline,
	LM_underbrace, 
	LM_overbrace,
	LM_overleftarrow,
	LM_overightarrow, 

/// dots
	LM_ldots,
	LM_cdots,
	LM_vdots,
	LM_ddots,            
	LM_emptyset,

/// Accents that don't exist in X11 symbol font
	LM_ddot,
	LM_check,
	LM_vec,
	LM_breve
};

#endif
