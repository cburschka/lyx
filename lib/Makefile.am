include $(top_srcdir)/config/common.am

SUBDIRS = doc lyx2lyx

CHMOD = chmod

EXTRA_DIST = examples/CMakeLists.txt scripts/CMakeLists.txt examples/README.new_examples

dist_pkgdata_DATA = CREDITS autocorrect chkconfig.ltx \
	encodings layouttranslations languages latexfonts RELEASE-NOTES \
	symbols syntax.default unicodesymbols

# We use DATA now instead of PYTHON because automake 1.11.2 complains.
# Note that we "chmod 755" manually this file in install-data-hook.
dist_pkgdata_DATA += configure.py

dist_noinst_DATA = \
	appdata.xml \
	fonts/test/check_glyphs.lyx \
	images/README \
	images/font-smallcaps.svgz \
	images/math/ams_arrows.png \
	images/math/ams_misc.png \
	images/math/ams_nrel.png \
	images/math/ams_ops.png \
	images/math/ams_rel.png \
	images/math/arrows.png \
	images/math/bop.png \
	images/math/brel.png \
	images/math/deco.svgz \
	images/math/delim0.svgz \
	images/math/delim1.svgz \
	images/math/delim.svgz \
	images/math/dots.svgz \
	images/math/font.svgz \
	images/math/greek.png \
	images/math/misc.png \
	images/math/varsz.png

skbinddir = $(pkgdatadir)/bind/sk
dist_skbind_DATA = \
	bind/sk/menus.bind

debinddir = $(pkgdatadir)/bind/de
dist_debind_DATA = \
	bind/de/menus.bind

binddir = $(pkgdatadir)/bind
dist_bind_DATA = \
	bind/broadway.bind \
	bind/cua.bind \
	bind/cyrkeys.bind \
	bind/emacs.bind \
	bind/greekkeys.bind \
	bind/hollywood.bind \
	bind/latinkeys.bind \
	bind/mac.bind \
	bind/math.bind \
	bind/menus.bind \
	bind/sciword.bind \
	bind/site.bind \
	bind/xemacs.bind \
	bind/aqua.bind

commandsdir = $(pkgdatadir)/commands
dist_commands_DATA = \
	commands/default.def

arexamplesdir = $(pkgdatadir)/examples/ar
dist_arexamples_DATA = \
	examples/ar/Welcome.lyx

caexamplesdir = $(pkgdatadir)/examples/ca
dist_caexamples_DATA = \
	examples/ca/Welcome.lyx

cainsetexamplesdir = $(pkgdatadir)/examples/ca/Graphics_and_Insets
dist_cainsetexamples_DATA = \
	examples/ca/Graphics_and_Insets/Itemize_Bullets.lyx

csexamplesdir = $(pkgdatadir)/examples/cs
dist_csexamples_DATA = \
	examples/cs/Welcome.lyx \
	examples/cs/Example_%28LyXified%29.lyx \
	examples/cs/Example_%28raw%29.lyx

daexamplesdir = $(pkgdatadir)/examples/da
dist_daexamples_DATA = \
	examples/da/Welcome.lyx

deexamplesdir = $(pkgdatadir)/examples/de
dist_deexamples_DATA = \
	examples/de/Example_%28LyXified%29.lyx \
	examples/de/Example_%28raw%29.lyx \
	examples/de/Welcome.lyx

decvexamplesdir = $(pkgdatadir)/examples/de/Curricula_Vitae
dist_decvexamples_DATA = \
	examples/de/Curricula_Vitae/Europass_CV_%282013%29.lyx \
	examples/de/Curricula_Vitae/Simple_CV.lyx

dehandoutexamplesdir = $(pkgdatadir)/examples/de/Handouts
dist_dehandoutexamples_DATA = \
	examples/de/Handouts/Tufte_Handout.lyx

deinsetexamplesdir = $(pkgdatadir)/examples/de/Graphics_and_Insets
dist_deinsetexamples_DATA = \
	examples/de/Graphics_and_Insets/Itemize_Bullets.lyx \
	examples/de/Graphics_and_Insets/Feynman_Diagrams.lyx

deletterexamplesdir = $(pkgdatadir)/examples/de/Letters
dist_deletterexamples_DATA = \
	examples/de/Letters/addressExample.adr \
	examples/de/Letters/Serial_Letter_1.lyx \
	examples/de/Letters/Serial_Letter_2.lyx \
	examples/de/Letters/Serial_Letter_3.lyx

depresentationexamplesdir = $(pkgdatadir)/examples/de/Presentations
dist_depresentationexamples_DATA = \
	examples/de/Presentations/Beamer.lyx \
	examples/de/Presentations/Beamer_Article_%28Standard_Class%29.lyx

demoduleexamplesdir = $(pkgdatadir)/examples/de/Modules
dist_demoduleexamples_DATA = \
	examples/de/Modules/Braille.lyx \
	examples/de/Modules/Linguistics.lyx \
	examples/de/Modules/Multilingual_Captions.lyx \
	examples/de/Modules/PDF_Comments.lyx \
	examples/de/Modules/PDF_Form.lyx

elexamplesdir = $(pkgdatadir)/examples/el
dist_elexamples_DATA = \
	examples/el/Welcome.lyx

esexamplesdir = $(pkgdatadir)/examples/es
dist_esexamples_DATA = \
	examples/es/Example_%28LyXified%29.lyx \
	examples/es/Example_%28raw%29.lyx \
	examples/es/Welcome.lyx

esbookexamplesdir = $(pkgdatadir)/examples/es/Books
dist_esbookexamples_DATA = \
	examples/es/Books/Tufte_Book.lyx

escvexamplesdir = $(pkgdatadir)/examples/es/Curricula_Vitae
dist_escvexamples_DATA = \
	examples/es/Curricula_Vitae/Europe_CV.lyx \
	examples/es/Curricula_Vitae/Modern_CV.lyx

eshandoutexamplesdir = $(pkgdatadir)/examples/es/Handouts
dist_eshandoutexamples_DATA = \
	examples/es/Handouts/Tufte_Handout.lyx

esinsetexamplesdir = $(pkgdatadir)/examples/es/Graphics_and_Insets
dist_esinsetexamples_DATA = \
	examples/es/Graphics_and_Insets/Itemize_Bullets.lyx

esmoduleexamplesdir = $(pkgdatadir)/examples/es/Modules
dist_esmoduleexamples_DATA = \
	examples/es/Modules/Braille.lyx \
	examples/es/Modules/Linguistics.lyx

euexamplesdir = $(pkgdatadir)/examples/eu
dist_euexamples_DATA = \
	examples/eu/Example_%28LyXified%29.lyx \
	examples/eu/Example_%28raw%29.lyx
	examples/eu/Welcome.lyx

faexamplesdir = $(pkgdatadir)/examples/fa
dist_faexamples_DATA = \
	examples/fa/Welcome.lyx

frexamplesdir = $(pkgdatadir)/examples/fr
dist_frexamples_DATA = \
	examples/fr/Example_%28LyXified%29.lyx \
	examples/fr/Example_%28raw%29.lyx \
	examples/fr/Welcome.lyx

frcvexamplesdir = $(pkgdatadir)/examples/fr/Curricula_Vitae
dist_frcvexamples_DATA = \
	examples/fr/Curricula_Vitae/Simple_CV.lyx

frinsetexamplesdir = $(pkgdatadir)/examples/fr/Graphics_and_Insets
dist_frinsetexamples_DATA = \
	examples/fr/Graphics_and_Insets/Itemize_Bullets.lyx \
	examples/fr/Graphics_and_Insets/Feynman_Diagrams.lyx \
	examples/fr/Graphics_and_Insets/XY-Figure.lyx \
	examples/fr/Graphics_and_Insets/xyfigure.png \
	examples/fr/Graphics_and_Insets/XY-Pic.lyx

frmoduleexamplesdir = $(pkgdatadir)/examples/fr/Modules
dist_frmoduleexamples_DATA = \
	examples/fr/Modules/Braille.lyx \
	examples/fr/Modules/Linguistics.lyx \
	examples/fr/Modules/Multilingual_Captions.lyx \
	examples/fr/Modules/PDF_Comments.lyx

frpresentationexamplesdir = $(pkgdatadir)/examples/fr/Presentations
dist_frpresentationexamples_DATA = \
	examples/fr/Presentations/Beamer.lyx \
	examples/fr/Presentations/Beamer_Article_%28Standard_Class%29.lyx \
	examples/fr/Presentations/Foils.lyx \
	examples/fr/Presentations/Powerdot.lyx \
	examples/fr/Presentations/Seminar.lyx

glexamplesdir = $(pkgdatadir)/examples/gl
dist_glexamples_DATA = \
	examples/gl/Example_%28LyXified%29.lyx \
	examples/gl/Example_%28raw%29.lyx \
	examples/gl/Welcome.lyx

heexamplesdir = $(pkgdatadir)/examples/he
dist_heexamples_DATA = \
	examples/he/Example_%28LyXified%29.lyx \
	examples/he/Example_%28raw%29.lyx \
	examples/he/Welcome.lyx

huexamplesdir = $(pkgdatadir)/examples/hu
dist_huexamples_DATA = \
	examples/hu/Example_%28LyXified%29.lyx \
	examples/hu/Example_%28raw%29.lyx \
	examples/hu/Welcome.lyx

idexamplesdir = $(pkgdatadir)/examples/id
dist_idexamples_DATA = \
	examples/id/Welcome.lyx

itexamplesdir = $(pkgdatadir)/examples/it
dist_itexamples_DATA = \
	examples/it/Welcome.lyx

itinsetexamplesdir = $(pkgdatadir)/examples/it/Graphics_and_Insets
dist_itinsetexamples_DATA = \
	examples/it/Graphics_and_Insets/Itemize_Bullets.lyx

jaexamplesdir = $(pkgdatadir)/examples/ja
dist_jaexamples_DATA = \
	examples/ja/Multilingual_Typesetting_with_CJKutf8.lyx \
	examples/ja/Multilingual_Typesetting_with_platex.lyx \
	examples/ja/Welcome.lyx

jainsetexamplesdir = $(pkgdatadir)/examples/ja/Graphics_and_Insets
dist_jainsetexamples_DATA = \
	examples/ja/Graphics_and_Insets/Feynman_Diagrams.lyx \
	examples/ja/Graphics_and_Insets/xyfigure.png \
	examples/ja/Graphics_and_Insets/XY-Pic.lyx

jamoduleexamplesdir = $(pkgdatadir)/examples/ja/Modules
dist_jamoduleexamples_DATA = \
	examples/ja/Modules/Braille.lyx \
	examples/ja/Modules/LilyPond_Book.lyx \
	examples/ja/Modules/Linguistics.lyx \
	examples/ja/Modules/Multilingual_Captions.lyx \
	examples/ja/Modules/Rnw_%28knitr%29.lyx \
	examples/ja/Modules/Sweave.lyx

japresentationexamplesdir = $(pkgdatadir)/examples/ja/Presentations
dist_japresentationexamples_DATA = \
	examples/ja/Presentations/Beamer.lyx

koexamplesdir = $(pkgdatadir)/examples/ko
dist_koexamples_DATA = \
	examples/ko/Welcome.lyx \
	examples/ko/cjk-ko-doc.lyx

nbexamplesdir = $(pkgdatadir)/examples/nb
dist_nbexamples_DATA = \
	examples/nb/Example_%28LyXified%29.lyx \
	examples/nb/Example_%28raw%29.lyx \
	examples/nb/Welcome.lyx

nlexamplesdir = $(pkgdatadir)/examples/nl
dist_nlexamples_DATA = \
	examples/nl/Example_%28LyXified%29.lyx \
	examples/nl/Example_%28raw%29.lyx \
	examples/nl/Welcome.lyx

nlinsetexamplesdir = $(pkgdatadir)/examples/nl/Graphics_and_Insets
dist_nlinsetexamples_DATA = \
	examples/nl/Graphics_and_Insets/Itemize_Bullets.lyx

plexamplesdir = $(pkgdatadir)/examples/pl
dist_plexamples_DATA = \
	examples/pl/Welcome.lyx

pt_BRexamplesdir = $(pkgdatadir)/examples/pt_BR
dist_pt_BRexamples_DATA = \
	examples/pt_BR/Welcome.lyx

pt_PTexamplesdir = $(pkgdatadir)/examples/pt_PT
dist_pt_PTexamples_DATA = \
	examples/pt_PT/Welcome.lyx

roexamplesdir = $(pkgdatadir)/examples/ro
dist_roexamples_DATA = \
	examples/ro/Welcome.lyx

ruexamplesdir = $(pkgdatadir)/examples/ru
dist_ruexamples_DATA = \
	examples/ru/Example_%28LyXified%29.lyx \
	examples/ru/Example_%28raw%29.lyx \
	examples/ru/Welcome.lyx

ruinsetexamplesdir = $(pkgdatadir)/examples/ru/Graphics_and_Insets
dist_ruinsetexamples_DATA = \
	examples/ru/Graphics_and_Insets/XY-Figure.lyx \
	examples/ru/Graphics_and_Insets/xyfigure.png \
	examples/ru/Graphics_and_Insets/XY-Pic.lyx

rupresentationexamplesdir = $(pkgdatadir)/examples/ru/Presentations
dist_rupresentationexamples_DATA = \
	examples/ru/Presentations/Beamer.lyx \
	examples/ru/Presentations/Beamer_Article_%28Standard_Class%29.lyx

slexamplesdir = $(pkgdatadir)/examples/sl
dist_slexamples_DATA = \
	examples/sl/Example_%28LyXified%29.lyx \
	examples/sl/Example_%28raw%29.lyx \
	examples/sl/Welcome.lyx

skexamplesdir = $(pkgdatadir)/examples/sk
dist_skexamples_DATA = \
	examples/sk/Welcome.lyx

srexamplesdir = $(pkgdatadir)/examples/sr
dist_srexamples_DATA = \
	examples/sr/Welcome.lyx

srmoduleexamplesdir = $(pkgdatadir)/examples/sr/Modules
dist_srmoduleexamples_DATA = \
	examples/sr/Modules/Braille.lyx

svexamplesdir = $(pkgdatadir)/examples/sv
dist_svexamples_DATA = \
	examples/sv/Welcome.lyx

thesistemplatesdir = $(pkgdatadir)/templates/Theses
dist_thesistemplates_DATA =

phdthesistemplatesdir = $(pkgdatadir)/templates/Theses/PhD_Thesis
dist_phdthesistemplates_DATA = \
	templates/Theses/PhD_Thesis/Acknowledgments.lyx \
	templates/Theses/PhD_Thesis/alpha.bst \
	templates/Theses/PhD_Thesis/Appendix.lyx \
	templates/Theses/PhD_Thesis/Bibliography.lyx \
	templates/Theses/PhD_Thesis/Chapter_1.lyx \
	templates/Theses/PhD_Thesis/Chapter_2.lyx \
	templates/Theses/PhD_Thesis/Main_File.lyx \
	templates/Theses/PhD_Thesis/Summary.lyx \
	templates/Theses/PhD_Thesis/thesisExample.bib

