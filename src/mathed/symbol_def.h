// This file was created automatically. Don't change it!  [asierra18jan96]

#ifndef SYMBOL_DEF
#define SYMBOL_DEF 

// Symbols that do exist in X11 symbol font
#define LM_Gamma 0x47
#define LM_Delta 0x44
#define LM_Theta 0x51
#define LM_Lambda 0x4c
#define LM_Xi 0x58
#define LM_Pi 0x50
#define LM_Sigma 0x53
//#define LM_Upsilon 0x55
#define LM_Upsilon 0xa1
#define LM_Phi 0x46
#define LM_Psi 0x59
#define LM_Omega 0x57
#define LM_alpha 0x61
#define LM_beta 0x62
#define LM_gamma 0x67
#define LM_delta 0x64
#define LM_epsilon 0x65
#define LM_eta 0x68
#define LM_theta 0x71
#define LM_vartheta 0x4a
#define LM_iota 0x69
#define LM_kappa 0x6b
#define LM_lambda 0x6c
#define LM_mu 0x6d
#define LM_nu 0x6e
#define LM_xi 0x78
#define LM_pi 0x70
#define LM_varpi 0x76
#define LM_rho 0x72
#define LM_sigma 0x73
#define LM_tau 0x74
#define LM_varsigma 0x56
#define LM_zeta 0x7a 
#define LM_upsilon 0x75
#define LM_phi 0x66
#define LM_varphi 0x6a
#define LM_chi 0x63
#define LM_psi 0x79
#define LM_omega 0x77
#define LM_downarrow 0xaf
#define LM_leftarrow 0xac
#define LM_Downarrow 0xdf
#define LM_Leftarrow 0xdc
#define LM_rightarrow 0xae
#define LM_uparrow 0xad
#define LM_Rightarrow 0xde
#define LM_Uparrow 0xdd
#define LM_Leftrightarrow 0xdb
#define LM_leftrightarrow 0xab
#define LM_leq 0xa3
#define LM_geq 0xb3
#define LM_equiv 0xba
#define LM_subset 0xcc
#define LM_supset 0xc9
#define LM_approx 0xbb
#define LM_subseteq 0xcd
#define LM_supseteq 0xca
#define LM_cong 0x40
#define LM_neq 0xb9
#define LM_in 0xce
#define LM_propto 0xb5
#define LM_pm 0xb1
#define LM_cap 0xc7
#define LM_diamond 0xe0
#define LM_oplus 0xc5
#define LM_cup 0xc8
#define LM_times 0xb4
#define LM_otimes 0xc4
#define LM_div 0xb8
#define LM_oslash 0xc6
#define LM_cdot 0xd7
#define LM_wedge 0xd9
#define LM_bullet 0xb7
#define LM_sum 0xe5
#define LM_int 0xf2
#define LM_prod 0xd5
#define LM_nabla 0xd1
#define LM_partial 0xb6
#define LM_infty 0xa5
#define LM_prime 0xa2
//#define LM_emptyset 0xc6
#define LM_exists 0x24
#define LM_forall 0x22
#define LM_Re 0xc2
#define LM_Im 0xc1
#define LM_aleph 0xc0
#define LM_wp 0xc3
#define LM_bot 0x5e
#define LM_neg 0xd8
#define LM_sharp 0x23
#define LM_surd 0xd6
#define LM_diamondsuit 0xa8
#define LM_heartsuit 0xa9
#define LM_clubsuit 0xa7
#define LM_spadesuit 0xaa
#define LM_langle 0xe1
#define LM_lceil 0xe9
#define LM_lfloor 0xeb
#define LM_rangle 0xf1
#define LM_rceil 0xf9
#define LM_rfloor 0xfb
#define LM_mid 0x7c

#define LM_angle 0xd0
#define LM_vee 0xda

//#define LM_backslash '\\'
  
// Symbols that don't exist in X11 symbol font
enum Math_Symbols_enum {
  LM_NoFont = 256,
  LM_varepsilon,  
  LM_hookleftarrow,  LM_hookrightarrow,  LM_updownarrow,  LM_leftharpoonup,
  LM_rightharpoonup,  LM_rightleftharpoons,  LM_Updownarrow,
  LM_leftharpoondown,  LM_rightharpoondown,  LM_mapsto,  LM_Longleftarrow,
  LM_Longrightarrow,  LM_Longleftrightarrow,  LM_longleftrightarrow,
  LM_longleftarrow,  LM_longrightarrow,  LM_longmapsto,  LM_nwarrow,
  LM_nearrow,  LM_swarrow,  LM_searrow,
  LM_models,  LM_prec,  LM_succ,  LM_sim,  LM_perp,  LM_preceq,  LM_succeq,
  LM_simeq,  LM_ll,  LM_gg,  LM_asymp,  LM_parallel,  LM_smile,
  LM_frown,  LM_sqsubseteq,  LM_sqsupseteq,  LM_doteq,  LM_ni,  LM_notin,
  LM_vdash,  LM_dashv,  LM_bowtie,
  LM_mp,  LM_bigtriangleup,  LM_ominus,  LM_uplus,  LM_bigtriangledown,
  LM_sqcap,  LM_triangleright,  LM_sqcup,  LM_triangleleft,  LM_odot,  LM_star,
  LM_amalg,  LM_bigcirc,  LM_setminus,  LM_dagger,  LM_circ,  LM_wr,
  LM_ddagger,
  LM_oint,  LM_coprod,  LM_bigsqcup,  LM_bigotimes,  LM_bigodot,  LM_bigoplus,
  LM_bigcap,  LM_bigcup,  LM_biguplus,  LM_bigvee,  LM_bigwedge,
  LM_ell,  LM_imath,  LM_jmath,  LM_hbar, LM_top,  LM_Vert,  LM_flat,
  LM_natural,  LM_triangle,
  LM_widehat, LM_widetilde, LM_underline, LM_overline, LM_underbrace, 
  LM_overbrace, LM_overleftarrow, LM_overightarrow, 
  LM_ldots, LM_cdots, LM_vdots, LM_ddots,            
  LM_backslash, LM_emptyset,
  LM_last_symbol
};

// Accents
#define LM_acute '\''
#define LM_grave  '`'
#define LM_hat '^'
#define LM_tilde '~'
#define LM_dot  '.'
#define LM_bar '-'

enum Math_Accent_enum {
  LM_ddot = LM_last_symbol,
  LM_check, LM_vec, LM_breve, LM_not
};

#define LM_quad  4
#define LM_qquad 5

#endif