ukexamplesdir = $(pkgdatadir)/examples/uk
dist_ukexamples_DATA = \
	examples/uk/Welcome.lyx

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = \
	examples/biblioExample.bib \
	examples/Example_%28LyXified%29.lyx \
	examples/Example_%28raw%29.lyx \
	examples/Localization_Test.lyx \
	examples/Welcome.lyx

articleexamplesdir = $(pkgdatadir)/examples/Articles
dist_articleexamples_DATA = \
	examples/Articles/Astronomy_%26_Astrophysics.lyx \
	examples/Articles/American_Astronomical_Society_%28AASTeX_v._6.2%29.lyx \
	examples/Articles/American_Chemical_Society_%28ACS%29.lyx \
	examples/Articles/American_Mathematical_Society_%28AMS%29.lyx

articlechessexamplesdir = $(pkgdatadir)/examples/Articles/Chess
dist_articlechessexamples_DATA = \
	examples/Articles/Chess/Game_1.lyx \
	examples/Articles/Chess/Game_2.lyx

bookexamplesdir = $(pkgdatadir)/examples/Books
dist_bookexamples_DATA = \
	examples/Books/American_Mathematical_Society_%28AMS%29.lyx \
	examples/Books/Recipe_Book.lyx \
	examples/Books/Tufte_Book.lyx

booksvmonotemplatesdir = $(pkgdatadir)/templates/Books/Springer_Monographs_%28svmono%29
dist_booksvmonotemplates_DATA = \
	templates/Books/Springer_Monographs_%28svmono%29/00_Main_File.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/01_Dedication.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/02_Foreword.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/03_Preface.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/04_Acknowledgements.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/05_Acronym.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/06_Part.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/07_Chapter.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/08_Appendix.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/09_Glossary.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/10_Solutions.lyx \
	templates/Books/Springer_Monographs_%28svmono%29/11_References.lyx

booksvmulttemplatesdir = $(pkgdatadir)/templates/Books/Springer_Contributed_Books_%28svmult%29
dist_booksvmulttemplates_DATA = \
	templates/Books/Springer_Contributed_Books_%28svmult%29/00_Main_File.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/01_Dedication.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/02_Foreword.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/03_Preface.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/04_Acknowledgements.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/05_Contributor_List.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/06_Acronym.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/07_Part.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/08_Author.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/09_Appendix.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/10_Glossary.lyx \
	templates/Books/Springer_Contributed_Books_%28svmult%29/11_References.lyx

cvexamplesdir = $(pkgdatadir)/examples/Curricula_Vitae
dist_cvexamples_DATA = \
	examples/Curricula_Vitae/Europass_CV_%282013%29.lyx \
	examples/Curricula_Vitae/Europe_CV.lyx \
	examples/Curricula_Vitae/Modern_CV.lyx \
	examples/Curricula_Vitae/Simple_CV.lyx \
	examples/Curricula_Vitae/CV-image.eps \
	examples/Curricula_Vitae/CV-image.png \
	examples/Curricula_Vitae/REPLACEphoto.jpg

externalexamplesdir = $(pkgdatadir)/examples/External_Material
dist_externalexamples_DATA = \
	examples/External_Material/Gnuplot.lyx \
	examples/External_Material/gnuplot-example.gp \
	examples/External_Material/longsheet.gnumeric \
	examples/External_Material/longsheet.tex \
	examples/External_Material/sheet1.gnumeric \
	examples/External_Material/sheet2.ods \
	examples/External_Material/sheet3.xls \
	examples/External_Material/Spreadsheet.lyx

handoutexamplesdir = $(pkgdatadir)/examples/Handouts
dist_handoutexamples_DATA = \
	examples/Handouts/Tufte_Handout.lyx

insetexamplesdir = $(pkgdatadir)/examples/Graphics_and_Insets
dist_insetexamples_DATA = \
	examples/Graphics_and_Insets/Feynman_Diagrams.lyx \
	examples/Graphics_and_Insets/iecc05.fen \
	examples/Graphics_and_Insets/iecc07.fen \
	examples/Graphics_and_Insets/iecc12.fen \
	examples/Graphics_and_Insets/Instant_Preview.lyx \
	examples/Graphics_and_Insets/Itemize_Bullets.lyx \
	examples/Graphics_and_Insets/Minted_File_Listing.lyx \
	examples/Graphics_and_Insets/Minted_Listings.lyx \
	examples/Graphics_and_Insets/XY-Figure.lyx \
	examples/Graphics_and_Insets/xyfigure.png \
	examples/Graphics_and_Insets/XY-Pic.lyx

letterexamplesdir = $(pkgdatadir)/examples/Letters
dist_letterexamples_DATA = \
	examples/Letters/addressExample.adr \
	examples/Letters/Serial_Letter_1.lyx \
	examples/Letters/Serial_Letter_2.lyx \
	examples/Letters/Serial_Letter_3.lyx

moduleexamplesdir = $(pkgdatadir)/examples/Modules
dist_moduleexamples_DATA = \
	examples/Modules/Braille.lyx \
	examples/Modules/Chessboard.lyx \
	examples/Modules/Fancy_Colored_Boxes.lyx \
	examples/Modules/Hazard_and_Precautionary_Statements.lyx \
	examples/Modules/LilyPond_Book.lyx \
	examples/Modules/Linguistics.lyx \
	examples/Modules/Multilingual_Captions.lyx \
	examples/Modules/Noweb.lyx \
	examples/Modules/Noweb2LyX.lyx \
	examples/Modules/Noweb_Listerrors.lyx \
	examples/Modules/Paragraph_Lists_%28paralist%29.lyx \
	examples/Modules/PDF_Comments.lyx \
	examples/Modules/PDF_Form.lyx \
	examples/Modules/Rnw_%28knitr%29.lyx \
	examples/Modules/Sweave.lyx \
	examples/Modules/Variable-width_Minipages.lyx

posterexamplesdir = $(pkgdatadir)/examples/Posters
dist_posterexamples_DATA = \
	examples/Posters/Beamerposter.lyx \
	examples/Posters/CV-image.png \
	examples/Posters/beamer-knight1.png

presentationexamplesdir = $(pkgdatadir)/examples/Presentations
dist_presentationexamples_DATA = \
	examples/Presentations/Beamer.lyx \
	examples/Presentations/Beamer_Article_%28Standard_Class%29.lyx \
	examples/Presentations/beamer-g4.jpg \
	examples/Presentations/beamer-g4-mask.jpg \
	examples/Presentations/beamer-icsi-logo.pdf \
	examples/Presentations/beamer-knight1-mask.png \
	examples/Presentations/beamer-knight1.png \
	examples/Presentations/beamer-knight2-mask.png \
	examples/Presentations/beamer-knight2.png \
	examples/Presentations/beamer-knight3-mask.png \
	examples/Presentations/beamer-knight3.png \
	examples/Presentations/beamer-knight4-mask.png \
	examples/Presentations/beamer-knight4.png \
	examples/Presentations/Beamer_%28Complex%29.lyx \
	examples/Presentations/Foils.lyx \
	examples/Presentations/Foils_Landslide.lyx \
	examples/Presentations/Powerdot.lyx \
	examples/Presentations/Seminar.lyx

scriptexamplesdir = $(pkgdatadir)/examples/Scripts
dist_scriptexamples_DATA = \
	examples/Scripts/Hollywood.lyx

fontsdir = $(pkgdatadir)/fonts
dist_fonts_DATA = \
	fonts/BaKoMaFontLicense.txt \
	fonts/cmex10.ttf \
	fonts/cmmi10.ttf \
	fonts/cmr10.ttf \
	fonts/cmsy10.ttf \
	fonts/dsrom10.ttf \
	fonts/esint10.ttf \
	fonts/eufm10.ttf \
	fonts/msam10.ttf \
	fonts/msbm10.ttf \
	fonts/README \
	fonts/rsfs10.ttf \
	fonts/stmary10.ttf \
	fonts/wasy10.ttf

imagesdir = $(pkgdatadir)/images
dist_images_DATA1X = \
	images/all-changes-accept.svgz \
	images/all-changes-reject.svgz \
	images/banner.svgz \
	images/bookmark-goto.svgz \
	images/bookmark-goto_0.svgz \
	images/bookmark-save.svgz \
	images/box-insert.svgz \
	images/break-line.svgz \
	images/buffer-close.svgz \
	images/buffer-export.svgz \
	images/buffer-export_dvi.svgz \
	images/buffer-export_dvi3.svgz \
	images/buffer-export_latex.svgz \
	images/buffer-export_pdf.svgz \
	images/buffer-export_pdf2.svgz \
	images/buffer-export_pdf3.svgz \
	images/buffer-export_pdf4.svgz \
	images/buffer-export_pdf5.svgz \
	images/buffer-export_ps.svgz \
	images/buffer-export_text.svgz \
	images/buffer-new.svgz \
	images/buffer-reload.svgz \
	images/buffer-toggle-output-sync.svgz \
	images/buffer-update.svgz \
	images/buffer-update_dvi.svgz \
	images/buffer-update_dvi3.svgz \
	images/buffer-update_pdf.svgz \
	images/buffer-update_pdf2.svgz \
	images/buffer-update_pdf3.svgz \
	images/buffer-update_pdf4.svgz \
	images/buffer-update_pdf5.svgz \
	images/buffer-update_ps.svgz \
	images/buffer-view.svgz \
	images/buffer-view_dvi.svgz \
	images/buffer-view_dvi3.svgz \
	images/buffer-view_pdf.svgz \
	images/buffer-view_pdf2.svgz \
	images/buffer-view_pdf3.svgz \
	images/buffer-view_pdf4.svgz \
	images/buffer-view_pdf5.svgz \
	images/buffer-view_ps.svgz \
	images/buffer-write-as.svgz \
	images/buffer-write.svgz \
	images/build-program.svgz \
	images/busy.gif \
	images/change-accept.svgz \
	images/change-next.svgz \
	images/change-reject.svgz \
	images/changes-merge.svgz \
	images/changes-output.svgz \
	images/changes-track.svgz \
	images/closetab.svgz \
	images/tab-group-close.svgz \
	images/command-alternatives_outline-in_depth-increment.svgz \
	images/command-alternatives_outline-out_depth-decrement.svgz \
	images/copy.svgz \
	images/cut.svgz \
	images/depth-decrement.svgz \
	images/depth-increment.svgz \
	images/dialog-preferences.svgz \
	images/dialog-show-new-inset_citation.svgz \
	images/dialog-show-new-inset_graphics.svgz \
	images/dialog-show-new-inset_include.svgz \
	images/dialog-show-new-inset_ref.svgz \
	images/dialog-show_character.svgz \
	images/dialog-show_findreplace.svgz \
	images/dialog-show_mathdelimiter.svgz \
	images/dialog-show_mathmatrix.svgz \
	images/dialog-show_print.svgz \
	images/dialog-show_spellchecker.svgz \
	images/dialog-show_vclog.svgz \
	images/dialog-toggle_findreplaceadv.svgz \
	images/dialog-toggle_toc.svgz \
	images/down.svgz \
	images/dynamic-char-styles.svgz \
	images/dynamic-custom-insets.svgz \
	images/editclear.svgz \
	images/emblem-readonly.svgz \
	images/emblem-shellescape.svgz \
	images/ert-insert.svgz \
	images/file-open.svgz \
	images/float-insert_figure.svgz \
	images/float-insert_table.svgz \
	images/font-bold.svgz \
	images/font-emph.svgz \
	images/textstyle-apply.svgz \
	images/font-noun.svgz \
	images/font-sans.svgz \
	images/footnote-insert.svgz \
	images/href-insert.svgz \
	images/hidetab.svgz \
	images/index-insert.svgz \
	images/info-insert.svgz \
	images/info-insert_vcs_revision.svgz \
	images/label-insert.svgz \
	images/layout.svgz \
	images/layout-document.svgz \
	images/layout-paragraph.svgz \
	images/layout-toggle_Chunk.svgz \
	images/layout-toggle_Description.svgz \
	images/layout-toggle_Enumerate+rtl.svgz  \
	images/layout-toggle_Enumerate.svgz  \
	images/layout-toggle_Itemize.svgz \
	images/layout-toggle_Labeling.svgz \
	images/layout-toggle_List.svgz \
	images/layout-toggle_LyX-Code.svgz \
	images/layout-toggle_Section.svgz \
	images/lyx.svg \
	images/lyxfiles-system.svgz \
	images/lyxfiles-user.svgz \
	images/lyx-quit.svgz \
	images/marginalnote-insert.svgz \
	images/master-buffer-update.svgz \
	images/master-buffer-view.svgz \
	images/math-display.svgz \
	images/math-macro-add-greedy-optional-param.svgz \
	images/math-macro-add-optional-param.svgz \
	images/math-macro-add-param.svgz \
	images/math-macro-append-greedy-param.svgz \
	images/math-macro-make-nonoptional.svgz \
	images/math-macro-make-optional.svgz \
	images/math-macro-remove-greedy-param.svgz \
	images/math-macro-remove-optional-param.svgz \
	images/math-macro-remove-param.svgz \
	images/math-macro_newmacroname_newcommand.svgz \
	images/math-matrix.svgz \
	images/math-mode.svgz \
	images/math-subscript.svgz \
	images/math-superscript.svgz \
	images/nomencl-insert.svgz \
	images/note-insert.svgz \
	images/note-next.svgz \
	images/outline-down.svgz \
	images/outline-in.svgz \
	images/outline-in+rtl.svgz \
	images/outline-out.svgz \
	images/outline-out+rtl.svgz \
	images/outline-up.svgz \
	images/paste.svgz \
	images/redo.svgz \
	images/reload.svgz \
	images/script-insert_subscript.svgz \
	images/script-insert_superscript.svgz \
	images/specialchar-insert_latex.svgz \
	images/specialchar-insert_latex2e.svgz \
	images/specialchar-insert_lyx.svgz \
	images/specialchar-insert_menu-separator.svgz \
	images/specialchar-insert_tex.svgz \
	images/spelling-continuously.svgz \
	images/view-split_horizontal.svgz \
	images/view-split_vertical.svgz \
	images/tabular-feature_align-decimal.svgz \
	images/tabular-feature_append-column.svgz \
	images/tabular-feature_append-row.svgz \
	images/tabular-feature_delete-column.svgz \
	images/tabular-feature_delete-row.svgz \
	images/tabular-feature_m-align-center.svgz \
	images/tabular-feature_m-align-left.svgz \
	images/tabular-feature_m-align-right.svgz \
	images/tabular-feature_m-valign-bottom.svgz \
	images/tabular-feature_m-valign-middle.svgz \
	images/tabular-feature_m-valign-top.svgz \
	images/tabular-feature_move-column-left.svgz \
	images/tabular-feature_move-column-right.svgz \
	images/tabular-feature_move-row-down.svgz \
	images/tabular-feature_move-row-up.svgz \
	images/tabular-feature_multicolumn.svgz \
	images/tabular-feature_multirow.svgz \
	images/tabular-feature_reset-formal-default.svgz \
	images/tabular-feature_set-all-lines.svgz \
	images/tabular-feature_set-longtabular.svgz \
	images/tabular-feature_set-rotate-cell.svgz \
	images/tabular-feature_set-border-lines.svgz \
	images/tabular-feature_set-inner-lines.svgz \
	images/tabular-feature_set-rotate-tabular.svgz \
	images/tabular-feature_toggle-line-bottom.svgz \
	images/tabular-feature_toggle-line-left.svgz \
	images/tabular-feature_toggle-line-right.svgz \
	images/tabular-feature_toggle-line-top.svgz \
	images/tabular-feature_toggle-rotate-cell.svgz \
	images/tabular-feature_toggle-rotate-tabular.svgz \
	images/tabular-feature_unset-all-lines.svgz \
	images/tabular-insert.svgz \
	images/thesaurus-entry.svgz \
	images/toolbar-toggle_math.svgz \
	images/toolbar-toggle_math_panels.svgz \
	images/toolbar-toggle_review.svgz \
	images/toolbar-toggle_table.svgz \
	images/undo.svgz \
	images/unknown.svgz \
	images/up.svgz \
	images/update-others.svgz \
	images/url-insert.svgz \
	images/vc-check-in.svgz \
	images/vc-check-out.svgz \
	images/vc-compare.svgz \
	images/vc-compare_0.svgz \
	images/vc-locking-toggle.svgz \
	images/vc-register.svgz \
	images/vc-repo-update.svgz \
	images/vc-revert.svgz \
	images/view-others.svgz

# possible shortcut for complete set of 2x sized icons
#dist_images_DATA2X = \
#	${dist_images_DATA1X:%.png=%@2x.png}

dist_images_DATA2X = \
	images/banner@2x.png

dist_images_DATA = \
	images/amssymb.png \
	images/banner.png \
	images/psnfss1.png \
	images/psnfss2.png \
	images/psnfss3.png \
	images/psnfss4.png \
	images/standard.png \
	$(dist_images_DATA1X) \
	$(dist_images_DATA2X) \
	$(dist_images_DATA3X)

imagesmathdir = $(imagesdir)/math
dist_imagesmath_DATA = \
	images/math/AC.svgz \
	images/math/acute.svgz \
	images/math/adots.svgz \
	images/math/agemO.svgz \
	images/math/aleph.svgz \
	images/math/alpha.svgz \
	images/math/amalg.svgz \
	images/math/angle.svgz \
	images/math/APLbox.svgz \
	images/math/APLcomment.svgz \
	images/math/APLdown.svgz \
	images/math/APLdownarrowbox.svgz \
	images/math/APLinput.svgz \
	images/math/APLinv.svgz \
	images/math/APLleftarrowbox.svgz \
	images/math/APLlog.svgz \
	images/math/APLrightarrowbox.svgz \
	images/math/APLstar.svgz \
	images/math/APLup.svgz \
	images/math/APLuparrowbox.svgz \
	images/math/apprge.svgz \
	images/math/apprle.svgz \
	images/math/approx.svgz \
	images/math/approxeq.svgz \
	images/math/aquarius.svgz \
	images/math/aries.svgz \
	images/math/arrownot.svgz \
	images/math/arrownot2.svgz \
	images/math/arrowvert.svgz \
	images/math/arrowvert2.svgz \
	images/math/ascnode.svgz \
	images/math/ast.svgz \
	images/math/astrosun.svgz \
	images/math/asymp.svgz \
	images/math/ataribox.svgz \
	images/math/backepsilon.svgz \
	images/math/backprime.svgz \
	images/math/backsim.svgz \
	images/math/backsimeq.svgz \
	images/math/backslash.svgz \
	images/math/bar.svgz \
	images/math/baro.svgz \
	images/math/bars.svgz \
	images/math/barwedge.svgz \
	images/math/Bbbk.svgz \
	images/math/bbslash.svgz \
	images/math/bcancel.svgz \
	images/math/because.svgz \
	images/math/bell.svgz \
	images/math/beta.svgz \
	images/math/beth.svgz \
	images/math/between.svgz \
	images/math/bigbox.svgz \
	images/math/bigcap.svgz \
	images/math/bigcirc.svgz \
	images/math/bigcup.svgz \
	images/math/bigcurlyvee.svgz \
	images/math/bigcurlywedge.svgz \
	images/math/biginterleave.svgz \
	images/math/bignplus.svgz \
	images/math/bigodot.svgz \
	images/math/bigoplus.svgz \
	images/math/bigotimes.svgz \
	images/math/bigparallel.svgz \
	images/math/bigsqcap.svgz \
	images/math/bigsqcup.svgz \
	images/math/bigstar.svgz \
	images/math/bigtriangledown.svgz \
	images/math/bigtriangleup.svgz \
	images/math/biguplus.svgz \
	images/math/bigvee.svgz \
	images/math/bigwedge.svgz \
	images/math/binampersand.svgz \
	images/math/bindnasrepma.svgz \
	images/math/blacklozenge.svgz \
	images/math/blacksmiley.svgz \
	images/math/blacksquare.svgz \
	images/math/blacktriangle.svgz \
	images/math/blacktriangledown.svgz \
	images/math/blacktriangleleft.svgz \
	images/math/blacktriangleright.svgz \
	images/math/boldsymbol.svgz \
	images/math/bot.svgz \
	images/math/bowtie.svgz \
	images/math/bowtie2.svgz \
	images/math/box2.svgz \
	images/math/boxast.svgz \
	images/math/boxbar.svgz \
	images/math/boxbox.svgz \
	images/math/boxbslash.svgz \
	images/math/boxcircle.svgz \
	images/math/boxdot.svgz \
	images/math/boxempty.svgz \
	images/math/boxminus.svgz \
	images/math/boxplus.svgz \
	images/math/boxslash.svgz \
	images/math/boxtimes.svgz \
	images/math/breve.svgz \
	images/math/brokenvert.svgz \
	images/math/bullet.svgz \
	images/math/bumpeq.svgz \
	images/math/bumpeq2.svgz \
	images/math/cancel.svgz \
	images/math/cancelto.svgz \
	images/math/cancer.svgz \
	images/math/cap.svgz \
	images/math/cap2.svgz \
	images/math/capricornus.svgz \
	images/math/cases.svgz \
	images/math/cdot.svgz \
	images/math/cdotp.svgz \
	images/math/cdots.svgz \
	images/math/cent.svgz \
	images/math/centerdot.svgz \
	images/math/check.svgz \
	images/math/checked.svgz \
	images/math/checkedbox2.svgz \
	images/math/checkmark.svgz \
	images/math/chi.svgz \
	images/math/circ.svgz \
	images/math/circeq.svgz \
	images/math/circle2.svgz \
	images/math/circle3.svgz \
	images/math/circlearrowleft.svgz \
	images/math/circlearrowright.svgz \
	images/math/circledR.svgz \
	images/math/circledS.svgz \
	images/math/circledast.svgz \
	images/math/circledcirc.svgz \
	images/math/circleddash.svgz \
	images/math/clock.svgz \
	images/math/clubsuit.svgz \
	images/math/colon.svgz \
	images/math/colonapprox.svgz \
	images/math/colonapprox2.svgz \
	images/math/coloneq.svgz \
	images/math/coloneq2.svgz \
	images/math/coloneqq.svgz \
	images/math/coloneqq2.svgz \
	images/math/colonsim.svgz \
	images/math/colonsim2.svgz \
	images/math/complement.svgz \
	images/math/cong.svgz \
	images/math/conjunction.svgz \
	images/math/coprod.svgz \
	images/math/cup.svgz \
	images/math/cup2.svgz \
	images/math/curlyeqprec.svgz \
	images/math/curlyeqsucc.svgz \
	images/math/curlyvee.svgz \
	images/math/curlyveedownarrow.svgz \
	images/math/curlyveeuparrow.svgz \
	images/math/curlywedge.svgz \
	images/math/curlywedgedownarrow.svgz \
	images/math/curlywedgeuparrow.svgz \
	images/math/currency.svgz \
	images/math/curvearrowleft.svgz \
	images/math/curvearrowright.svgz \
	images/math/dagger.svgz \
	images/math/daleth.svgz \
	images/math/dasharrow.svgz \
	images/math/dashleftarrow.svgz \
	images/math/dashrightarrow.svgz \
	images/math/dashv.svgz \
	images/math/davidsstar.svgz \
	images/math/dblcolon.svgz \
	images/math/ddagger.svgz \
	images/math/ddddot.svgz \
	images/math/dddot.svgz \
	images/math/ddot.svgz \
	images/math/ddots.svgz \
	images/math/delim.svgz \
	images/math/delta.svgz \
	images/math/delta2.svgz \
	images/math/descnode.svgz \
	images/math/diagdown.svgz \
	images/math/diagup.svgz \
	images/math/diameter.svgz \
	images/math/diamond.svgz \
	images/math/diamond2.svgz \
	images/math/diamondsuit.svgz \
	images/math/digamma.svgz \
	images/math/div.svgz \
	images/math/divideontimes.svgz \
	images/math/dot.svgz \
	images/math/doteq.svgz \
	images/math/doteq2.svgz \
	images/math/doteqdot.svgz \
	images/math/dotplus.svgz \
	images/math/dots.svgz \
	images/math/dotsb.svgz \
	images/math/dotsc.svgz \
	images/math/dotsi.svgz \
	images/math/dotsint.svgz \
	images/math/dotsintop.svgz \
	images/math/dotsm.svgz \
	images/math/dotso.svgz \
	images/math/doublebarwedge.svgz \
	images/math/doublecap.svgz \
	images/math/doublecup.svgz \
	images/math/downarrow.svgz \
	images/math/downarrow2.svgz \
	images/math/downarrow3.svgz \
	images/math/downdownarrows.svgz \
	images/math/downharpoonleft.svgz \
	images/math/downharpoonright.svgz \
	images/math/earth.svgz \
	images/math/eighthnote.svgz \
	images/math/ell.svgz \
	images/math/empty.svgz \
	images/math/emptyset.svgz \
	images/math/epsilon.svgz \
	images/math/eqcirc.svgz \
	images/math/eqcolon.svgz \
	images/math/eqcolon2.svgz \
	images/math/eqqcolon.svgz \
	images/math/eqqcolon2.svgz \
	images/math/eqslantgtr.svgz \
	images/math/eqslantless.svgz \
	images/math/eqsim.svgz \
	images/math/equation.svgz \
	images/math/equiv.svgz \
	images/math/eta.svgz \
	images/math/eth.svgz \
	images/math/exists.svgz \
	images/math/export-others.svgz \
	images/math/fallingdotseq.svgz \
	images/math/fatbslash.svgz \
	images/math/fatsemi.svgz \
	images/math/fatslash.svgz \
	images/math/female.svgz \
	images/math/fint.svgz \
	images/math/fintop.svgz \
	images/math/Finv.svgz \
	images/math/flat.svgz \
	images/math/font.svgz \
	images/math/forall.svgz \
	images/math/frac.svgz \
	images/math/frac-square.svgz \
	images/math/frown.svgz \
	images/math/frownie.svgz \
	images/math/fullmoon.svgz \
	images/math/fullnote.svgz \
	images/math/functions.svgz \
	images/math/Game.svgz \
	images/math/gamma.svgz \
	images/math/gamma2.svgz \
	images/math/gemini.svgz \
	images/math/ge.svgz \
	images/math/geq.svgz \
	images/math/geqq.svgz \
	images/math/geqslant.svgz \
	images/math/gets.svgz \
	images/math/gg.svgz \
	images/math/ggg.svgz \
	images/math/gggtr.svgz \
	images/math/gimel.svgz \
	images/math/gluon.svgz \
	images/math/gnapprox.svgz \
	images/math/gneq.svgz \
	images/math/gneqq.svgz \
	images/math/gnsim.svgz \
	images/math/grave.svgz \
	images/math/gtrapprox.svgz \
	images/math/gtrdot.svgz \
	images/math/gtreqless.svgz \
	images/math/gtreqqless.svgz \
	images/math/gtrless.svgz \
	images/math/gtrsim.svgz \
	images/math/gvertneqq.svgz \
	images/math/halfnote.svgz \
	images/math/hat.svgz \
	images/math/hbar.svgz \
	images/math/heartsuit.svgz \
	images/math/hexagon.svgz \
	images/math/hexstar.svgz \
	images/math/HF.svgz \
	images/math/hookleftarrow.svgz \
	images/math/hookrightarrow.svgz \
	images/math/hphantom.svgz \
	images/math/hslash.svgz \
	images/math/iddots.svgz \
	images/math/idotsint.svgz \
	images/math/iff.svgz \
	images/math/iiiint.svgz \
	images/math/iiiintop.svgz \
	images/math/iiint.svgz \
	images/math/iiintop.svgz \
	images/math/iint.svgz \
	images/math/iintop.svgz \
	images/math/Im.svgz \
	images/math/imath.svgz \
	images/math/impliedby.svgz \
	images/math/implies.svgz \
	images/math/in.svgz \
	images/math/infty.svgz \
	images/math/inplus.svgz \
	images/math/int.svgz \
	images/math/intercal.svgz \
	images/math/interleave.svgz \
	images/math/intop.svgz \
	images/math/iota.svgz \
	images/math/invdiameter.svgz \
	images/math/inve.svgz \
	images/math/invneg.svgz \
	images/math/jmath.svgz \
	images/math/join2.svgz \
	images/math/jupiter.svgz \
	images/math/kappa.svgz \
	images/math/kreuz.svgz \
	images/math/lambda.svgz \
	images/math/lambda2.svgz \
	images/math/land.svgz \
	images/math/landdownint.svgz \
	images/math/landdownintop.svgz \
	images/math/landupint.svgz \
	images/math/landupintop.svgz \
	images/math/langle.svgz \
	images/math/lbag.svgz \
	images/math/lbag2.svgz \
	images/math/lbrace.svgz \
	images/math/lbrace_rbrace.svgz \
	images/math/lbracket.svgz \
	images/math/lbracket_rbracket.svgz \
	images/math/lceil.svgz \
	images/math/lceil_rceil.svgz \
	images/math/ldotp.svgz \
	images/math/ldots.svgz \
	images/math/le.svgz \
	images/math/leadsto.svgz \
	images/math/leftarrow.svgz \
	images/math/leftarrow2.svgz \
	images/math/leftarrow3.svgz \
	images/math/leftarrowtail.svgz \
	images/math/leftarrowtriangle.svgz \
	images/math/leftcircle2.svgz \
	images/math/leftcircle3.svgz \
	images/math/leftcircle4.svgz \
	images/math/leftharpoondown.svgz \
	images/math/leftharpoonup.svgz \
	images/math/leftleftarrows.svgz \
	images/math/leftmoon.svgz \
	images/math/leftrightarrow.svgz \
	images/math/leftrightarrow2.svgz \
	images/math/leftrightarroweq.svgz \
	images/math/leftrightarrows.svgz \
	images/math/leftrightarrowtriangle.svgz \
	images/math/leftrightharpoons.svgz \
	images/math/leftrightsquigarrow.svgz \
	images/math/leftslice.svgz \
	images/math/leftthreetimes.svgz \
	images/math/leftturn.svgz \
	images/math/leo.svgz \
	images/math/leq.svgz \
	images/math/leqq.svgz \
	images/math/leqslant.svgz \
	images/math/lessapprox.svgz \
	images/math/lessdot.svgz \
	images/math/lesseqgtr.svgz \
	images/math/lesseqqgtr.svgz \
	images/math/lessgtr.svgz \
	images/math/lesssim.svgz \
	images/math/lfloor.svgz \
	images/math/lfloor_rfloor.svgz \
	images/math/lhd.svgz \
	images/math/lhd2.svgz \
	images/math/lhook.svgz \
	images/math/libra.svgz \
	images/math/lightning.svgz \
	images/math/ll.svgz \
	images/math/llbracket.svgz \
	images/math/llceil.svgz \
	images/math/llcorner.svgz \
	images/math/llfloor.svgz \
	images/math/llparenthesis.svgz \
	images/math/Lleftarrow.svgz \
	images/math/lll.svgz \
	images/math/llless.svgz \
	images/math/lnapprox.svgz \
	images/math/lneq.svgz \
	images/math/lneqq.svgz \
	images/math/lnot.svgz \
	images/math/lnsim.svgz \
	images/math/logof.svgz \
	images/math/longarrownot.svgz \
	images/math/longarrownot2.svgz \
	images/math/longleftarrow.svgz \
	images/math/longleftarrow2.svgz \
	images/math/longleftrightarrow.svgz \
	images/math/longleftrightarrow2.svgz \
	images/math/longmapsfrom.svgz \
	images/math/longmapsfrom2.svgz \
	images/math/longmapsto.svgz \
	images/math/longmapsto2.svgz \
	images/math/longrightarrow.svgz \
	images/math/longrightarrow2.svgz \
	images/math/looparrowleft.svgz \
	images/math/looparrowright.svgz \
	images/math/lor.svgz \
	images/math/lozenge.svgz \
	images/math/lparen.svgz \
	images/math/lparen_rparen.svgz \
	images/math/lrcorner.svgz \
	images/math/Lsh.svgz \
	images/math/ltimes.svgz \
	images/math/lvertneqq.svgz \
	images/math/male.svgz \
	images/math/maltese.svgz \
	images/math/mapsfrom.svgz \
	images/math/mapsfrom2.svgz \
	images/math/mapsfromchar.svgz \
	images/math/mapsfromchar2.svgz \
	images/math/mapsto.svgz \
	images/math/mapsto2.svgz \
	images/math/mapstochar.svgz \
	images/math/mapstochar2.svgz \
	images/math/mars.svgz \
	images/math/mathbb.svgz \
	images/math/mathbb_C.svgz \
	images/math/mathbb_H.svgz \
	images/math/mathbb_N.svgz \
	images/math/mathbb_Q.svgz \
	images/math/mathbb_R.svgz \
	images/math/mathbb_Z.svgz \
	images/math/mathbin.svgz \
	images/math/mathbf.svgz \
	images/math/mathcal.svgz \
	images/math/mathcal_F.svgz \
	images/math/mathcal_H.svgz \
	images/math/mathcal_L.svgz \
	images/math/mathcal_O.svgz \
	images/math/mathcircumflex.svgz \
	images/math/mathclap.svgz \
	images/math/mathdollar.svgz \
	images/math/mathfrak.svgz \
	images/math/mathit.svgz \
	images/math/mathllap.svgz \
	images/math/mathop.svgz \
	images/math/mathord.svgz \
	images/math/mathparagraph.svgz \
	images/math/mathscr.svgz \
	images/math/mathsection.svgz \
	images/math/mathrel.svgz \
	images/math/mathring.svgz \
	images/math/mathrlap.svgz \
	images/math/mathrm.svgz \
	images/math/mathrm_T.svgz \
	images/math/mathsf.svgz \
	images/math/mathtt.svgz \
	images/math/matrix.svgz \
	images/math/measuredangle.svgz \
	images/math/mercury.svgz \
	images/math/merge.svgz \
	images/math/mho.svgz \
	images/math/mid.svgz \
	images/math/minuso.svgz \
	images/math/models.svgz \
	images/math/moo.svgz \
	images/math/mp.svgz \
	images/math/mu.svgz \
	images/math/multimap.svgz \
	images/math/nabla.svgz \
	images/math/natural.svgz \
	images/math/ncong.svgz \
	images/math/ne.svgz \
	images/math/nearrow.svgz \
	images/math/neg.svgz \
	images/math/neptune.svgz \
	images/math/neq.svgz \
	images/math/newmoon.svgz \
	images/math/nexists.svgz \
	images/math/ngeq.svgz \
	images/math/ngeqq.svgz \
	images/math/ngeqslant.svgz \
	images/math/ngtr.svgz \
	images/math/ni.svgz \
	images/math/niplus.svgz \
	images/math/nleftarrow.svgz \
	images/math/nleftarrow2.svgz \
	images/math/nleftrightarrow.svgz \
	images/math/nleftrightarrow2.svgz \
	images/math/nleq.svgz \
	images/math/nleqq.svgz \
	images/math/nleqslant.svgz \
	images/math/nless.svgz \
	images/math/nmid.svgz \
	images/math/nnearrow.svgz \
	images/math/nnwarrow.svgz \
	images/math/not.svgz \
	images/math/notin.svgz \
	images/math/nparallel.svgz \
	images/math/nplus.svgz \
	images/math/nprec.svgz \
	images/math/npreceq.svgz \
	images/math/nrightarrow.svgz \
	images/math/nrightarrow2.svgz \
	images/math/nshortmid.svgz \
	images/math/nshortparallel.svgz \
	images/math/nsim.svgz \
	images/math/nsubseteq.svgz \
	images/math/nsubseteqq.svgz \
	images/math/nsucc.svgz \
	images/math/nsucceq.svgz \
	images/math/nsupseteq.svgz \
	images/math/nsupseteqq.svgz \
	images/math/ntriangleleft.svgz \
	images/math/ntrianglelefteq.svgz \
	images/math/ntrianglelefteqslant.svgz \
	images/math/ntriangleright.svgz \
	images/math/ntrianglerighteq.svgz \
	images/math/ntrianglerighteqslant.svgz \
	images/math/nu.svgz \
	images/math/nvdash.svgz \
	images/math/nvdash2.svgz \
	images/math/nvdash3.svgz \
	images/math/nvdash4.svgz \
	images/math/nwarrow.svgz \
	images/math/obar.svgz \
	images/math/oblong.svgz \
	images/math/obslash.svgz \
	images/math/ocircle.svgz \
	images/math/octagon.svgz \
	images/math/odot.svgz \
	images/math/ogreaterthan.svgz \
	images/math/oiint.svgz \
	images/math/oiintop.svgz \
	images/math/oint.svgz \
	images/math/ointclockwise.svgz \
	images/math/ointclockwiseop.svgz \
	images/math/ointctrclockwise.svgz \
	images/math/ointctrclockwiseop.svgz \
	images/math/ointop.svgz \
	images/math/olessthan.svgz \
	images/math/omega.svgz \
	images/math/omega2.svgz \
	images/math/ominus.svgz \
	images/math/openo.svgz \
	images/math/oplus.svgz \
	images/math/opposition.svgz \
	images/math/oslash.svgz \
	images/math/otimes.svgz \
	images/math/ovee.svgz \
	images/math/overbrace.svgz \
	images/math/overleftarrow.svgz \
	images/math/overleftrightarrow.svgz \
	images/math/overline.svgz \
	images/math/overrightarrow.svgz \
	images/math/overset.svgz \
	images/math/owedge.svgz \
	images/math/owns.svgz \
	images/math/parallel.svgz \
	images/math/partial.svgz \
	images/math/pentagon.svgz \
	images/math/permil.svgz \
	images/math/perp.svgz \
	images/math/phantom.svgz \
	images/math/phi.svgz \
	images/math/phi2.svgz \
	images/math/phone.svgz \
	images/math/photon.svgz \
	images/math/pi.svgz \
	images/math/pi2.svgz \
	images/math/pisces.svgz \
	images/math/pitchfork.svgz \
	images/math/pluto.svgz \
	images/math/pm.svgz \
	images/math/pointer.svgz \
	images/math/prec.svgz \
	images/math/precapprox.svgz \
	images/math/preccurlyeq.svgz \
	images/math/preceq.svgz \
	images/math/precnapprox.svgz \
	images/math/precneqq.svgz \
	images/math/precnsim.svgz \
	images/math/precsim.svgz \
	images/math/prime.svgz \
	images/math/prod.svgz \
	images/math/propto.svgz \
	images/math/psi.svgz \
	images/math/psi2.svgz \
	images/math/quarternote.svgz \
	images/math/rangle.svgz \
	images/math/rbag.svgz \
	images/math/rbag2.svgz \
	images/math/rbrace.svgz \
	images/math/rbracket.svgz \
	images/math/rceil.svgz \
	images/math/Re.svgz \
	images/math/recorder.svgz \
	images/math/restriction.svgz \
	images/math/rfloor.svgz \
	images/math/rhd.svgz \
	images/math/rhd2.svgz \
	images/math/rho.svgz \
	images/math/rhook.svgz \
	images/math/rightarrow.svgz \
	images/math/rightarrow2.svgz \
	images/math/rightarrow3.svgz \
	images/math/rightarrowtail.svgz \
	images/math/rightarrowtriangle.svgz \
	images/math/rightcircle2.svgz \
	images/math/rightcircle3.svgz \
	images/math/rightcircle4.svgz \
	images/math/rightharpoondown.svgz \
	images/math/rightharpoonup.svgz \
	images/math/rightleftarrows.svgz \
	images/math/rightleftharpoons.svgz \
	images/math/rightmoon.svgz \
	images/math/rightrightarrows.svgz \
	images/math/rightslice.svgz \
	images/math/rightsquigarrow.svgz \
	images/math/rightthreetimes.svgz \
	images/math/rightturn.svgz \
	images/math/risingdotseq.svgz \
	images/math/root.svgz \
	images/math/rparen.svgz \
	images/math/rrbracket.svgz \
	images/math/rrceil.svgz \
	images/math/rrfloor.svgz \
	images/math/rrparenthesis.svgz \
	images/math/Rrightarrow.svgz \
	images/math/Rsh.svgz \
	images/math/rtimes.svgz \
	images/math/sagittarius.svgz \
	images/math/saturn.svgz \
	images/math/scorpio.svgz \
	images/math/searrow.svgz \
	images/math/setminus.svgz \
	images/math/sharp.svgz \
	images/math/shortdownarrow.svgz \
	images/math/shortleftarrow.svgz \
	images/math/shortmid.svgz \
	images/math/shortparallel.svgz \
	images/math/shortrightarrow.svgz \
	images/math/shortuparrow.svgz \
	images/math/sideset.svgz \
	images/math/sidesetl.svgz \
	images/math/sidesetn.svgz \
	images/math/sidesetr.svgz \
	images/math/sigma.svgz \
	images/math/sigma2.svgz \
	images/math/sim.svgz \
	images/math/simeq.svgz \
	images/math/slash.svgz \
	images/math/smallfrown.svgz \
	images/math/smallint.svgz \
	images/math/smallsetminus.svgz \
	images/math/smallsmile.svgz \
	images/math/smash.svgz \
	images/math/smashb.svgz \
	images/math/smasht.svgz \
	images/math/smile.svgz \
	images/math/smiley.svgz \
	images/math/space.svgz \
	images/math/spadesuit.svgz \
	images/math/sphericalangle.svgz \
	images/math/square2.svgz \
	images/math/sqcap.svgz \
	images/math/sqcup.svgz \
	images/math/sqiint.svgz \
	images/math/sqiintop.svgz \
	images/math/sqint.svgz \
	images/math/sqintop.svgz \
	images/math/sqrt.svgz \
	images/math/sqrt-square.svgz \
	images/math/sqsubset.svgz \
	images/math/sqsubseteq.svgz \
	images/math/sqsupset.svgz \
	images/math/sqsupseteq.svgz \
	images/math/square.svgz \
	images/math/ssearrow.svgz \
	images/math/sslash.svgz \
	images/math/sswarrow.svgz \
	images/math/stackrel.svgz \
	images/math/stackrelthree.svgz \
	images/math/star.svgz \
	images/math/style.svgz \
	images/math/sub.svgz \
	images/math/subset.svgz \
	images/math/subset2.svgz \
	images/math/subseteq.svgz \
	images/math/subseteqq.svgz \
	images/math/subsetneq.svgz \
	images/math/subsetneqq.svgz \
	images/math/subsetplus.svgz \
	images/math/subsetpluseq.svgz \
	images/math/succ.svgz \
	images/math/succapprox.svgz \
	images/math/succcurlyeq.svgz \
	images/math/succeq.svgz \
	images/math/succnapprox.svgz \
	images/math/succneqq.svgz \
	images/math/succnsim.svgz \
	images/math/succsim.svgz \
	images/math/sum.svgz \
	images/math/sun.svgz \
	images/math/super.svgz \
	images/math/supset.svgz \
	images/math/supset2.svgz \
	images/math/supseteq.svgz \
	images/math/supseteqq.svgz \
	images/math/supsetneq.svgz \
	images/math/supsetneqq.svgz \
	images/math/supsetplus.svgz \
	images/math/supsetpluseq.svgz \
	images/math/surd.svgz \
	images/math/swarrow.svgz \
	images/math/talloblong.svgz \
	images/math/tau.svgz \
	images/math/taurus.svgz \
	images/math/textrm_AA.svgz \
	images/math/textrm_O.svgz \
	images/math/therefore.svgz \
	images/math/theta.svgz \
	images/math/theta2.svgz \
	images/math/textdegree.svgz \
	images/math/textrm.svgz \
	images/math/thickapprox.svgz \
	images/math/thicksim.svgz \
	images/math/thorn.svgz \
	images/math/thorn2.svgz \
	images/math/tilde.svgz \
	images/math/times.svgz \
	images/math/to.svgz \
	images/math/top.svgz \
	images/math/triangle.svgz \
	images/math/triangledown.svgz \
	images/math/triangleleft.svgz \
	images/math/trianglelefteq.svgz \
	images/math/trianglelefteqslant.svgz \
	images/math/triangleq.svgz \
	images/math/triangleright.svgz \
	images/math/trianglerighteq.svgz \
	images/math/trianglerighteqslant.svgz \
	images/math/twoheadleftarrow.svgz \
	images/math/twoheadrightarrow.svgz \
	images/math/twonotes.svgz \
	images/math/ulcorner.svgz \
	images/math/underbar.svgz \
	images/math/underbrace.svgz \
	images/math/underleftarrow.svgz \
	images/math/underleftrightarrow.svgz \
	images/math/underline.svgz \
	images/math/underrightarrow.svgz \
	images/math/underscore.svgz \
	images/math/underset.svgz \
	images/math/undertilde.svgz \
	images/math/unlhd.svgz \
	images/math/unrhd.svgz \
	images/math/uparrow.svgz \
	images/math/uparrow2.svgz \
	images/math/uparrow3.svgz \
	images/math/updownarrow.svgz \
	images/math/updownarrow2.svgz \
	images/math/upharpoonleft.svgz \
	images/math/upharpoonright.svgz \
	images/math/uplus.svgz \
	images/math/upsilon.svgz \
	images/math/upsilon2.svgz \
	images/math/upuparrows.svgz \
	images/math/uranus.svgz \
	images/math/urcorner.svgz \
	images/math/utilde.svgz \
	images/math/varangle.svgz \
	images/math/varbigcirc.svgz \
	images/math/varcopyright.svgz \
	images/math/varcurlyvee.svgz \
	images/math/varcurlywedge.svgz \
	images/math/vargamma2.svgz \
	images/math/vardelta2.svgz \
	images/math/varepsilon.svgz \
	images/math/varhexagon.svgz \
	images/math/varhexstar.svgz \
	images/math/variiint.svgz \
	images/math/variiintop.svgz \
	images/math/variint.svgz \
	images/math/variintop.svgz \
	images/math/varint.svgz \
	images/math/varintop.svgz \
	images/math/varkappa.svgz \
	images/math/varlambda2.svgz \
	images/math/varnothing.svgz \
	images/math/varoast.svgz \
	images/math/varobar.svgz \
	images/math/varobslash.svgz \
	images/math/varocircle.svgz \
	images/math/varodot.svgz \
	images/math/varogreaterthan.svgz \
	images/math/varoiint.svgz \
	images/math/varoiintop.svgz \
	images/math/varoint.svgz \
	images/math/varointop.svgz \
	images/math/varointclockwise.svgz \
	images/math/varointclockwiseop.svgz \
	images/math/varointctrclockwise.svgz \
	images/math/varointctrclockwiseop.svgz \
	images/math/varolessthan.svgz \
	images/math/varomega2.svgz \
	images/math/varominus.svgz \
	images/math/varoplus.svgz \
	images/math/varoslash.svgz \
	images/math/varotimes.svgz \
	images/math/varovee.svgz \
	images/math/varowedge.svgz \
	images/math/varphi.svgz \
	images/math/varphi2.svgz \
	images/math/varpi.svgz \
	images/math/varpi2.svgz \
	images/math/varpropto.svgz \
	images/math/varpsi2.svgz \
	images/math/varrho.svgz \
	images/math/varsigma.svgz \
	images/math/varsigma2.svgz \
	images/math/varsubsetneq.svgz \
	images/math/varsubsetneqq.svgz \
	images/math/varsupsetneq.svgz \
	images/math/varsupsetneqq.svgz \
	images/math/vartheta.svgz \
	images/math/vartheta2.svgz \
	images/math/vartimes.svgz \
	images/math/vartriangle.svgz \
	images/math/vartriangleleft.svgz \
	images/math/vartriangleright.svgz \
	images/math/varupsilon2.svgz \
	images/math/varxi2.svgz \
	images/math/vcentcolon.svgz \
	images/math/vdash.svgz \
	images/math/vdash2.svgz \
	images/math/vdash3.svgz \
	images/math/vdots.svgz \
	images/math/vec.svgz \
	images/math/vee.svgz \
	images/math/veebar.svgz \
	images/math/venus.svgz \
	images/math/vernal.svgz \
	images/math/vert.svgz \
	images/math/vert2.svgz \
	images/math/VHF.svgz \
	images/math/virgo.svgz \
	images/math/vphantom.svgz \
	images/math/Vvdash.svgz \
	images/math/wasylozenge.svgz \
	images/math/wasypropto.svgz \
	images/math/wasytherefore.svgz \
	images/math/wedge.svgz \
	images/math/widehat.svgz \
	images/math/widetilde.svgz \
	images/math/wp.svgz \
	images/math/wr.svgz \
	images/math/xbox3.svgz \
	images/math/xcancel.svgz \
	images/math/xi.svgz \
	images/math/xi2.svgz \
	images/math/xleftarrow.svgz \
	images/math/xrightarrow.svgz \
	images/math/Ydown.svgz \
	images/math/yen.svgz \
	images/math/Yleft.svgz \
	images/math/Yright.svgz \
	images/math/Yup.svgz \
	images/math/zeta.svgz

imagesipadir = $(imagesdir)/ipa
dist_imagesipa_DATA = \
	images/ipa/ipamacro-insert_deco_bottomtiebar.svgz \
	images/ipa/ipamacro-insert_deco_toptiebar.svgz \
	images/ipa/ipamacro-insert_tone-falling.svgz \
	images/ipa/ipamacro-insert_tone-high-rising-falling.svgz \
	images/ipa/ipamacro-insert_tone-high-rising.svgz \
	images/ipa/ipamacro-insert_tone-low-rising.svgz \
	images/ipa/ipamacro-insert_tone-rising.svgz \
	images/ipa/unicode-insert_0x002e.svgz \
	images/ipa/unicode-insert_0x0061.svgz \
	images/ipa/unicode-insert_0x0062.svgz \
	images/ipa/unicode-insert_0x0063.svgz \
	images/ipa/unicode-insert_0x0064.svgz \
	images/ipa/unicode-insert_0x0065.svgz \
	images/ipa/unicode-insert_0x0066.svgz \
	images/ipa/unicode-insert_0x0067.svgz \
	images/ipa/unicode-insert_0x0068.svgz \
	images/ipa/unicode-insert_0x0069.svgz \
	images/ipa/unicode-insert_0x006a.svgz \
	images/ipa/unicode-insert_0x006b.svgz \
	images/ipa/unicode-insert_0x006c.svgz \
	images/ipa/unicode-insert_0x006d.svgz \
	images/ipa/unicode-insert_0x006e.svgz \
	images/ipa/unicode-insert_0x006f.svgz \
	images/ipa/unicode-insert_0x0070.svgz \
	images/ipa/unicode-insert_0x0071.svgz \
	images/ipa/unicode-insert_0x0072.svgz \
	images/ipa/unicode-insert_0x0073.svgz \
	images/ipa/unicode-insert_0x0074.svgz \
	images/ipa/unicode-insert_0x0075.svgz \
	images/ipa/unicode-insert_0x0076.svgz \
	images/ipa/unicode-insert_0x0077.svgz \
	images/ipa/unicode-insert_0x0078.svgz \
	images/ipa/unicode-insert_0x0079.svgz \
	images/ipa/unicode-insert_0x007a.svgz \
	images/ipa/unicode-insert_0x007c.svgz \
	images/ipa/unicode-insert_0x00e6.svgz \
	images/ipa/unicode-insert_0x00e7.svgz \
	images/ipa/unicode-insert_0x00f0.svgz \
	images/ipa/unicode-insert_0x00f8.svgz \
	images/ipa/unicode-insert_0x0127.svgz \
	images/ipa/unicode-insert_0x014b.svgz \
	images/ipa/unicode-insert_0x0153.svgz \
	images/ipa/unicode-insert_0x01c0.svgz \
	images/ipa/unicode-insert_0x01c1.svgz \
	images/ipa/unicode-insert_0x01c2.svgz \
	images/ipa/unicode-insert_0x01c3.svgz \
	images/ipa/unicode-insert_0x0250.svgz \
	images/ipa/unicode-insert_0x0251.svgz \
	images/ipa/unicode-insert_0x0252.svgz \
	images/ipa/unicode-insert_0x0253.svgz \
	images/ipa/unicode-insert_0x0254.svgz \
	images/ipa/unicode-insert_0x0255.svgz \
	images/ipa/unicode-insert_0x0256.svgz \
	images/ipa/unicode-insert_0x0257.svgz \
	images/ipa/unicode-insert_0x0258.svgz \
	images/ipa/unicode-insert_0x0259.svgz \
	images/ipa/unicode-insert_0x025a.svgz \
	images/ipa/unicode-insert_0x025b.svgz \
	images/ipa/unicode-insert_0x025c.svgz \
	images/ipa/unicode-insert_0x025d.svgz \
	images/ipa/unicode-insert_0x025e.svgz \
	images/ipa/unicode-insert_0x025f.svgz \
	images/ipa/unicode-insert_0x0260.svgz \
	images/ipa/unicode-insert_0x0261.svgz \
	images/ipa/unicode-insert_0x0262.svgz \
	images/ipa/unicode-insert_0x0263.svgz \
	images/ipa/unicode-insert_0x0264.svgz \
	images/ipa/unicode-insert_0x0265.svgz \
	images/ipa/unicode-insert_0x0266.svgz \
	images/ipa/unicode-insert_0x0267.svgz \
	images/ipa/unicode-insert_0x0268.svgz \
	images/ipa/unicode-insert_0x0269.svgz \
	images/ipa/unicode-insert_0x026a.svgz \
	images/ipa/unicode-insert_0x026b.svgz \
	images/ipa/unicode-insert_0x026c.svgz \
	images/ipa/unicode-insert_0x026d.svgz \
	images/ipa/unicode-insert_0x026e.svgz \
	images/ipa/unicode-insert_0x026f.svgz \
	images/ipa/unicode-insert_0x0270.svgz \
	images/ipa/unicode-insert_0x0271.svgz \
	images/ipa/unicode-insert_0x0272.svgz \
	images/ipa/unicode-insert_0x0273.svgz \
	images/ipa/unicode-insert_0x0274.svgz \
	images/ipa/unicode-insert_0x0275.svgz \
	images/ipa/unicode-insert_0x0276.svgz \
	images/ipa/unicode-insert_0x0277.svgz \
	images/ipa/unicode-insert_0x0278.svgz \
	images/ipa/unicode-insert_0x0279.svgz \
	images/ipa/unicode-insert_0x027a.svgz \
	images/ipa/unicode-insert_0x027b.svgz \
	images/ipa/unicode-insert_0x027c.svgz \
	images/ipa/unicode-insert_0x027d.svgz \
	images/ipa/unicode-insert_0x027e.svgz \
	images/ipa/unicode-insert_0x027f.svgz \
	images/ipa/unicode-insert_0x0280.svgz \
	images/ipa/unicode-insert_0x0281.svgz \
	images/ipa/unicode-insert_0x0282.svgz \
	images/ipa/unicode-insert_0x0283.svgz \
	images/ipa/unicode-insert_0x0284.svgz \
	images/ipa/unicode-insert_0x0285.svgz \
	images/ipa/unicode-insert_0x0286.svgz \
	images/ipa/unicode-insert_0x0287.svgz \
	images/ipa/unicode-insert_0x0288.svgz \
	images/ipa/unicode-insert_0x0289.svgz \
	images/ipa/unicode-insert_0x028a.svgz \
	images/ipa/unicode-insert_0x028b.svgz \
	images/ipa/unicode-insert_0x028c.svgz \
	images/ipa/unicode-insert_0x028d.svgz \
	images/ipa/unicode-insert_0x028e.svgz \
	images/ipa/unicode-insert_0x028f.svgz \
	images/ipa/unicode-insert_0x0290.svgz \
	images/ipa/unicode-insert_0x0291.svgz \
	images/ipa/unicode-insert_0x0292.svgz \
	images/ipa/unicode-insert_0x0293.svgz \
	images/ipa/unicode-insert_0x0294.svgz \
	images/ipa/unicode-insert_0x0295.svgz \
	images/ipa/unicode-insert_0x0296.svgz \
	images/ipa/unicode-insert_0x0297.svgz \
	images/ipa/unicode-insert_0x0298.svgz \
	images/ipa/unicode-insert_0x0299.svgz \
	images/ipa/unicode-insert_0x029a.svgz \
	images/ipa/unicode-insert_0x029b.svgz \
	images/ipa/unicode-insert_0x029c.svgz \
	images/ipa/unicode-insert_0x029d.svgz \
	images/ipa/unicode-insert_0x029e.svgz \
	images/ipa/unicode-insert_0x029f.svgz \
	images/ipa/unicode-insert_0x02a0.svgz \
	images/ipa/unicode-insert_0x02a1.svgz \
	images/ipa/unicode-insert_0x02a2.svgz \
	images/ipa/unicode-insert_0x02a3.svgz \
	images/ipa/unicode-insert_0x02a4.svgz \
	images/ipa/unicode-insert_0x02a5.svgz \
	images/ipa/unicode-insert_0x02a6.svgz \
	images/ipa/unicode-insert_0x02a7.svgz \
	images/ipa/unicode-insert_0x02a8.svgz \
	images/ipa/unicode-insert_0x02a9.svgz \
	images/ipa/unicode-insert_0x02aA.svgz \
	images/ipa/unicode-insert_0x02ab.svgz \
	images/ipa/unicode-insert_0x02ac.svgz \
	images/ipa/unicode-insert_0x02ad.svgz \
	images/ipa/unicode-insert_0x02b0.svgz \
	images/ipa/unicode-insert_0x02b2.svgz \
	images/ipa/unicode-insert_0x02b7.svgz \
	images/ipa/unicode-insert_0x02bc.svgz \
	images/ipa/unicode-insert_0x02c8.svgz \
	images/ipa/unicode-insert_0x02cc.svgz \
	images/ipa/unicode-insert_0x02d0.svgz \
	images/ipa/unicode-insert_0x02d1.svgz \
	images/ipa/unicode-insert_0x02de.svgz \
	images/ipa/unicode-insert_0x02e0.svgz \
	images/ipa/unicode-insert_0x02e1.svgz \
	images/ipa/unicode-insert_0x02e4.svgz \
	images/ipa/unicode-insert_0x02e5.svgz \
	images/ipa/unicode-insert_0x02e6.svgz \
	images/ipa/unicode-insert_0x02e7.svgz \
	images/ipa/unicode-insert_0x02e8.svgz \
	images/ipa/unicode-insert_0x02e9.svgz \
	images/ipa/unicode-insert_0x02fa.svgz \
	images/ipa/unicode-insert_0x0300.svgz \
	images/ipa/unicode-insert_0x0301.svgz \
	images/ipa/unicode-insert_0x0302.svgz \
	images/ipa/unicode-insert_0x0303.svgz \
	images/ipa/unicode-insert_0x0304.svgz \
	images/ipa/unicode-insert_0x0306.svgz \
	images/ipa/unicode-insert_0x0308.svgz \
	images/ipa/unicode-insert_0x030a.svgz \
	images/ipa/unicode-insert_0x030b.svgz \
	images/ipa/unicode-insert_0x030c.svgz \
	images/ipa/unicode-insert_0x030f.svgz \
	images/ipa/unicode-insert_0x0318.svgz \
	images/ipa/unicode-insert_0x0319.svgz \
	images/ipa/unicode-insert_0x031c.svgz \
	images/ipa/unicode-insert_0x031d.svgz \
	images/ipa/unicode-insert_0x031e.svgz \
	images/ipa/unicode-insert_0x031f.svgz \
	images/ipa/unicode-insert_0x0320.svgz \
	images/ipa/unicode-insert_0x0324.svgz \
	images/ipa/unicode-insert_0x0325.svgz \
	images/ipa/unicode-insert_0x0329.svgz \
	images/ipa/unicode-insert_0x032a.svgz \
	images/ipa/unicode-insert_0x032c.svgz \
	images/ipa/unicode-insert_0x032f.svgz \
	images/ipa/unicode-insert_0x0330.svgz \
	images/ipa/unicode-insert_0x0334.svgz \
	images/ipa/unicode-insert_0x0339.svgz \
	images/ipa/unicode-insert_0x033a.svgz \
	images/ipa/unicode-insert_0x033b.svgz \
	images/ipa/unicode-insert_0x033c.svgz \
	images/ipa/unicode-insert_0x033d.svgz \
	images/ipa/unicode-insert_0x03b2.svgz \
	images/ipa/unicode-insert_0x03b8.svgz \
	images/ipa/unicode-insert_0x03c7.svgz \
	images/ipa/unicode-insert_0x1dc4.svgz \
	images/ipa/unicode-insert_0x1dc5.svgz \
	images/ipa/unicode-insert_0x1dc8.svgz \
	images/ipa/unicode-insert_0x2016.svgz \
	images/ipa/unicode-insert_0x203f.svgz \
	images/ipa/unicode-insert_0x207f.svgz \
	images/ipa/unicode-insert_0x2197.svgz \
	images/ipa/unicode-insert_0x2198.svgz \
	images/ipa/unicode-insert_0x2c71.svgz \
	images/ipa/unicode-insert_0xa71b.svgz \
	images/ipa/unicode-insert_0xa71c.svgz

imagesoxygendir = $(imagesdir)/oxygen
dist_imagesoxygen_DATA1X = \
	images/oxygen/all-changes-accept.svgz \
	images/oxygen/all-changes-reject.svgz \
	images/oxygen/bookmark-goto.svgz \
	images/oxygen/bookmark-goto_0.svgz \
	images/oxygen/bookmark-save.svgz \
	images/oxygen/box-insert.svgz \
	images/oxygen/break-line.svgz \
	images/oxygen/buffer-close.svgz \
	images/oxygen/buffer-export.svgz \
	images/oxygen/buffer-export_dvi.svgz \
	images/oxygen/buffer-export_dvi3.svgz \
	images/oxygen/buffer-export_latex.svgz \
	images/oxygen/buffer-export_pdf.svgz \
	images/oxygen/buffer-export_pdf2.svgz \
	images/oxygen/buffer-export_pdf3.svgz \
	images/oxygen/buffer-export_pdf4.svgz \
	images/oxygen/buffer-export_pdf5.svgz \
	images/oxygen/buffer-export_ps.svgz \
	images/oxygen/buffer-export_text.svgz \
	images/oxygen/buffer-new.svgz \
	images/oxygen/buffer-reload.svgz \
	images/oxygen/buffer-toggle-output-sync.svgz \
	images/oxygen/buffer-update.svgz \
	images/oxygen/buffer-update_dvi.svgz \
	images/oxygen/buffer-update_dvi3.svgz \
	images/oxygen/buffer-update_pdf.svgz \
	images/oxygen/buffer-update_pdf2.svgz \
	images/oxygen/buffer-update_pdf3.svgz \
	images/oxygen/buffer-update_pdf4.svgz \
	images/oxygen/buffer-update_pdf5.svgz \
	images/oxygen/buffer-update_ps.svgz \
	images/oxygen/buffer-view.svgz \
	images/oxygen/buffer-view_dvi.svgz \
	images/oxygen/buffer-view_dvi3.svgz \
	images/oxygen/buffer-view_pdf.svgz \
	images/oxygen/buffer-view_pdf2.svgz \
	images/oxygen/buffer-view_pdf3.svgz \
	images/oxygen/buffer-view_pdf4.svgz \
	images/oxygen/buffer-view_pdf5.svgz \
	images/oxygen/buffer-view_ps.svgz \
	images/oxygen/buffer-write-as.svgz \
	images/oxygen/buffer-write.svgz \
	images/oxygen/build-program.svgz \
	images/oxygen/change-accept.svgz \
	images/oxygen/change-next.svgz \
	images/oxygen/change-reject.svgz \
	images/oxygen/changes-merge.svgz \
	images/oxygen/changes-output.svgz \
	images/oxygen/changes-track.svgz \
	images/oxygen/closetab.svgz \
	images/oxygen/command-alternatives_outline-in_depth-increment.svgz \
	images/oxygen/command-alternatives_outline-out_depth-decrement.svgz \
	images/oxygen/copy.svgz \
	images/oxygen/cut.svgz \
	images/oxygen/depth-decrement.svgz \
	images/oxygen/depth-increment.svgz \
	images/oxygen/dialog-preferences.svgz \
	images/oxygen/dialog-show-new-inset_citation.svgz \
	images/oxygen/dialog-show-new-inset_graphics.svgz \
	images/oxygen/dialog-show-new-inset_include.svgz \
	images/oxygen/dialog-show-new-inset_ref.svgz \
	images/oxygen/dialog-show_character.svgz \
	images/oxygen/dialog-show_findreplace.svgz \
	images/oxygen/dialog-show_mathdelimiter.svgz \
	images/oxygen/dialog-show_mathmatrix.svgz \
	images/oxygen/dialog-show_print.svgz \
	images/oxygen/dialog-show_spellchecker.svgz \
	images/oxygen/dialog-show_vclog.svgz \
	images/oxygen/dialog-toggle_findreplaceadv.svgz \
	images/oxygen/dialog-toggle_toc.svgz \
	images/oxygen/dynamic-char-styles.svgz \
	images/oxygen/dynamic-custom-insets.svgz \
	images/oxygen/down.svgz \
	images/oxygen/editclear.svgz \
	images/oxygen/ert-insert.svgz \
	images/oxygen/file-open.svgz \
	images/oxygen/float-insert_figure.svgz \
	images/oxygen/float-insert_table.svgz \
	images/oxygen/font-bold.svgz \
	images/oxygen/font-emph.svgz \
	images/oxygen/font-noun.svgz \
	images/oxygen/font-sans.svgz \
	images/oxygen/font-smallcaps.svgz \
	images/oxygen/footnote-insert.svgz \
	images/oxygen/hidetab.svgz \
	images/oxygen/href-insert.svgz \
	images/oxygen/index-insert.svgz \
	images/oxygen/info-insert_vcs_revision.svgz \
	images/oxygen/label-insert.svgz \
	images/oxygen/layout-document.svgz \
	images/oxygen/layout-paragraph.svgz \
	images/oxygen/layout.svgz \
	images/oxygen/layout-toggle_Chunk.svgz \
	images/oxygen/layout-toggle_Description.svgz \
	images/oxygen/layout-toggle_Enumerate+rtl.svgz  \
	images/oxygen/layout-toggle_Enumerate.svgz  \
	images/oxygen/layout-toggle_Itemize.svgz \
	images/oxygen/layout-toggle_Labeling.svgz \
	images/oxygen/layout-toggle_List.svgz \
	images/oxygen/layout-toggle_LyX-Code.svgz \
	images/oxygen/layout-toggle_Section.svgz \
	images/oxygen/lyx-quit.svgz \
	images/oxygen/marginalnote-insert.svgz \
	images/oxygen/master-buffer-update.svgz \
	images/oxygen/master-buffer-update_pdf2.svgz \
	images/oxygen/master-buffer-view.svgz \
	images/oxygen/math-display.svgz \
	images/oxygen/math-macro-add-greedy-optional-param.svgz \
	images/oxygen/math-macro-add-optional-param.svgz \
	images/oxygen/math-macro-add-param.svgz \
	images/oxygen/math-macro-append-greedy-param.svgz \
	images/oxygen/math-macro-make-nonoptional.svgz \
	images/oxygen/math-macro-make-optional.svgz \
	images/oxygen/math-macro-remove-greedy-param.svgz \
	images/oxygen/math-macro-remove-optional-param.svgz \
	images/oxygen/math-macro-remove-param.svgz \
	images/oxygen/math-macro_newmacroname_newcommand.svgz \
	images/oxygen/math-matrix.svgz \
	images/oxygen/math-mode.svgz \
	images/oxygen/math-subscript.svgz \
	images/oxygen/math-superscript.svgz \
	images/oxygen/nomencl-insert.svgz \
	images/oxygen/note-insert.svgz \
	images/oxygen/note-next.svgz \
	images/oxygen/outline-down.svgz \
	images/oxygen/outline-in.svgz \
	images/oxygen/outline-in+rtl.svgz \
	images/oxygen/outline-out.svgz \
	images/oxygen/outline-out+rtl.svgz \
	images/oxygen/outline-up.svgz \
	images/oxygen/paste.svgz \
	images/oxygen/redo.svgz \
	images/oxygen/reload.svgz \
	images/oxygen/script-insert_subscript.svgz \
	images/oxygen/script-insert_superscript.svgz \
	images/oxygen/spelling-continuously.svgz \
	images/oxygen/sub.svgz \
	images/oxygen/super.svgz \
	images/oxygen/tab-group-close.svgz \
	images/oxygen/tabular-feature_align-decimal.svgz \
	images/oxygen/tabular-feature_append-column.svgz \
	images/oxygen/tabular-feature_append-row.svgz \
	images/oxygen/tabular-feature_delete-column.svgz \
	images/oxygen/tabular-feature_delete-row.svgz \
	images/oxygen/tabular-feature_m-align-center.svgz \
	images/oxygen/tabular-feature_m-align-left.svgz \
	images/oxygen/tabular-feature_m-align-right.svgz \
	images/oxygen/tabular-feature_m-valign-bottom.svgz \
	images/oxygen/tabular-feature_m-valign-middle.svgz \
	images/oxygen/tabular-feature_m-valign-top.svgz \
	images/oxygen/tabular-feature_move-column-left.svgz \
	images/oxygen/tabular-feature_move-column-right.svgz \
	images/oxygen/tabular-feature_move-row-down.svgz \
	images/oxygen/tabular-feature_move-row-up.svgz \
	images/oxygen/tabular-feature_multicolumn.svgz \
	images/oxygen/tabular-feature_multirow.svgz \
	images/oxygen/tabular-feature_reset-formal-default.svgz \
	images/oxygen/tabular-feature_set-all-lines.svgz \
	images/oxygen/tabular-feature_set-border-lines.svgz \
	images/oxygen/tabular-feature_set-inner-lines.svgz \
	images/oxygen/tabular-feature_set-longtabular.svgz \
	images/oxygen/tabular-feature_toggle-line-bottom.svgz \
	images/oxygen/tabular-feature_toggle-line-left.svgz \
	images/oxygen/tabular-feature_toggle-line-right.svgz \
	images/oxygen/tabular-feature_toggle-line-top.svgz \
	images/oxygen/tabular-feature_toggle-rotate-cell.svgz \
	images/oxygen/tabular-feature_toggle-rotate-tabular.svgz \
	images/oxygen/tabular-feature_unset-all-lines.svgz \
	images/oxygen/tabular-insert.svgz \
	images/oxygen/textstyle-apply.svgz \
	images/oxygen/thesaurus-entry.svgz \
	images/oxygen/toolbar-toggle_math.svgz \
	images/oxygen/toolbar-toggle_math_panels.svgz \
	images/oxygen/toolbar-toggle_review.svgz \
	images/oxygen/toolbar-toggle_table.svgz \
	images/oxygen/undo.svgz \
	images/oxygen/unknown.svgz \
	images/oxygen/up.svgz \
	images/oxygen/update-others.svgz \
	images/oxygen/url-insert.svgz \
	images/oxygen/vc-check-in.svgz \
	images/oxygen/vc-check-out.svgz \
	images/oxygen/vc-compare.svgz \
	images/oxygen/vc-compare_0.svgz \
	images/oxygen/vc-locking-toggle.svgz \
	images/oxygen/vc-register.svgz \
	images/oxygen/vc-repo-update.svgz \
	images/oxygen/vc-revert.svgz \
	images/oxygen/view-others.svgz \
	images/oxygen/view-split_horizontal.svgz \
	images/oxygen/view-split_vertical.svgz

dist_imagesoxygen_DATA = \
	images/oxygen/iconsize.png \
	$(dist_imagesoxygen_DATA1X)

imagesmathoxygendir = $(imagesdir)/math/oxygen
dist_imagesmathoxygen_DATA = \
	images/math/oxygen/cases.svgz \
	images/math/oxygen/delim.svgz \
	images/math/oxygen/font.svgz \
	images/math/oxygen/frac.svgz \
	images/math/oxygen/frac-square.svgz \
	images/math/oxygen/functions.svgz \
	images/math/oxygen/space.svgz \
	images/math/oxygen/style.svgz

imagesclassicdir = $(imagesdir)/classic
dist_imagesclassic_DATA = \
	images/classic/all-changes-accept.png  \
	images/classic/all-changes-reject.png  \
	images/classic/bookmark-goto.png  \
	images/classic/bookmark-goto_0.png  \
	images/classic/bookmark-save.png  \
	images/classic/box-insert.png  \
	images/classic/break-line.png  \
	images/classic/buffer-close.png  \
	images/classic/buffer-export.png  \
	images/classic/buffer-export_dvi.png  \
	images/classic/buffer-export_dvi3.png  \
	images/classic/buffer-export_latex.png  \
	images/classic/buffer-export_pdf.png  \
	images/classic/buffer-export_pdf2.png  \
	images/classic/buffer-export_pdf3.png  \
	images/classic/buffer-export_pdf4.png  \
	images/classic/buffer-export_pdf5.png  \
	images/classic/buffer-export_ps.png  \
	images/classic/buffer-export_text.png  \
	images/classic/buffer-new.png  \
	images/classic/buffer-reload.png  \
	images/classic/buffer-toggle-output-sync.png  \
	images/classic/buffer-update.png  \
	images/classic/buffer-update_dvi.png  \
	images/classic/buffer-update_dvi3.png  \
	images/classic/buffer-update_pdf.png  \
	images/classic/buffer-update_pdf2.png  \
	images/classic/buffer-update_pdf3.png  \
	images/classic/buffer-update_pdf4.png  \
	images/classic/buffer-update_pdf5.png  \
	images/classic/buffer-update_ps.png  \
	images/classic/buffer-view.png  \
	images/classic/buffer-view_dvi.png  \
	images/classic/buffer-view_dvi3.png  \
	images/classic/buffer-view_pdf.png  \
	images/classic/buffer-view_pdf2.png  \
	images/classic/buffer-view_pdf3.png  \
	images/classic/buffer-view_pdf4.png  \
	images/classic/buffer-view_pdf5.png  \
	images/classic/buffer-view_ps.png  \
	images/classic/buffer-write-as.png  \
	images/classic/buffer-write.png  \
	images/classic/build-program.png  \
	images/classic/change-accept.png  \
	images/classic/change-next.png  \
	images/classic/change-reject.png  \
	images/classic/changes-merge.png  \
	images/classic/changes-output.png  \
	images/classic/changes-track.png  \
	images/classic/tab-group-close.png  \
	images/classic/command-alternatives_outline-in_depth-increment.png \
	images/classic/command-alternatives_outline-out_depth-decrement.png \
	images/classic/copy.png  \
	images/classic/cut.png  \
	images/classic/depth-decrement.png  \
	images/classic/depth-increment.png  \
	images/classic/dialog-preferences.png  \
	images/classic/dialog-show_mathdelimiter.png  \
	images/classic/dialog-show_mathmatrix.png  \
	images/classic/dialog-show-new-inset_citation.png  \
	images/classic/dialog-show-new-inset_graphics.png  \
	images/classic/dialog-show-new-inset_include.png  \
	images/classic/dialog-show-new-inset_ref.png  \
	images/classic/dialog-show_character.png  \
	images/classic/dialog-show_findreplace.png  \
	images/classic/dialog-show_print.png  \
	images/classic/dialog-show_spellchecker.png  \
	images/classic/dialog-show_vclog.png  \
	images/classic/dialog-toggle_findreplaceadv.png  \
	images/classic/dialog-toggle_toc.png  \
	images/classic/down.png  \
	images/classic/ert-insert.png  \
	images/classic/file-open.png  \
	images/classic/float-insert_figure.png  \
	images/classic/float-insert_table.png  \
	images/classic/font-bold.png  \
	images/classic/font-emph.png  \
	images/classic/font-noun.png  \
	images/classic/font-sans.png  \
	images/classic/font-smallcaps.png  \
	images/classic/footnote-insert.png  \
	images/classic/href-insert.png  \
	images/classic/iconsize.png  \
	images/classic/index-insert.png  \
	images/classic/info-insert_vcs_revision.png  \
	images/classic/label-insert.png  \
	images/classic/layout-document.png  \
	images/classic/layout-paragraph.png  \
	images/classic/layout.png  \
	images/classic/layout-toggle_Chunk.png  \
	images/classic/layout-toggle_Description.png \
	images/classic/layout-toggle_Enumerate+rtl.png  \
	images/classic/layout-toggle_Enumerate.png  \
	images/classic/layout-toggle_Itemize.png \
	images/classic/layout-toggle_Labeling.png \
	images/classic/layout-toggle_List.png \
	images/classic/layout-toggle_LyX-Code.png \
	images/classic/layout-toggle_Section.png \
	images/classic/marginalnote-insert.png  \
	images/classic/master-buffer-update.png  \
	images/classic/master-buffer-view.png  \
	images/classic/math-display.png  \
	images/classic/math-macro-add-greedy-optional-param.png \
	images/classic/math-macro-add-optional-param.png \
	images/classic/math-macro-add-param.png \
	images/classic/math-macro-append-greedy-param.png \
	images/classic/math-macro-make-nonoptional.png \
	images/classic/math-macro-make-optional.png \
	images/classic/math-macro-remove-greedy-param.png \
	images/classic/math-macro-remove-optional-param.png \
	images/classic/math-macro-remove-param.png \
	images/classic/math-macro_newmacroname_newcommand.png  \
	images/classic/math-matrix.png  \
	images/classic/math-mode.png  \
	images/classic/math-subscript.png  \
	images/classic/math-superscript.png  \
	images/classic/nomencl-insert.png  \
	images/classic/note-insert.png  \
	images/classic/note-next.png  \
	images/classic/outline-down.png  \
	images/classic/outline-in.png  \
	images/classic/outline-in+rtl.png  \
	images/classic/outline-out.png  \
	images/classic/outline-out+rtl.png  \
	images/classic/outline-up.png  \
	images/classic/paste.png  \
	images/classic/redo.png  \
	images/classic/reload.png  \
	images/classic/script-insert_subscript.png  \
	images/classic/script-insert_superscript.png  \
	images/classic/view-split_horizontal.png  \
	images/classic/view-split_vertical.png  \
	images/classic/tabular-feature_align-decimal.png  \
	images/classic/tabular-feature_append-column.png  \
	images/classic/tabular-feature_append-row.png  \
	images/classic/tabular-feature_delete-column.png  \
	images/classic/tabular-feature_delete-row.png  \
	images/classic/tabular-feature_m-align-center.png  \
	images/classic/tabular-feature_m-align-left.png  \
	images/classic/tabular-feature_m-align-right.png  \
	images/classic/tabular-feature_m-valign-bottom.png  \
	images/classic/tabular-feature_m-valign-middle.png  \
	images/classic/tabular-feature_m-valign-top.png  \
	images/classic/tabular-feature_multicolumn.png  \
	images/classic/tabular-feature_multirow.png  \
	images/classic/tabular-feature_reset-formal-default.png \
	images/classic/tabular-feature_set-all-lines.png  \
	images/classic/tabular-feature_set-border-lines.png  \
	images/classic/tabular-feature_set-inner-lines.png  \
	images/classic/tabular-feature_set-longtabular.png  \
	images/classic/tabular-feature_set-rotate-cell.png  \
	images/classic/tabular-feature_set-rotate-tabular.png  \
	images/classic/tabular-feature_toggle-line-bottom.png  \
	images/classic/tabular-feature_toggle-line-left.png  \
	images/classic/tabular-feature_toggle-line-right.png  \
	images/classic/tabular-feature_toggle-line-top.png  \
	images/classic/tabular-feature_toggle-rotate-cell.png  \
	images/classic/tabular-feature_toggle-rotate-tabular.png  \
	images/classic/tabular-feature_unset-all-lines.png  \
	images/classic/tabular-insert.png  \
	images/classic/textstyle-apply.png  \
	images/classic/thesaurus-entry.png  \
	images/classic/toolbar-toggle_math.png  \
	images/classic/toolbar-toggle_math_panels.png  \
	images/classic/toolbar-toggle_review.png \
	images/classic/toolbar-toggle_table.png  \
	images/classic/undo.png  \
	images/classic/unknown.png  \
	images/classic/up.png  \
	images/classic/update-others.png  \
	images/classic/url-insert.png  \
	images/classic/vc-check-in.png  \
	images/classic/vc-check-out.png  \
	images/classic/vc-compare.png  \
	images/classic/vc-compare_0.png  \
	images/classic/vc-locking-toggle.png  \
	images/classic/vc-register.png  \
	images/classic/vc-repo-update.png  \
	images/classic/vc-revert.png  \
	images/classic/view-others.png

imagescommandsdir = $(imagesdir)/commands
dist_imagescommands_DATA =

kbddir = $(pkgdatadir)/kbd
dist_kbd_DATA = \
	kbd/american-2.kmap \
	kbd/american.kmap \
	kbd/arabic.kmap \
	kbd/brazil.kmap \
	kbd/brazil2.kmap \
	kbd/bulgarian.kmap \
	kbd/czech-prg.kmap \
	kbd/czech.kmap \
	kbd/espanol.kmap \
	kbd/european.kmap \
	kbd/farsi.kmap \
	kbd/francais.kmap \
	kbd/french.kmap \
	kbd/german-2.kmap \
	kbd/german-3.kmap \
	kbd/german.kmap \
	kbd/greek.kmap \
	kbd/hebrew.kmap \
	kbd/latvian.kmap \
	kbd/magyar-2.kmap \
	kbd/magyar-3.kmap \
	kbd/magyar.kmap \
	kbd/null.kmap \
	kbd/polish.kmap \
	kbd/polski.kmap \
	kbd/portuges.kmap \
	kbd/romanian.kmap \
	kbd/russian.kmap \
	kbd/serbian.kmap \
	kbd/serbocroatian.kmap \
	kbd/sf.kmap \
	kbd/sg.kmap \
	kbd/slovak.kmap \
	kbd/slovene.kmap \
	kbd/thai-kedmanee.kmap \
	kbd/transilvanian.kmap \
	kbd/turkish-f.kmap \
	kbd/turkish.kmap \
	kbd/ukrainian.kmap

layoutsdir = $(pkgdatadir)/layouts
dist_layouts_DATA =\
	layouts/a0poster.layout \
	layouts/aa.layout \
	layouts/aapaper.inc \
	layouts/aapaper.layout \
	layouts/aastex.layout \
	layouts/aastex6.layout \
	layouts/aastex62.layout \
	layouts/achemso.layout \
	layouts/acm-sigs.layout \
	layouts/acm-sigs-alt.layout \
	layouts/acm-sigs.inc \
	layouts/acmart.layout \
	layouts/acmsiggraph.layout \
	layouts/acmsiggraph-0-92.layout \
	layouts/AEA.layout \
	layouts/agutex.layout \
	layouts/agums.layout \
	layouts/aguplus.inc \
	layouts/algorithm2e.module \
	layouts/amsart.layout \
	layouts/amsbook.layout \
	layouts/amsdefs.inc \
	layouts/apa.layout \
	layouts/apa6.layout \
	layouts/apa7.layout \
	layouts/apax.inc \
	layouts/arab-article.layout \
	layouts/article.layout \
	layouts/article-beamer.layout \
	layouts/beamer.layout \
	layouts/beamerposter.layout \
	layouts/bicaption.module \
	layouts/book.layout \
	layouts/braille.module \
	layouts/broadway.layout \
	layouts/bxjsarticle.layout \
	layouts/bxjsbook.layout \
	layouts/bxjsreport.layout \
	layouts/bxjsslide.layout \
	layouts/chess.layout \
	layouts/chessboard.module \
	layouts/cl2emult.layout \
	layouts/copernicus.layout \
	layouts/ctex-article.layout \
	layouts/ctex-book.layout \
	layouts/ctex-report.layout \
	layouts/customHeadersFooters.module \
	layouts/db_stdcharstyles.inc \
	layouts/dinbrief.layout \
	layouts/docbook-book.layout \
	layouts/docbook-chapter.layout \
	layouts/docbook-section.layout \
	layouts/doublecol-new.layout \
	layouts/dtk.layout \
	layouts/ectaart.layout \
	layouts/egs.layout \
	layouts/elsart.layout \
	layouts/elsarticle.layout \
	layouts/endnotes.module \
	layouts/enotez.module \
	layouts/entcs.layout \
	layouts/enumitem.module \
	layouts/eqs-within-sections.module \
	layouts/europasscv.layout \
	layouts/europecv.layout \
	layouts/extarticle.layout \
	layouts/extbook.layout \
	layouts/extletter.layout \
	layouts/extreport.layout \
	layouts/figs-within-sections.module \
	layouts/fixltx2e.module \
	layouts/fixme.module \
	layouts/fix-cm.module \
	layouts/foils.layout \
	layouts/foottoend.module \
	layouts/foottoenotez.module \
	layouts/frletter.layout \
	layouts/g-brief.layout \
	layouts/g-brief2.layout \
	layouts/graphicboxes.module \
	layouts/hanging.module \
	layouts/heb-article.layout \
	layouts/heb-letter.layout \
	layouts/hollywood.layout \
	layouts/hpstatement.module \
	layouts/IEEEtran.layout \
	layouts/IEEEtran-CompSoc.layout \
	layouts/IEEEtran-TransMag.layout \
	layouts/InStar.module \
	layouts/ijmpc.layout \
	layouts/ijmpd.layout \
	layouts/initials.module \
	layouts/iopart.layout \
	layouts/isprs.layout \
	layouts/iucr.layout \
	layouts/jarticle.layout \
	layouts/jasatex.layout \
	layouts/jbook.layout \
	layouts/jgrga.layout \
	layouts/jreport.layout \
	layouts/jsarticle.layout \
	layouts/jsbook.layout \
	layouts/jss.layout \
	layouts/kluwer.layout \
	layouts/knitr.module \
	layouts/landscape.module \
	layouts/latex8.layout \
	layouts/letter.layout \
	layouts/lettre.layout \
	layouts/lilypond.module \
	layouts/linguistics.module \
	layouts/litinsets.inc \
	layouts/llncs.layout \
	layouts/logicalmkup.module \
	layouts/ltugboat.layout \
	layouts/lyxmacros.inc \
	layouts/maa-monthly.layout \
	layouts/memoir.layout \
	layouts/minimalistic.module \
	layouts/moderncv.layout \
	layouts/multicol.module \
	layouts/mwart.layout \
	layouts/mwbk.layout \
	layouts/mwrep.layout \
	layouts/natbibapa.module \
	layouts/noweb.module \
	layouts/numarticle.inc \
	layouts/numreport.inc \
	layouts/numrevtex.inc \
	layouts/paper.layout \
	layouts/paralist.module \
	layouts/pdfcomment.module \
	layouts/pdfform.module \
	layouts/powerdot.layout \
	layouts/RJournal.layout \
	layouts/recipebook.layout \
	layouts/report.layout \
	layouts/revtex.layout \
	layouts/revtex4.layout \
	layouts/revtex4-1.layout \
	layouts/revtex4-2.layout \
	layouts/revtex4-x.inc \
	layouts/rsphrase.module \
	layouts/ruby.module \
	layouts/sciposter.layout \
	layouts/scrartcl.layout \
	layouts/scrarticle-beamer.layout \
	layouts/scrbook.layout \
	layouts/scrclass.inc \
	layouts/scrlettr.layout \
	layouts/scrlttr2.layout \
	layouts/scrreprt.layout \
	layouts/sectionbox.module \
	layouts/seminar.layout \
	layouts/shapepar.module \
	layouts/siamltex.layout \
	layouts/sigplanconf.layout \
	layouts/simplecv.layout \
	layouts/singlecol.layout \
	layouts/singlecol-new.layout \
	layouts/slides.layout \
	layouts/soul.module \
	layouts/spie.layout \
	layouts/stdciteformats.inc \
	layouts/stdclass.inc \
	layouts/stdcharstyles.inc \
	layouts/stdcounters.inc \
	layouts/stdcustom.inc \
	layouts/stdfloatnames.inc \
	layouts/stdfloats.inc \
	layouts/stdinsets.inc \
	layouts/stdlayouts.inc \
	layouts/stdletter.inc \
	layouts/stdlists.inc \
	layouts/stdlyxlist.inc \
	layouts/stdrefprefix.inc \
	layouts/stdsections.inc \
	layouts/stdstarsections.inc \
	layouts/stdstruct.inc \
	layouts/stdtitle.inc \
	layouts/subequations.module \
	layouts/svcommon.inc \
	layouts/svglobal.layout \
	layouts/svglobal3.layout \
	layouts/svjog.layout \
	layouts/svmono.layout \
	layouts/svmult.layout \
	layouts/svprobth.layout \
	layouts/sweave.module \
	layouts/tabs-within-sections.module \
	layouts/tarticle.layout \
	layouts/tbook.layout \
	layouts/tcolorbox.module \
	layouts/theorems-ams.inc \
	layouts/theorems-ams.module \
	layouts/theorems-ams-bytype.inc \
	layouts/theorems-ams-bytype.module \
	layouts/theorems-ams-chap-bytype.inc \
	layouts/theorems-ams-chap-bytype.module \
	layouts/theorems-ams-extended.module \
	layouts/theorems-ams-extended-bytype.module \
	layouts/theorems-ams-extended-chap-bytype.module \
	layouts/theorems-case.inc \
	layouts/theorems-chap.module \
	layouts/theorems.inc \
	layouts/theorems-bytype.inc \
	layouts/theorems-bytype.module \
	layouts/theorems-chap-bytype.module \
	layouts/theorems-named.module \
	layouts/theorems-order.inc \
	layouts/theorems-proof.inc \
	layouts/theorems-proof-std.inc \
	layouts/theorems-refprefix.inc \
	layouts/theorems-sec.module \
	layouts/theorems-sec-bytype.module \
	layouts/theorems-starred-equivalents.inc \
	layouts/theorems-starred.inc \
	layouts/theorems-starred.module \
	layouts/theorems-std.module \
	layouts/theorems-without-preamble.inc \
	layouts/todonotes.module \
	layouts/treport.layout \
	layouts/tufte-book.layout \
	layouts/tufte-handout.layout \
	layouts/varwidth.module

citeenginesdir = $(pkgdatadir)/citeengines
dist_citeengines_DATA = \
	citeengines/basic.citeengine \
        citeengines/biblatex.citeengine \
        citeengines/biblatex-natbib.citeengine \
        citeengines/jurabib.citeengine \
        citeengines/natbib.citeengine

scriptsdir = $(pkgdatadir)/scripts
dist_scripts_DATA = \
	scripts/bash_completion \
	scripts/lyxknitr.R \
	scripts/lyxstangle.R \
	scripts/lyxsweave.R

# We use DATA now instead of PYTHON because automake 1.11.2 complains.
# Note that we "chmod 755" manually these files in install-data-hook.
dist_scripts_DATA += \
	scripts/clean_dvi.py \
	scripts/convert_pdf.py \
	scripts/convertDefault.py \
	scripts/csv2lyx.py \
	scripts/ext_copy.py \
	scripts/fen2ascii.py \
	scripts/fig2pdftex.py \
	scripts/fig2pstex.py \
	scripts/svg2pdftex.py \
	scripts/svg2pstex.py \
	scripts/fig_copy.py \
	scripts/gnuplot2pdf.py \
	scripts/html2latexwrapper.py \
	scripts/include_bib.py \
	scripts/layout2layout.py \
	scripts/legacy_lyxpreview2ppm.py \
	scripts/listerrors \
	scripts/lyxpak.py \
	scripts/lyxpaperview.py \
	scripts/lyxpreview2bitmap.py \
	scripts/lyxpreview_tools.py \
	scripts/lyx_batch.pl.in \
	scripts/prefs2prefs.py \
	scripts/prefs2prefs_lfuns.py \
	scripts/prefs2prefs_prefs.py \
	scripts/prefTest.pl.in \
	scripts/spreadsheet_to_docbook.py \
	scripts/tex_copy.py \
	scripts/TeXFiles.py

templatesdir = $(pkgdatadir)/templates
dist_templates_DATA = \
	templates/EPS_%28cropped%29.lyx \
	templates/PDF_%28cropped%29.lyx \
	templates/README.new_templates

articletemplatesdir = $(pkgdatadir)/templates/Articles
dist_articletemplates_DATA = \
	templates/Articles/American_Astronomical_Society_%28AASTeX_v._6.2%29.lyx \
	templates/Articles/American_Economic_Association_%28AEA%29.lyx \
	templates/Articles/American_Psychological_Association_%28APA%29.lyx \
	templates/Articles/American_Psychological_Association_%28APA%29,_v._7.lyx \
	templates/Articles/Association_for_Computing_Machinery_%28ACM%29.lyx \
	templates/Articles/Astronomy_%26_Astrophysics.lyx \
	templates/Articles/Chinese_Article_%28CTeX%29.lyx \
	templates/Articles/Copernicus_Publications_Manuscript_Preparation.lyx \
	templates/Articles/Econometrica.lyx \
	templates/Articles/Elsevier.lyx \
	templates/Articles/Figure1.pdf \
	templates/Articles/Hebrew_Article_%28KOMA-Script%29.lyx \
	templates/Articles/IEEE_Transactions_Computer_Society.lyx \
	templates/Articles/IEEE_Transactions_Conference.lyx \
	templates/Articles/IEEE_Transactions_Journal.lyx \
	templates/Articles/IEEE_Transactions_on_Magnetics.lyx \
	templates/Articles/Institute_of_Physics_%28IOP%29.lyx \
	templates/Articles/Int._Journal_of_Modern_Physics_C.lyx \
	templates/Articles/Int._Journal_of_Modern_Physics_D.lyx \
	templates/Articles/International_Union_of_Crystallography_%28IUCr%29.lyx \
	templates/Articles/Journal_of_Statistical_Software_%28JSS%29_version_2.lyx \
	templates/Articles/Journal_of_the_Acoustical_Society_of_America_%28JASA%29.lyx \
	templates/Articles/Kluwer.lyx \
	templates/Articles/Mathematical_Monthly.lyx \
	templates/Articles/R_Journal.lyx \
	templates/Articles/REVTeX_%28V._4%29.lyx \
	templates/Articles/REVTeX_%28V._4.1%29.lyx \
	templates/Articles/Springers_Global_Journal_Template_%28V._3%29.lyx

depresentationtemplatesdir = $(pkgdatadir)/templates/de/Presentations
dist_depresentationtemplates_DATA = \
	templates/de/Presentations/Beamer.lyx

espresentationtemplatesdir = $(pkgdatadir)/templates/es/Presentations
dist_espresentationtemplates_DATA = \
	templates/es/Presentations/Beamer.lyx

frpresentationtemplatesdir = $(pkgdatadir)/templates/fr/Presentations
dist_frpresentationtemplates_DATA = \
	templates/fr/Presentations/Beamer.lyx

japresentationtemplatesdir = $(pkgdatadir)/templates/ja/Presentations
dist_japresentationtemplates_DATA = \
	templates/ja/Presentations/Beamer.lyx

lettertemplatesdir = $(pkgdatadir)/templates/Letters
dist_lettertemplates_DATA = \
	templates/Letters/DIN-Brief.lyx \
	templates/Letters/French_Letter_%28frletter%29.lyx \
	templates/Letters/French_Letter_%28lettre%29.lyx \
	templates/Letters/G-Brief_%28V._2%29.lyx \
	templates/Letters/KOMA-Script_Letter_%28V._2%29.lyx \
	templates/Letters/Letter_%28Standard_Class%29.lyx

obsoletetemplatesdir = $(pkgdatadir)/templates/Obsolete
dist_obsoletetemplates_DATA = \
	templates/Obsolete/ACM_SIGGGRAPH_0.92_%28obsolete%29.lyx \
	templates/Obsolete/ACM_SIGPLAN_%28Obsolete%29.lyx \
	templates/Obsolete/ACM_SIG_Proceedings_%28SP,_Obsolete%29.lyx \
	templates/Obsolete/American_Astronomical_Society_%28AASTeX_v._6%29.lyx \
	templates/Obsolete/American_Geophysical_Union_%28AGUTeX%29.lyx \
	templates/Obsolete/American_Psychological_Association_%28APA%29,_v._6.lyx \
	templates/Obsolete/Latex8_Article_%28Obsolete%29.lyx

obsoletegbrieftemplatesdir = $(pkgdatadir)/templates/Obsolete/G-Brief_%28V._1,_Obsolete%29
dist_obsoletegbrieftemplates_DATA = \
	templates/Obsolete/G-Brief_%28V._1,_Obsolete%29/German.lyx \
	templates/Obsolete/G-Brief_%28V._1,_Obsolete%29/English.lyx

postertemplatesdir = $(pkgdatadir)/templates/Posters
dist_postertemplates_DATA = \
	templates/Posters/Beamerposter.lyx \
	templates/Posters/SciPoster.lyx

posteraztemplatesdir = $(pkgdatadir)/templates/Posters/A0_Poster
dist_posteraztemplates_DATA = \
	templates/Posters/A0_Poster/Colored.lyx \
	templates/Posters/A0_Poster/Simple.lyx

presentationtemplatesdir = $(pkgdatadir)/templates/Presentations
dist_presentationtemplates_DATA = \
	templates/Presentations/Beamer.lyx \
	templates/Presentations/Slides.lyx

scriptstemplatesdir = $(pkgdatadir)/templates/Scripts
dist_scriptstemplates_DATA = \
	templates/Scripts/Hollywood.lyx

texdir = $(pkgdatadir)/tex
dist_tex_DATA = \
	tex/broadway.cls \
	tex/hollywood.cls \
	tex/lyxchess.sty \
	tex/lyxskak.sty \
	tex/revtex.cls

# subscript.sty is not in dist_tex_DATA since we don't want to install it
# (it is part of TeXLive). We include it in our source package only so that
# packagers may decide to install it (e.g. for MikTeX).
dist_noinst_DATA += tex/subscript.sty

uidir = $(pkgdatadir)/ui
dist_ui_DATA = \
	ui/default.ui \
	ui/stdcontext.inc \
	ui/stdmenus.inc \
	ui/stdtoolbars.inc

tabletemplatesdir = $(pkgdatadir)/tabletemplates
dist_tabletemplates_DATA = \
	tabletemplates/Formal_with_Footline.lyx \
	tabletemplates/Formal_with_Footline_1x1.lyx \
	tabletemplates/Formal_with_Footline_1x2.lyx \
	tabletemplates/Formal_without_Footline.lyx \
	tabletemplates/Formal_without_Footline_1x1.lyx \
	tabletemplates/Formal_without_Footline_1x2.lyx \
	tabletemplates/Grid_with_Head.lyx \
	tabletemplates/Grid_with_Head_1x1.lyx \
	tabletemplates/Grid_with_Head_1x2.lyx \
	tabletemplates/No_Borders.lyx \
	tabletemplates/No_Borders_1x1.lyx \
	tabletemplates/No_Borders_1x2.lyx \
	tabletemplates/Simple_Grid.lyx \
	tabletemplates/Simple_Grid_1x1.lyx \
	tabletemplates/Simple_Grid_1x2.lyx

xtemplatesdir = $(pkgdatadir)/xtemplates
dist_xtemplates_DATA = \
	xtemplates/chess.xtemplate \
	xtemplates/dia.xtemplate \
	xtemplates/gnumeric.xtemplate \
	xtemplates/inkscape.xtemplate \
	xtemplates/lilypond.xtemplate \
	xtemplates/pdfpages.xtemplate \
	xtemplates/raster_image.xtemplate \
	xtemplates/vector_graphics.xtemplate \
	xtemplates/xfig.xtemplate

if INSTALL_POSIX
# install application icons. The 'hicolor' theme is the default theme
# where applications themselves should install their icons.
lyx@program_suffix@.svg: images/lyx.svg
	$(AM_V_GEN)$(INSTALL_DATA) $< $@
scalableappiconsdir = $(datadir)/icons/hicolor/scalable/apps
scalableappicons_DATA = lyx@program_suffix@.svg

lyx@program_suffix@.png: images/lyx.png
	$(AM_V_GEN)$(INSTALL_DATA) $< $@
fixedappiconsdir = $(datadir)/icons/hicolor/48x48/apps
fixedappicons_DATA = lyx@program_suffix@.png

# if configure is asked to generate the name with suffix directly, then
# config.status refuses to rebuild this file (with a variable name), and
# a build error ensues when lyx.desktop.in is changed.
lyx@program_suffix@.desktop: lyx.desktop-temp
	$(AM_V_GEN)$(INSTALL_DATA) $< $@
desktopdir = $(datadir)/applications
desktop_DATA = lyx@program_suffix@.desktop

CLEANFILES = lyx@program_suffix@.desktop lyx.desktop-temp \
             lyx@program_suffix@.svg lyx@program_suffix@.png
endif
dist_noinst_DATA += lyx.desktop.in images/lyx.png

exampleandtemplate_files = \
			   $(dist_articleexamples_DATA) \
			   $(dist_articlechessexamples_DATA) \
			   $(dist_articletemplates_DATA) \
			   $(dist_bookexamples_DATA) \
			   $(dist_booksvmonotemplates_DATA) \
			   $(dist_booksvmulttemplates_DATA) \
			   $(dist_caexamples_DATA) \
			   $(dist_cainsetexamples_DATA) \
			   $(dist_csexamples_DATA) \
			   $(dist_cvexamples_DATA) \
			   $(dist_daexamples_DATA) \
			   $(dist_deexamples_DATA) \
			   $(dist_decvexamples_DATA) \
			   $(dist_dehandoutexamples_DATA) \
			   $(dist_deinsetexamples_DATA) \
			   $(dist_deletterexamples_DATA) \
			   $(dist_depresentationexamples_DATA) \
			   $(dist_demoduleexamples_DATA) \
			   $(dist_depresentationtemplates_DATA) \
			   $(dist_elexamples_DATA) \
			   $(dist_esexamples_DATA) \
			   $(dist_esbookexamples_DATA) \
			   $(dist_escvexamples_DATA) \
			   $(dist_eshandoutexamples_DATA) \
			   $(dist_esinsetexamples_DATA) \
			   $(dist_esmoduleexamples_DATA) \
			   $(dist_espresentationtemplates_DATA) \
			   $(dist_euexamples_DATA) \
			   $(dist_externalexamples_DATA) \
			   $(dist_faexamples_DATA) \
			   $(dist_frexamples_DATA) \
			   $(dist_frcvexamples_DATA) \
			   $(dist_frinsetexamples_DATA) \
			   $(dist_frmoduleexamples_DATA) \
			   $(dist_frpresentationexamples_DATA) \
			   $(dist_frpresentationtemplates_DATA) \
			   $(dist_glexamples_DATA) \
			   $(dist_handoutexamples_DATA) \
			   $(dist_heexamples_DATA) \
			   $(dist_huexamples_DATA) \
			   $(dist_idexamples_DATA) \
			   $(dist_insetexamples_DATA) \
			   $(dist_itexamples_DATA) \
			   $(dist_itinsetexamples_DATA) \
			   $(dist_jaexamples_DATA) \
			   $(dist_jainsetexamples_DATA) \
			   $(dist_jamoduleexamples_DATA) \
			   $(dist_japresentationexamples_DATA) \
			   $(dist_japresentationtemplates_DATA) \
			   $(dist_koexamples_DATA) \
			   $(dist_letterexamples_DATA) \
			   $(dist_lettertemplates_DATA) \
			   $(dist_moduleexamples_DATA) \
			   $(dist_nbexamples_DATA) \
			   $(dist_nlexamples_DATA) \
			   $(dist_nlinsetexamples_DATA) \
			   $(dist_obsoletetemplates_DATA) \
			   $(dist_obsoletegbrieftemplates_DATA) \
			   $(dist_phdthesistemplates_DATA) \
			   $(dist_plexamples_DATA) \
			   $(dist_posterexamples_DATA) \
			   $(dist_postertemplates_DATA) \
			   $(dist_posteraztemplates_DATA) \
			   $(dist_presentationexamples_DATA) \
			   $(dist_presentationtemplates_DATA) \
			   $(dist_ptexamples_DATA) \
			   $(dist_roexamples_DATA) \
			   $(dist_ruexamples_DATA) \
			   $(dist_ruinsetexamples_DATA) \
			   $(dist_rupresentationexamples_DATA) \
			   $(dist_scriptexamples_DATA) \
			   $(dist_scriptstemplates_DATA) \
			   $(dist_slexamples_DATA) \
			   $(dist_skexamples_DATA) \
			   $(dist_srexamples_DATA) \
			   $(dist_srmoduleexamples_DATA) \
			   $(dist_svexamples_DATA) \
			   $(dist_tabletemplates_DATA) \
			   $(dist_thesistemplates_DATA) \
			   $(dist_ukexamples_DATA) \
			   $(dist_examples_DATA) \
			   $(dist_templates_DATA)

install-data-hook:
	$(CHMOD) 755 $(DESTDIR)$(pkgdatadir)/configure.py
	for i in $(DESTDIR)$(scriptsdir)/*.py $(DESTDIR)$(scriptsdir)/listerrors ; do \
		$(CHMOD) 755 $$i; \
	done
	for i in $(exampleandtemplate_files); do \
		if $(EGREP) -q "^\\\\origin " "$(DESTDIR)$(pkgdatadir)/$$i"; then \
			origin=`dirname "/systemlyxdir/$$i"`/; \
			sed -e "s|^\\\\origin .*|\\\\origin $$origin|" \
				"$(DESTDIR)$(pkgdatadir)/$$i" > "$(DESTDIR)$(pkgdatadir)/$$i.tmp"; \
			mv -f "$(DESTDIR)$(pkgdatadir)/$$i.tmp" "$(DESTDIR)$(pkgdatadir)/$$i"; \
			chmod 644 "$(DESTDIR)$(pkgdatadir)/$$i"; \
		fi; \
	done

alltests: check alltests-recursive

alltests-recursive:
	cd lyx2lyx; $(MAKE) alltests-recursive

