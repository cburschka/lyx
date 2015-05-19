include $(top_srcdir)/config/common.am

SUBDIRS = doc lyx2lyx

CHMOD = chmod

EXTRA_DIST = examples/CMakeLists.txt scripts/CMakeLists.txt

dist_pkgdata_DATA = CREDITS autocorrect chkconfig.ltx external_templates \
	encodings layouttranslations languages latexfonts RELEASE-NOTES \
	symbols syntax.default unicodesymbols

# We use DATA now instead of PYTHON because automake 1.11.2 complains.
# Note that we "chmod 755" manually this file in install-data-hook.
dist_pkgdata_DATA += configure.py 

dist_noinst_DATA = \
	appdata.xml \
	fonts/stmary10.sfd \
	fonts/test/stmary10.lyx \
	images/README \
	images/font-smallcaps.png \
	images/math/ams_arrows.png \
	images/math/ams_misc.png \
	images/math/ams_nrel.png \
	images/math/ams_ops.png \
	images/math/ams_rel.png \
	images/math/arrows.png \
	images/math/bop.png \
	images/math/brel.png \
	images/math/deco.png \
	images/math/deco.png \
	images/math/delim0.png \
	images/math/delim1.png \
	images/math/delim.png \
	images/math/dots.png \
	images/math/font.png \
	images/math/greek.png \
	images/math/misc.png \
	images/math/varsz.png

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

caexamplesdir = $(pkgdatadir)/examples/ca
dist_caexamples_DATA = \
	examples/ca/ItemizeBullets.lyx \
	examples/ca/mathed.lyx \
	examples/ca/splash.lyx

csexamplesdir = $(pkgdatadir)/examples/cs
dist_csexamples_DATA = \
	examples/cs/splash.lyx \
	examples/cs/priklad_syrovy.lyx \
	examples/cs/priklad_zLyXovany.lyx

daexamplesdir = $(pkgdatadir)/examples/da
dist_daexamples_DATA = \
	examples/da/splash.lyx

deexamplesdir = $(pkgdatadir)/examples/de
dist_deexamples_DATA = \
	examples/de/beamer.lyx \
	examples/de/beamer-article.lyx \
	examples/de/beispiel_gelyxt.lyx \
	examples/de/beispiel_roh.lyx \
	examples/de/Braille.lyx \
	examples/de/Dezimal.lyx \
	examples/de/europassCV.lyx \
	examples/de/FeynmanDiagrams.lyx \
	examples/de/ItemizeBullets.lyx \
	examples/de/Lebenslauf.lyx \
	examples/de/linguistics.lyx \
	examples/de/MultilingualCaptions.lyx \
	examples/de/serienbrief1.lyx \
	examples/de/serienbrief2.lyx \
	examples/de/serienbrief3.lyx \
	examples/de/splash.lyx \
	examples/de/tufte-handout.lyx

elexamplesdir = $(pkgdatadir)/examples/el
dist_elexamples_DATA = \
	examples/el/splash.lyx

esexamplesdir = $(pkgdatadir)/examples/es
dist_esexamples_DATA = \
	examples/es/Braille.lyx \
	examples/es/ejemplo_con_lyx.lyx \
	examples/es/ejemplo_sin_lyx.lyx \
	examples/es/europeCV.lyx \
	examples/es/ItemizeBullets.lyx \
	examples/es/linguistics.lyx \
	examples/es/modernCV.lyx \
	examples/es/splash.lyx \
	examples/es/tufte-book.lyx \
	examples/es/tufte-handout.lyx

euexamplesdir = $(pkgdatadir)/examples/eu
dist_euexamples_DATA = \
	examples/eu/adibide_gordina.lyx \
	examples/eu/adibide_lyx-atua.lyx \
	examples/eu/splash.lyx

faexamplesdir = $(pkgdatadir)/examples/fa
dist_faexamples_DATA = \
	examples/fa/splash.lyx

frexamplesdir = $(pkgdatadir)/examples/fr
dist_frexamples_DATA = \
	examples/fr/AlignementDecimal.lyx \
	examples/fr/Braille.lyx \
	examples/fr/exemple_brut.lyx \
	examples/fr/exemple_lyxifie.lyx \
	examples/fr/exemple-powerdot.lyx \
	examples/fr/FeynmanDiagrams.lyx \
	examples/fr/Foils.lyx \
	examples/fr/linguistics.lyx \
	examples/fr/ListesPuces.lyx \
	examples/fr/MultilingualCaptions.lyx \
	examples/fr/seminar.lyx \
	examples/fr/simplecv.lyx \
	examples/fr/splash.lyx \
	examples/fr/xyfigure.png \
	examples/fr/xypic.lyx

glexamplesdir = $(pkgdatadir)/examples/gl
dist_glexamples_DATA = \
	examples/gl/exemplo_bruto.lyx \
	examples/gl/exemplo_lyxificado.lyx \
	examples/gl/splash.lyx

heexamplesdir = $(pkgdatadir)/examples/he
dist_heexamples_DATA = \
	examples/he/example_lyxified.lyx \
	examples/he/example_raw.lyx \
	examples/he/splash.lyx

huexamplesdir = $(pkgdatadir)/examples/hu
dist_huexamples_DATA = \
	examples/hu/example_lyxified.lyx \
	examples/hu/example_raw.lyx \
	examples/hu/splash.lyx

idexamplesdir = $(pkgdatadir)/examples/id
dist_idexamples_DATA = \
	examples/id/splash.lyx

itexamplesdir = $(pkgdatadir)/examples/it
dist_itexamples_DATA = \
	examples/it/ItemizeBullets.lyx \
	examples/it/splash.lyx

jaexamplesdir = $(pkgdatadir)/examples/ja
dist_jaexamples_DATA = \
	examples/ja/Braille.lyx \
	examples/ja/FeynmanDiagrams.lyx \
	examples/ja/MultilingualCaptions.lyx \
	examples/ja/R-S-statements.lyx \
	examples/ja/beamer.lyx \
	examples/ja/knitr.lyx \
	examples/ja/lilypond.lyx \
	examples/ja/linguistics.lyx \
	examples/ja/splash.lyx \
	examples/ja/sweave.lyx \
	examples/ja/xyfigure.png \
	examples/ja/xypic.lyx

koexamplesdir = $(pkgdatadir)/examples/ko
dist_koexamples_DATA = \
	examples/ko/splash.lyx

nlexamplesdir = $(pkgdatadir)/examples/nl
dist_nlexamples_DATA = \
	examples/nl/opsommingstekens.lyx \
	examples/nl/splash.lyx \
	examples/nl/voorbeeld_ruw.lyx \
	examples/nl/voorbeeld_verlyxt.lyx

plexamplesdir = $(pkgdatadir)/examples/pl
dist_plexamples_DATA = \
	examples/pl/splash.lyx

ptexamplesdir = $(pkgdatadir)/examples/pt
dist_ptexamples_DATA = \
	examples/pt/splash.lyx

roexamplesdir = $(pkgdatadir)/examples/ro
dist_roexamples_DATA = \
	examples/ro/splash.lyx

ruexamplesdir = $(pkgdatadir)/examples/ru
dist_ruexamples_DATA = \
	examples/ru/splash.lyx

slexamplesdir = $(pkgdatadir)/examples/sl
dist_slexamples_DATA = \
	examples/sl/primer_lyxan.lyx \
	examples/sl/primer_surov.lyx \
	examples/sl/splash.lyx

skexamplesdir = $(pkgdatadir)/examples/sk
dist_skexamples_DATA = \
	examples/sk/splash.lyx

srexamplesdir = $(pkgdatadir)/examples/sr
dist_srexamples_DATA = \
	examples/sr/Braille.lyx \
	examples/sr/splash.lyx

svexamplesdir = $(pkgdatadir)/examples/sv
dist_svexamples_DATA = \
	examples/sv/splash.lyx

springerexamplesdir = $(pkgdatadir)/examples/springer
dist_springerexamples_DATA = \
	examples/springer/svmono_acknow.lyx \
	examples/springer/svmono_acronym.lyx \
	examples/springer/svmono_appendix.lyx \
	examples/springer/svmono_book-master.lyx \
	examples/springer/svmono_chapter.lyx \
	examples/springer/svmono_dedication.lyx \
	examples/springer/svmono_foreword.lyx \
	examples/springer/svmono_glossary.lyx \
	examples/springer/svmono_part.lyx \
	examples/springer/svmono_preface.lyx \
	examples/springer/svmono_referenc.lyx \
	examples/springer/svmono_solutions.lyx \
	examples/springer/svmult_acknow.lyx \
	examples/springer/svmult_acronym.lyx \
	examples/springer/svmult_appendix.lyx \
	examples/springer/svmult_author.lyx \
	examples/springer/svmult_cblist.lyx \
	examples/springer/svmult_dedication.lyx \
	examples/springer/svmult_editor-master.lyx \
	examples/springer/svmult_foreword.lyx \
	examples/springer/svmult_glossary.lyx \
	examples/springer/svmult_part.lyx \
	examples/springer/svmult_preface.lyx \
	examples/springer/svmult_referenc.lyx
	
thesisexamplesdir = $(pkgdatadir)/examples/thesis
dist_thesisexamples_DATA = \
	examples/thesis/Acknowledgments.lyx \
	examples/thesis/alpha.bst \
	examples/thesis/Appendix.lyx \
	examples/thesis/chapter-1.lyx \
	examples/thesis/chapter-2.lyx \
	examples/thesis/Summary.lyx \
	examples/thesis/thesis.lyx \
	examples/thesis/thesisExample.bib

ukexamplesdir = $(pkgdatadir)/examples/uk
dist_ukexamples_DATA = \
	examples/uk/splash.lyx

examplesdir = $(pkgdatadir)/examples
dist_examples_DATA = \
	examples/Braille.lyx \
	examples/CV-image.eps \
	examples/CV-image.png \
	examples/Foils.lyx \
	examples/ItemizeBullets.lyx \
	examples/Literate.lyx \
	examples/aa_sample.lyx \
	examples/aas_sample.lyx \
	examples/achemso.lyx \
	examples/addressExample.adr \
	examples/amsart-test.lyx \
	examples/amsbook-test.lyx \
	examples/beamer.lyx \
	examples/beamer-article.lyx \
	examples/beamer-g4.jpg \
	examples/beamer-g4-mask.jpg \
	examples/beamer-icsi-logo.pdf \
	examples/beamer-knight1-mask.png \
	examples/beamer-knight1.png \
	examples/beamer-knight2-mask.png \
	examples/beamer-knight2.png \
	examples/beamer-knight3-mask.png \
	examples/beamer-knight3.png \
	examples/beamer-knight4-mask.png \
	examples/beamer-knight4.png \
	examples/beamerlyxexample1.lyx \
	examples/beamerposter.lyx \
	examples/biblioExample.bib \
	examples/chess-article.lyx \
	examples/chessgame.lyx \
	examples/docbook_article.lyx \
	examples/europeCV.lyx \
	examples/europassCV.lyx \
	examples/example_lyxified.lyx \
	examples/example_raw.lyx \
	examples/FeynmanDiagrams.lyx \
	examples/iecc05.fen \
	examples/iecc07.fen \
	examples/iecc12.fen \
	examples/instant_preview.lyx \
	examples/knitr.lyx \
	examples/landslide.lyx \
	examples/lilypond.lyx \
	examples/linguistics.lyx \
	examples/listerrors.lyx \
	examples/localization_test.lyx \
	examples/longsheet.gnumeric \
	examples/modernCV.lyx \
	examples/MultilingualCaptions.lyx \
	examples/noweb2lyx.lyx \
	examples/PDF-comment.lyx \
	examples/PDF-form.lyx \
	examples/powerdot-example.lyx \
	examples/recipebook.lyx \
	examples/R-S-statements.lyx \
	examples/script_form.lyx \
	examples/seminar.lyx \
	examples/serial_letter1.lyx \
	examples/serial_letter2.lyx \
	examples/serial_letter3.lyx \
	examples/sheet1.gnumeric \
	examples/sheet2.ods \
	examples/sheet3.xls \
	examples/simplecv.lyx \
	examples/splash.lyx \
	examples/spreadsheet.lyx \
	examples/sweave.lyx \
	examples/tufte-book.lyx \
	examples/tufte-handout.lyx \
	examples/xyfigure.lyx \
	examples/xyfigure.png \
	examples/xypic.lyx

fontsdir = $(pkgdatadir)/fonts
dist_fonts_DATA = \
	fonts/BaKoMaFontLicense.txt \
	fonts/cmex10.ttf \
	fonts/cmmi10.ttf \
	fonts/cmr10.ttf \
	fonts/cmsy10.ttf \
	fonts/esint10.ttf \
	fonts/eufm10.ttf \
	fonts/msam10.ttf \
	fonts/msbm10.ttf \
	fonts/ReadmeBaKoMa4LyX.txt \
	fonts/rsfs10.ttf \
	fonts/stmary10.ttf \
	fonts/wasy10.ttf

imagesdir = $(pkgdatadir)/images
dist_images_DATA1X = \
	images/all-changes-accept.png \
	images/all-changes-reject.png \
	images/banner.png \
	images/bookmark-goto.png \
	images/bookmark-goto_0.png \
	images/bookmark-save.png \
	images/box-insert.png \
	images/break-line.png \
	images/buffer-close.png \
	images/buffer-export.png \
	images/buffer-export_dvi.png \
	images/buffer-export_dvi3.png \
	images/buffer-export_latex.png \
	images/buffer-export_pdf.png \
	images/buffer-export_pdf2.png \
	images/buffer-export_pdf3.png \
	images/buffer-export_pdf4.png \
	images/buffer-export_pdf5.png \
	images/buffer-export_ps.png \
	images/buffer-export_text.png \
	images/buffer-new.png \
	images/buffer-reload.png \
	images/buffer-toggle-output-sync.png \
	images/buffer-update.png \
	images/buffer-update_dvi.png \
	images/buffer-update_dvi3.png \
	images/buffer-update_pdf.png \
	images/buffer-update_pdf2.png \
	images/buffer-update_pdf3.png \
	images/buffer-update_pdf4.png \
	images/buffer-update_pdf5.png \
	images/buffer-update_ps.png \
	images/buffer-view.png \
	images/buffer-view_dvi.png \
	images/buffer-view_dvi3.png \
	images/buffer-view_pdf.png \
	images/buffer-view_pdf2.png \
	images/buffer-view_pdf3.png \
	images/buffer-view_pdf4.png \
	images/buffer-view_pdf5.png \
	images/buffer-view_ps.png \
	images/buffer-write-as.png \
	images/buffer-write.png \
	images/build-program.png \
	images/busy.gif \
	images/change-accept.png \
	images/change-next.png \
	images/change-reject.png \
	images/changes-merge.png \
	images/changes-output.png \
	images/changes-track.png \
	images/closetab.png \
	images/tab-group-close.png \
	images/copy.png \
	images/cut.png \
	images/depth-decrement.png \
	images/depth-increment.png \
	images/dialog-preferences.png \
	images/dialog-show-new-inset_citation.png \
	images/dialog-show-new-inset_graphics.png \
	images/dialog-show-new-inset_include.png \
	images/dialog-show-new-inset_ref.png \
	images/dialog-show_character.png \
	images/dialog-show_findreplace.png \
	images/dialog-show_mathdelimiter.png \
	images/dialog-show_mathmatrix.png \
	images/dialog-show_print.png \
	images/dialog-show_spellchecker.png \
	images/dialog-show_vclog.png \
	images/dialog-toggle_findreplaceadv.png \
	images/dialog-toggle_toc.png \
	images/down.png \
	images/editclear.png \
	images/ert-insert.png \
	images/file-open.png \
	images/float-insert_figure.png \
	images/float-insert_table.png \
	images/font-bold.png \
	images/font-emph.png \
	images/textstyle-apply.png \
	images/font-noun.png \
	images/font-sans.png \
	images/footnote-insert.png \
	images/href-insert.png \
	images/hidetab.png \
	images/index-insert.png \
	images/info-insert_buffer_vcs-revision.png \
	images/label-insert.png \
	images/layout.png \
	images/layout-document.png \
	images/layout-paragraph.png \
	images/layout_Description.png \
	images/layout_Enumerate.png  \
	images/layout_Itemize.png \
	images/layout_List.png \
	images/layout_LyX-Code.png \
	images/layout_Chunk.png \
	images/layout_Section.png \
	images/lyx-quit.png \
	images/marginalnote-insert.png \
	images/master-buffer-update.png \
	images/master-buffer-view.png \
	images/math-display.png \
	images/math-macro-add-greedy-optional-param.png \
	images/math-macro-add-optional-param.png \
	images/math-macro-add-param.png \
	images/math-macro-append-greedy-param.png \
	images/math-macro-make-nonoptional.png \
	images/math-macro-make-optional.png \
	images/math-macro-remove-greedy-param.png \
	images/math-macro-remove-optional-param.png \
	images/math-macro-remove-param.png \
	images/math-macro_newmacroname_newcommand.png \
	images/math-matrix.png \
	images/math-mode.png \
	images/math-subscript.png \
	images/math-superscript.png \
	images/nomencl-insert.png \
	images/note-insert.png \
	images/note-next.png \
	images/outline-down.png \
	images/outline-in.png \
	images/outline-out.png \
	images/outline-up.png \
	images/paste.png \
	images/redo.png \
	images/reload.png \
	images/script-insert_subscript.png \
	images/script-insert_superscript.png \
	images/spelling-continuously.png \
	images/view-split_horizontal.png \
	images/view-split_vertical.png \
	images/tabular-feature_align-decimal.png \
	images/tabular-feature_append-column.png \
	images/tabular-feature_append-row.png \
	images/tabular-feature_delete-column.png \
	images/tabular-feature_delete-row.png \
	images/tabular-feature_m-align-center.png \
	images/tabular-feature_m-align-left.png \
	images/tabular-feature_m-align-right.png \
	images/tabular-feature_m-valign-bottom.png \
	images/tabular-feature_m-valign-middle.png \
	images/tabular-feature_m-valign-top.png \
	images/tabular-feature_move-column-left.png \
	images/tabular-feature_move-column-right.png \
	images/tabular-feature_move-row-down.png \
	images/tabular-feature_move-row-up.png \
	images/tabular-feature_multicolumn.png \
	images/tabular-feature_multirow.png \
	images/tabular-feature_set-all-lines.png \
	images/tabular-feature_set-longtabular.png \
	images/tabular-feature_set-rotate-cell.png \
	images/tabular-feature_set-border-lines.png \
	images/tabular-feature_set-rotate-tabular.png \
	images/tabular-feature_toggle-line-bottom.png \
	images/tabular-feature_toggle-line-left.png \
	images/tabular-feature_toggle-line-right.png \
	images/tabular-feature_toggle-line-top.png \
	images/tabular-feature_toggle-rotate-cell.png \
	images/tabular-feature_toggle-rotate-tabular.png \
	images/tabular-feature_unset-all-lines.png \
	images/tabular-insert.png \
	images/thesaurus-entry.png \
	images/toolbar-toggle_math.png \
	images/toolbar-toggle_math_panels.png \
	images/toolbar-toggle_table.png \
	images/undo.png \
	images/unknown.png \
	images/up.png \
	images/update-others.png \
	images/url-insert.png \
	images/vc-check-in.png \
	images/vc-check-out.png \
	images/vc-compare.png \
	images/vc-compare_0.png \
	images/vc-locking-toggle.png \
	images/vc-register.png \
	images/vc-repo-update.png \
	images/vc-revert.png \
	images/view-others.png

dist_images_DATA3X = \
	${dist_images_DATA1X:%.png=%.svgz}

# possible shortcut for complete set of 2x sized icons
#dist_images_DATA2X = \
#	${dist_images_DATA1X:%.png=%@2x.png}

dist_images_DATA2X = \
	images/banner@2x.png

dist_images_DATA = \
	images/amssymb.png \
	images/psnfss1.png \
	images/psnfss2.png \
	images/psnfss3.png \
	images/psnfss4.png \
	images/standard.png \
	${dist_images_DATA1X} \
	${dist_images_DATA2X} \
	${dist_images_DATA3X}

imagesmathdir = $(imagesdir)/math
dist_imagesmath_DATA1X = \
	images/math/AC.png \
	images/math/acute.png \
	images/math/adots.png \
	images/math/agemO.png \
	images/math/aleph.png \
	images/math/alpha.png \
	images/math/amalg.png \
	images/math/angle.png \
	images/math/APLbox.png \
	images/math/APLcomment.png \
	images/math/APLdown.png \
	images/math/APLdownarrowbox.png \
	images/math/APLinput.png \
	images/math/APLinv.png \
	images/math/APLleftarrowbox.png \
	images/math/APLlog.png \
	images/math/APLrightarrowbox.png \
	images/math/APLstar.png \
	images/math/APLup.png \
	images/math/APLuparrowbox.png \
	images/math/apprge.png \
	images/math/apprle.png \
	images/math/approx.png \
	images/math/approxeq.png \
	images/math/aquarius.png \
	images/math/aries.png \
	images/math/arrownot.png \
	images/math/arrownot2.png \
	images/math/arrowvert.png \
	images/math/arrowvert2.png \
	images/math/ascnode.png \
	images/math/ast.png \
	images/math/astrosun.png \
	images/math/asymp.png \
	images/math/ataribox.png \
	images/math/backepsilon.png \
	images/math/backprime.png \
	images/math/backsim.png \
	images/math/backsimeq.png \
	images/math/backslash.png \
	images/math/bar.png \
	images/math/baro.png \
	images/math/bars.png \
	images/math/barwedge.png \
	images/math/Bbbk.png \
	images/math/bbslash.png \
	images/math/bcancel.png \
	images/math/because.png \
	images/math/bell.png \
	images/math/beta.png \
	images/math/beth.png \
	images/math/between.png \
	images/math/bigbox.png \
	images/math/bigcap.png \
	images/math/bigcirc.png \
	images/math/bigcup.png \
	images/math/bigcurlyvee.png \
	images/math/bigcurlywedge.png \
	images/math/biginterleave.png \
	images/math/bignplus.png \
	images/math/bigodot.png \
	images/math/bigoplus.png \
	images/math/bigotimes.png \
	images/math/bigparallel.png \
	images/math/bigsqcap.png \
	images/math/bigsqcup.png \
	images/math/bigstar.png \
	images/math/bigtriangledown.png \
	images/math/bigtriangleup.png \
	images/math/biguplus.png \
	images/math/bigvee.png \
	images/math/bigwedge.png \
	images/math/binampersand.png \
	images/math/bindnasrepma.png \
	images/math/blacklozenge.png \
	images/math/blacksmiley.png \
	images/math/blacksquare.png \
	images/math/blacktriangle.png \
	images/math/blacktriangledown.png \
	images/math/blacktriangleleft.png \
	images/math/blacktriangleright.png \
	images/math/boldsymbol.png \
	images/math/bot.png \
	images/math/bowtie.png \
	images/math/bowtie2.png \
	images/math/box2.png \
	images/math/boxast.png \
	images/math/boxbar.png \
	images/math/boxbox.png \
	images/math/boxbslash.png \
	images/math/boxcircle.png \
	images/math/boxdot.png \
	images/math/boxempty.png \
	images/math/boxminus.png \
	images/math/boxplus.png \
	images/math/boxslash.png \
	images/math/boxtimes.png \
	images/math/breve.png \
	images/math/brokenvert.png \
	images/math/bullet.png \
	images/math/bumpeq.png \
	images/math/bumpeq2.png \
	images/math/cancel.png \
	images/math/cancelto.png \
	images/math/cancer.png \
	images/math/cap.png \
	images/math/cap2.png \
	images/math/capricornus.png \
	images/math/cases.png \
	images/math/cdot.png \
	images/math/cdotp.png \
	images/math/cdots.png \
	images/math/cent.png \
	images/math/centerdot.png \
	images/math/check.png \
	images/math/checked.png \
	images/math/checkedbox2.png \
	images/math/checkmark.png \
	images/math/chi.png \
	images/math/circ.png \
	images/math/circeq.png \
	images/math/circle2.png \
	images/math/circle3.png \
	images/math/circlearrowleft.png \
	images/math/circlearrowright.png \
	images/math/circledR.png \
	images/math/circledS.png \
	images/math/circledast.png \
	images/math/circledcirc.png \
	images/math/circleddash.png \
	images/math/clock.png \
	images/math/clubsuit.png \
	images/math/colon.png \
	images/math/colonapprox.png \
	images/math/colonapprox2.png \
	images/math/coloneq.png \
	images/math/coloneq2.png \
	images/math/coloneqq.png \
	images/math/coloneqq2.png \
	images/math/colonsim.png \
	images/math/colonsim2.png \
	images/math/complement.png \
	images/math/cong.png \
	images/math/conjunction.png \
	images/math/coprod.png \
	images/math/cup.png \
	images/math/cup2.png \
	images/math/curlyeqprec.png \
	images/math/curlyeqsucc.png \
	images/math/curlyvee.png \
	images/math/curlyveedownarrow.png \
	images/math/curlyveeuparrow.png \
	images/math/curlywedge.png \
	images/math/curlywedgedownarrow.png \
	images/math/curlywedgeuparrow.png \
	images/math/currency.png \
	images/math/curvearrowleft.png \
	images/math/curvearrowright.png \
	images/math/dagger.png \
	images/math/daleth.png \
	images/math/dasharrow.png \
	images/math/dashleftarrow.png \
	images/math/dashrightarrow.png \
	images/math/dashv.png \
	images/math/davidsstar.png \
	images/math/dblcolon.png \
	images/math/ddagger.png \
	images/math/ddddot.png \
	images/math/dddot.png \
	images/math/ddot.png \
	images/math/ddots.png \
	images/math/delim.png \
	images/math/delta.png \
	images/math/delta2.png \
	images/math/descnode.png \
	images/math/diagdown.png \
	images/math/diagup.png \
	images/math/diameter.png \
	images/math/diamond.png \
	images/math/diamond2.png \
	images/math/diamondsuit.png \
	images/math/digamma.png \
	images/math/div.png \
	images/math/divideontimes.png \
	images/math/dot.png \
	images/math/doteq.png \
	images/math/doteq2.png \
	images/math/doteqdot.png \
	images/math/dotplus.png \
	images/math/dots.png \
	images/math/dotsb.png \
	images/math/dotsc.png \
	images/math/dotsi.png \
	images/math/dotsint.png \
	images/math/dotsintop.png \
	images/math/dotsm.png \
	images/math/dotso.png \
	images/math/doublebarwedge.png \
	images/math/doublecap.png \
	images/math/doublecup.png \
	images/math/downarrow.png \
	images/math/downarrow2.png \
	images/math/downarrow3.png \
	images/math/downdownarrows.png \
	images/math/downharpoonleft.png \
	images/math/downharpoonright.png \
	images/math/earth.png \
	images/math/eighthnote.png \
	images/math/ell.png \
	images/math/empty.png \
	images/math/emptyset.png \
	images/math/epsilon.png \
	images/math/eqcirc.png \
	images/math/eqcolon.png \
	images/math/eqcolon2.png \
	images/math/eqqcolon.png \
	images/math/eqqcolon2.png \
	images/math/eqslantgtr.png \
	images/math/eqslantless.png \
	images/math/eqsim.png \
	images/math/equation.png \
	images/math/equiv.png \
	images/math/eta.png \
	images/math/eth.png \
	images/math/exists.png \
	images/math/export-others.png \
	images/math/fallingdotseq.png \
	images/math/fatbslash.png \
	images/math/fatsemi.png \
	images/math/fatslash.png \
	images/math/female.png \
	images/math/fint.png \
	images/math/fintop.png \
	images/math/Finv.png \
	images/math/flat.png \
	images/math/font.png \
	images/math/forall.png \
	images/math/frac.png \
	images/math/frac-square.png \
	images/math/frown.png \
	images/math/frownie.png \
	images/math/fullmoon.png \
	images/math/fullnote.png \
	images/math/functions.png \
	images/math/Game.png \
	images/math/gamma.png \
	images/math/gamma2.png \
	images/math/gemini.png \
	images/math/ge.png \
	images/math/geq.png \
	images/math/geqq.png \
	images/math/geqslant.png \
	images/math/gets.png \
	images/math/gg.png \
	images/math/ggg.png \
	images/math/gggtr.png \
	images/math/gimel.png \
	images/math/gluon.png \
	images/math/gnapprox.png \
	images/math/gneq.png \
	images/math/gneqq.png \
	images/math/gnsim.png \
	images/math/grave.png \
	images/math/gtrapprox.png \
	images/math/gtrdot.png \
	images/math/gtreqless.png \
	images/math/gtreqqless.png \
	images/math/gtrless.png \
	images/math/gtrsim.png \
	images/math/gvertneqq.png \
	images/math/halfnote.png \
	images/math/hat.png \
	images/math/hbar.png \
	images/math/heartsuit.png \
	images/math/hexagon.png \
	images/math/hexstar.png \
	images/math/HF.png \
	images/math/hookleftarrow.png \
	images/math/hookrightarrow.png \
	images/math/hphantom.png \
	images/math/hslash.png \
	images/math/iddots.png \
	images/math/idotsint.png \
	images/math/iff.png \
	images/math/iiiint.png \
	images/math/iiiintop.png \
	images/math/iiint.png \
	images/math/iiintop.png \
	images/math/iint.png \
	images/math/iintop.png \
	images/math/Im.png \
	images/math/imath.png \
	images/math/impliedby.png \
	images/math/implies.png \
	images/math/in.png \
	images/math/infty.png \
	images/math/inplus.png \
	images/math/int.png \
	images/math/intercal.png \
	images/math/interleave.png \
	images/math/intop.png \
	images/math/iota.png \
	images/math/invdiameter.png \
	images/math/inve.png \
	images/math/invneg.png \
	images/math/jmath.png \
	images/math/join2.png \
	images/math/jupiter.png \
	images/math/kappa.png \
	images/math/kreuz.png \
	images/math/lambda.png \
	images/math/lambda2.png \
	images/math/land.png \
	images/math/landdownint.png \
	images/math/landdownintop.png \
	images/math/landupint.png \
	images/math/landupintop.png \
	images/math/langle.png \
	images/math/lbag.png \
	images/math/lbag2.png \
	images/math/lbrace.png \
	images/math/lbrace_rbrace.png \
	images/math/lbracket.png \
	images/math/lbracket_rbracket.png \
	images/math/lceil.png \
	images/math/lceil_rceil.png \
	images/math/ldotp.png \
	images/math/ldots.png \
	images/math/le.png \
	images/math/leadsto.png \
	images/math/leftarrow.png \
	images/math/leftarrow2.png \
	images/math/leftarrow3.png \
	images/math/leftarrowtail.png \
	images/math/leftarrowtriangle.png \
	images/math/leftcircle2.png \
	images/math/leftcircle3.png \
	images/math/leftcircle4.png \
	images/math/leftharpoondown.png \
	images/math/leftharpoonup.png \
	images/math/leftleftarrows.png \
	images/math/leftmoon.png \
	images/math/leftrightarrow.png \
	images/math/leftrightarrow2.png \
	images/math/leftrightarroweq.png \
	images/math/leftrightarrows.png \
	images/math/leftrightarrowtriangle.png \
	images/math/leftrightharpoons.png \
	images/math/leftrightsquigarrow.png \
	images/math/leftslice.png \
	images/math/leftthreetimes.png \
	images/math/leftturn.png \
	images/math/leo.png \
	images/math/leq.png \
	images/math/leqq.png \
	images/math/leqslant.png \
	images/math/lessapprox.png \
	images/math/lessdot.png \
	images/math/lesseqgtr.png \
	images/math/lesseqqgtr.png \
	images/math/lessgtr.png \
	images/math/lesssim.png \
	images/math/lfloor.png \
	images/math/lfloor_rfloor.png \
	images/math/lhd.png \
	images/math/lhd2.png \
	images/math/lhook.png \
	images/math/libra.png \
	images/math/lightning.png \
	images/math/ll.png \
	images/math/llbracket.png \
	images/math/llceil.png \
	images/math/llcorner.png \
	images/math/llfloor.png \
	images/math/llparenthesis.png \
	images/math/Lleftarrow.png \
	images/math/lll.png \
	images/math/llless.png \
	images/math/lnapprox.png \
	images/math/lneq.png \
	images/math/lneqq.png \
	images/math/lnot.png \
	images/math/lnsim.png \
	images/math/logof.png \
	images/math/longarrownot.png \
	images/math/longarrownot2.png \
	images/math/longleftarrow.png \
	images/math/longleftarrow2.png \
	images/math/longleftrightarrow.png \
	images/math/longleftrightarrow2.png \
	images/math/longmapsfrom.png \
	images/math/longmapsfrom2.png \
	images/math/longmapsto.png \
	images/math/longmapsto2.png \
	images/math/longrightarrow.png \
	images/math/longrightarrow2.png \
	images/math/looparrowleft.png \
	images/math/looparrowright.png \
	images/math/lor.png \
	images/math/lozenge.png \
	images/math/lparen.png \
	images/math/lparen_rparen.png \
	images/math/lrcorner.png \
	images/math/Lsh.png \
	images/math/ltimes.png \
	images/math/lvertneqq.png \
	images/math/male.png \
	images/math/maltese.png \
	images/math/mapsfrom.png \
	images/math/mapsfrom2.png \
	images/math/mapsfromchar.png \
	images/math/mapsfromchar2.png \
	images/math/mapsto.png \
	images/math/mapsto2.png \
	images/math/mapstochar.png \
	images/math/mapstochar2.png \
	images/math/mars.png \
	images/math/mathbb.png \
	images/math/mathbb_C.png \
	images/math/mathbb_H.png \
	images/math/mathbb_N.png \
	images/math/mathbb_Q.png \
	images/math/mathbb_R.png \
	images/math/mathbb_Z.png \
	images/math/mathbf.png \
	images/math/mathcal.png \
	images/math/mathcal_F.png \
	images/math/mathcal_H.png \
	images/math/mathcal_L.png \
	images/math/mathcal_O.png \
	images/math/mathcircumflex.png \
	images/math/mathclap.png \
	images/math/mathdollar.png \
	images/math/mathfrak.png \
	images/math/mathit.png \
	images/math/mathllap.png \
	images/math/mathparagraph.png \
	images/math/mathscr.png \
	images/math/mathsection.png \
	images/math/mathring.png \
	images/math/mathrlap.png \
	images/math/mathrm.png \
	images/math/mathrm_T.png \
	images/math/mathsf.png \
	images/math/mathtt.png \
	images/math/matrix.png \
	images/math/measuredangle.png \
	images/math/mercury.png \
	images/math/merge.png \
	images/math/mho.png \
	images/math/mid.png \
	images/math/minuso.png \
	images/math/models.png \
	images/math/moo.png \
	images/math/mp.png \
	images/math/mu.png \
	images/math/multimap.png \
	images/math/nabla.png \
	images/math/natural.png \
	images/math/ncong.png \
	images/math/ne.png \
	images/math/nearrow.png \
	images/math/neg.png \
	images/math/neptune.png \
	images/math/neq.png \
	images/math/newmoon.png \
	images/math/nexists.png \
	images/math/ngeq.png \
	images/math/ngeqq.png \
	images/math/ngeqslant.png \
	images/math/ngtr.png \
	images/math/ni.png \
	images/math/niplus.png \
	images/math/nleftarrow.png \
	images/math/nleftarrow2.png \
	images/math/nleftrightarrow.png \
	images/math/nleftrightarrow2.png \
	images/math/nleq.png \
	images/math/nleqq.png \
	images/math/nleqslant.png \
	images/math/nless.png \
	images/math/nmid.png \
	images/math/nnearrow.png \
	images/math/nnwarrow.png \
	images/math/not.png \
	images/math/notin.png \
	images/math/nparallel.png \
	images/math/nplus.png \
	images/math/nprec.png \
	images/math/npreceq.png \
	images/math/nrightarrow.png \
	images/math/nrightarrow2.png \
	images/math/nshortmid.png \
	images/math/nshortparallel.png \
	images/math/nsim.png \
	images/math/nsubseteq.png \
	images/math/nsubseteqq.png \
	images/math/nsucc.png \
	images/math/nsucceq.png \
	images/math/nsupseteq.png \
	images/math/nsupseteqq.png \
	images/math/ntriangleleft.png \
	images/math/ntrianglelefteq.png \
	images/math/ntrianglelefteqslant.png \
	images/math/ntriangleright.png \
	images/math/ntrianglerighteq.png \
	images/math/ntrianglerighteqslant.png \
	images/math/nu.png \
	images/math/nvdash.png \
	images/math/nvdash2.png \
	images/math/nvdash3.png \
	images/math/nvdash4.png \
	images/math/nwarrow.png \
	images/math/obar.png \
	images/math/oblong.png \
	images/math/obslash.png \
	images/math/ocircle.png \
	images/math/octagon.png \
	images/math/odot.png \
	images/math/ogreaterthan.png \
	images/math/oiint.png \
	images/math/oiintop.png \
	images/math/oint.png \
	images/math/ointclockwise.png \
	images/math/ointclockwiseop.png \
	images/math/ointctrclockwise.png \
	images/math/ointctrclockwiseop.png \
	images/math/ointop.png \
	images/math/olessthan.png \
	images/math/omega.png \
	images/math/omega2.png \
	images/math/ominus.png \
	images/math/openo.png \
	images/math/oplus.png \
	images/math/opposition.png \
	images/math/oslash.png \
	images/math/otimes.png \
	images/math/ovee.png \
	images/math/overbrace.png \
	images/math/overleftarrow.png \
	images/math/overleftrightarrow.png \
	images/math/overline.png \
	images/math/overrightarrow.png \
	images/math/overset.png \
	images/math/owedge.png \
	images/math/owns.png \
	images/math/parallel.png \
	images/math/partial.png \
	images/math/pentagon.png \
	images/math/permil.png \
	images/math/perp.png \
	images/math/phantom.png \
	images/math/phi.png \
	images/math/phi2.png \
	images/math/phone.png \
	images/math/photon.png \
	images/math/pi.png \
	images/math/pi2.png \
	images/math/pisces.png \
	images/math/pitchfork.png \
	images/math/pluto.png \
	images/math/pm.png \
	images/math/pointer.png \
	images/math/prec.png \
	images/math/precapprox.png \
	images/math/preccurlyeq.png \
	images/math/preceq.png \
	images/math/precnapprox.png \
	images/math/precneqq.png \
	images/math/precnsim.png \
	images/math/precsim.png \
	images/math/prime.png \
	images/math/prod.png \
	images/math/propto.png \
	images/math/psi.png \
	images/math/psi2.png \
	images/math/quarternote.png \
	images/math/rangle.png \
	images/math/rbag.png \
	images/math/rbag2.png \
	images/math/rbrace.png \
	images/math/rbracket.png \
	images/math/rceil.png \
	images/math/Re.png \
	images/math/recorder.png \
	images/math/restriction.png \
	images/math/rfloor.png \
	images/math/rhd.png \
	images/math/rhd2.png \
	images/math/rho.png \
	images/math/rhook.png \
	images/math/rightarrow.png \
	images/math/rightarrow2.png \
	images/math/rightarrow3.png \
	images/math/rightarrowtail.png \
	images/math/rightarrowtriangle.png \
	images/math/rightcircle2.png \
	images/math/rightcircle3.png \
	images/math/rightcircle4.png \
	images/math/rightharpoondown.png \
	images/math/rightharpoonup.png \
	images/math/rightleftarrows.png \
	images/math/rightleftharpoons.png \
	images/math/rightmoon.png \
	images/math/rightrightarrows.png \
	images/math/rightslice.png \
	images/math/rightsquigarrow.png \
	images/math/rightthreetimes.png \
	images/math/rightturn.png \
	images/math/risingdotseq.png \
	images/math/root.png \
	images/math/rparen.png \
	images/math/rrbracket.png \
	images/math/rrceil.png \
	images/math/rrfloor.png \
	images/math/rrparenthesis.png \
	images/math/Rrightarrow.png \
	images/math/Rsh.png \
	images/math/rtimes.png \
	images/math/sagittarius.png \
	images/math/saturn.png \
	images/math/scorpio.png \
	images/math/searrow.png \
	images/math/setminus.png \
	images/math/sharp.png \
	images/math/shortdownarrow.png \
	images/math/shortleftarrow.png \
	images/math/shortmid.png \
	images/math/shortparallel.png \
	images/math/shortrightarrow.png \
	images/math/shortuparrow.png \
	images/math/sideset.png \
	images/math/sidesetl.png \
	images/math/sidesetn.png \
	images/math/sidesetr.png \
	images/math/sigma.png \
	images/math/sigma2.png \
	images/math/sim.png \
	images/math/simeq.png \
	images/math/slash.png \
	images/math/smallfrown.png \
	images/math/smallint.png \
	images/math/smallsetminus.png \
	images/math/smallsmile.png \
	images/math/smash.png \
	images/math/smashb.png \
	images/math/smasht.png \
	images/math/smile.png \
	images/math/smiley.png \
	images/math/space.png \
	images/math/spadesuit.png \
	images/math/sphericalangle.png \
	images/math/square2.png \
	images/math/sqcap.png \
	images/math/sqcup.png \
	images/math/sqiint.png \
	images/math/sqiintop.png \
	images/math/sqint.png \
	images/math/sqintop.png \
	images/math/sqrt.png \
	images/math/sqrt-square.png \
	images/math/sqsubset.png \
	images/math/sqsubseteq.png \
	images/math/sqsupset.png \
	images/math/sqsupseteq.png \
	images/math/square.png \
	images/math/ssearrow.png \
	images/math/sslash.png \
	images/math/sswarrow.png \
	images/math/stackrel.png \
	images/math/stackrelthree.png \
	images/math/star.png \
	images/math/style.png \
	images/math/sub.png \
	images/math/subset.png \
	images/math/subset2.png \
	images/math/subseteq.png \
	images/math/subseteqq.png \
	images/math/subsetneq.png \
	images/math/subsetneqq.png \
	images/math/subsetplus.png \
	images/math/subsetpluseq.png \
	images/math/succ.png \
	images/math/succapprox.png \
	images/math/succcurlyeq.png \
	images/math/succeq.png \
	images/math/succnapprox.png \
	images/math/succneqq.png \
	images/math/succnsim.png \
	images/math/succsim.png \
	images/math/sum.png \
	images/math/sun.png \
	images/math/super.png \
	images/math/supset.png \
	images/math/supset2.png \
	images/math/supseteq.png \
	images/math/supseteqq.png \
	images/math/supsetneq.png \
	images/math/supsetneqq.png \
	images/math/supsetplus.png \
	images/math/supsetpluseq.png \
	images/math/surd.png \
	images/math/swarrow.png \
	images/math/talloblong.png \
	images/math/tau.png \
	images/math/taurus.png \
	images/math/textrm_AA.png \
	images/math/textrm_O.png \
	images/math/therefore.png \
	images/math/theta.png \
	images/math/theta2.png \
	images/math/textdegree.png \
	images/math/textrm.png \
	images/math/thickapprox.png \
	images/math/thicksim.png \
	images/math/thorn.png \
	images/math/thorn2.png \
	images/math/tilde.png \
	images/math/times.png \
	images/math/to.png \
	images/math/top.png \
	images/math/triangle.png \
	images/math/triangledown.png \
	images/math/triangleleft.png \
	images/math/trianglelefteq.png \
	images/math/trianglelefteqslant.png \
	images/math/triangleq.png \
	images/math/triangleright.png \
	images/math/trianglerighteq.png \
	images/math/trianglerighteqslant.png \
	images/math/twoheadleftarrow.png \
	images/math/twoheadrightarrow.png \
	images/math/twonotes.png \
	images/math/ulcorner.png \
	images/math/underbar.png \
	images/math/underbrace.png \
	images/math/underleftarrow.png \
	images/math/underleftrightarrow.png \
	images/math/underline.png \
	images/math/underrightarrow.png \
	images/math/underscore.png \
	images/math/underset.png \
	images/math/undertilde.png \
	images/math/unlhd.png \
	images/math/unrhd.png \
	images/math/uparrow.png \
	images/math/uparrow2.png \
	images/math/uparrow3.png \
	images/math/updownarrow.png \
	images/math/updownarrow2.png \
	images/math/upharpoonleft.png \
	images/math/upharpoonright.png \
	images/math/uplus.png \
	images/math/upsilon.png \
	images/math/upsilon2.png \
	images/math/upuparrows.png \
	images/math/uranus.png \
	images/math/urcorner.png \
	images/math/utilde.png \
	images/math/varangle.png \
	images/math/varbigcirc.png \
	images/math/varcopyright.png \
	images/math/varcurlyvee.png \
	images/math/varcurlywedge.png \
	images/math/vargamma2.png \
	images/math/vardelta2.png \
	images/math/varepsilon.png \
	images/math/varhexagon.png \
	images/math/varhexstar.png \
	images/math/variiint.png \
	images/math/variiintop.png \
	images/math/variint.png \
	images/math/variintop.png \
	images/math/varint.png \
	images/math/varintop.png \
	images/math/varkappa.png \
	images/math/varlambda2.png \
	images/math/varnothing.png \
	images/math/varoast.png \
	images/math/varobar.png \
	images/math/varobslash.png \
	images/math/varocircle.png \
	images/math/varodot.png \
	images/math/varogreaterthan.png \
	images/math/varoiint.png \
	images/math/varoiintop.png \
	images/math/varoint.png \
	images/math/varointop.png \
	images/math/varointclockwise.png \
	images/math/varointclockwiseop.png \
	images/math/varointctrclockwise.png \
	images/math/varointctrclockwiseop.png \
	images/math/varolessthan.png \
	images/math/varomega2.png \
	images/math/varominus.png \
	images/math/varoplus.png \
	images/math/varoslash.png \
	images/math/varotimes.png \
	images/math/varovee.png \
	images/math/varowedge.png \
	images/math/varphi.png \
	images/math/varphi2.png \
	images/math/varpi.png \
	images/math/varpi2.png \
	images/math/varpropto.png \
	images/math/varpsi2.png \
	images/math/varrho.png \
	images/math/varsigma.png \
	images/math/varsigma2.png \
	images/math/varsubsetneq.png \
	images/math/varsubsetneqq.png \
	images/math/varsupsetneq.png \
	images/math/varsupsetneqq.png \
	images/math/vartheta.png \
	images/math/vartheta2.png \
	images/math/vartimes.png \
	images/math/vartriangle.png \
	images/math/vartriangleleft.png \
	images/math/vartriangleright.png \
	images/math/varupsilon2.png \
	images/math/varxi2.png \
	images/math/vcentcolon.png \
	images/math/vdash.png \
	images/math/vdash2.png \
	images/math/vdash3.png \
	images/math/vdots.png \
	images/math/vec.png \
	images/math/vee.png \
	images/math/veebar.png \
	images/math/venus.png \
	images/math/vernal.png \
	images/math/vert.png \
	images/math/vert2.png \
	images/math/VHF.png \
	images/math/virgo.png \
	images/math/vphantom.png \
	images/math/Vvdash.png \
	images/math/wasylozenge.png \
	images/math/wasypropto.png \
	images/math/wasytherefore.png \
	images/math/wedge.png \
	images/math/widehat.png \
	images/math/widetilde.png \
	images/math/wp.png \
	images/math/wr.png \
	images/math/xbox3.png \
	images/math/xcancel.png \
	images/math/xi.png \
	images/math/xi2.png \
	images/math/xleftarrow.png \
	images/math/xrightarrow.png \
	images/math/Ydown.png \
	images/math/yen.png \
	images/math/Yleft.png \
	images/math/Yright.png \
	images/math/Yup.png \
	images/math/zeta.png

dist_imagesmath_DATA2X = \
	${dist_imagesmath_DATA1X:%.png=%.svgz}

dist_imagesmath_DATA = \
	${dist_imagesmath_DATA1X} \
	${dist_imagesmath_DATA2X}

imagesipadir = $(imagesdir)/ipa
dist_imagesipa_DATA1X = \
	images/ipa/ipamacro-insert_deco_bottomtiebar.png \
	images/ipa/ipamacro-insert_deco_toptiebar.png \
	images/ipa/ipamacro-insert_tone-falling.png \
	images/ipa/ipamacro-insert_tone-high-rising-falling.png \
	images/ipa/ipamacro-insert_tone-high-rising.png \
	images/ipa/ipamacro-insert_tone-low-rising.png \
	images/ipa/ipamacro-insert_tone-rising.png \
	images/ipa/unicode-insert_0x002e.png \
	images/ipa/unicode-insert_0x0061.png \
	images/ipa/unicode-insert_0x0062.png \
	images/ipa/unicode-insert_0x0063.png \
	images/ipa/unicode-insert_0x0064.png \
	images/ipa/unicode-insert_0x0065.png \
	images/ipa/unicode-insert_0x0066.png \
	images/ipa/unicode-insert_0x0068.png \
	images/ipa/unicode-insert_0x0069.png \
	images/ipa/unicode-insert_0x006b.png \
	images/ipa/unicode-insert_0x006c.png \
	images/ipa/unicode-insert_0x006d.png \
	images/ipa/unicode-insert_0x006e.png \
	images/ipa/unicode-insert_0x006f.png \
	images/ipa/unicode-insert_0x0070.png \
	images/ipa/unicode-insert_0x0071.png \
	images/ipa/unicode-insert_0x0072.png \
	images/ipa/unicode-insert_0x0073.png \
	images/ipa/unicode-insert_0x0074.png \
	images/ipa/unicode-insert_0x0075.png \
	images/ipa/unicode-insert_0x0076.png \
	images/ipa/unicode-insert_0x0077.png \
	images/ipa/unicode-insert_0x0078.png \
	images/ipa/unicode-insert_0x0079.png \
	images/ipa/unicode-insert_0x007a.png \
	images/ipa/unicode-insert_0x007c.png \
	images/ipa/unicode-insert_0x00e6.png \
	images/ipa/unicode-insert_0x00e7.png \
	images/ipa/unicode-insert_0x00f0.png \
	images/ipa/unicode-insert_0x00f8.png \
	images/ipa/unicode-insert_0x0127.png \
	images/ipa/unicode-insert_0x014b.png \
	images/ipa/unicode-insert_0x0153.png \
	images/ipa/unicode-insert_0x01c0.png \
	images/ipa/unicode-insert_0x01c1.png \
	images/ipa/unicode-insert_0x01c2.png \
	images/ipa/unicode-insert_0x01c3.png \
	images/ipa/unicode-insert_0x0250.png \
	images/ipa/unicode-insert_0x0251.png \
	images/ipa/unicode-insert_0x0252.png \
	images/ipa/unicode-insert_0x0253.png \
	images/ipa/unicode-insert_0x0254.png \
	images/ipa/unicode-insert_0x0255.png \
	images/ipa/unicode-insert_0x0256.png \
	images/ipa/unicode-insert_0x0257.png \
	images/ipa/unicode-insert_0x0258.png \
	images/ipa/unicode-insert_0x0259.png \
	images/ipa/unicode-insert_0x025b.png \
	images/ipa/unicode-insert_0x025c.png \
	images/ipa/unicode-insert_0x025e.png \
	images/ipa/unicode-insert_0x025f.png \
	images/ipa/unicode-insert_0x0260.png \
	images/ipa/unicode-insert_0x0261.png \
	images/ipa/unicode-insert_0x0262.png \
	images/ipa/unicode-insert_0x0263.png \
	images/ipa/unicode-insert_0x0264.png \
	images/ipa/unicode-insert_0x0265.png \
	images/ipa/unicode-insert_0x0266.png \
	images/ipa/unicode-insert_0x0267.png \
	images/ipa/unicode-insert_0x0268.png \
	images/ipa/unicode-insert_0x026a.png \
	images/ipa/unicode-insert_0x026c.png \
	images/ipa/unicode-insert_0x026d.png \
	images/ipa/unicode-insert_0x026e.png \
	images/ipa/unicode-insert_0x026f.png \
	images/ipa/unicode-insert_0x0270.png \
	images/ipa/unicode-insert_0x0271.png \
	images/ipa/unicode-insert_0x0272.png \
	images/ipa/unicode-insert_0x0273.png \
	images/ipa/unicode-insert_0x0274.png \
	images/ipa/unicode-insert_0x0275.png \
	images/ipa/unicode-insert_0x0276.png \
	images/ipa/unicode-insert_0x0278.png \
	images/ipa/unicode-insert_0x0279.png \
	images/ipa/unicode-insert_0x027a.png \
	images/ipa/unicode-insert_0x027b.png \
	images/ipa/unicode-insert_0x027d.png \
	images/ipa/unicode-insert_0x027e.png \
	images/ipa/unicode-insert_0x0280.png \
	images/ipa/unicode-insert_0x0281.png \
	images/ipa/unicode-insert_0x0282.png \
	images/ipa/unicode-insert_0x0283.png \
	images/ipa/unicode-insert_0x0284.png \
	images/ipa/unicode-insert_0x0288.png \
	images/ipa/unicode-insert_0x0289.png \
	images/ipa/unicode-insert_0x028a.png \
	images/ipa/unicode-insert_0x028b.png \
	images/ipa/unicode-insert_0x028c.png \
	images/ipa/unicode-insert_0x028d.png \
	images/ipa/unicode-insert_0x028e.png \
	images/ipa/unicode-insert_0x028f.png \
	images/ipa/unicode-insert_0x0290.png \
	images/ipa/unicode-insert_0x0291.png \
	images/ipa/unicode-insert_0x0292.png \
	images/ipa/unicode-insert_0x0294.png \
	images/ipa/unicode-insert_0x0295.png \
	images/ipa/unicode-insert_0x0298.png \
	images/ipa/unicode-insert_0x0299.png \
	images/ipa/unicode-insert_0x029b.png \
	images/ipa/unicode-insert_0x029c.png \
	images/ipa/unicode-insert_0x029d.png \
	images/ipa/unicode-insert_0x029f.png \
	images/ipa/unicode-insert_0x02a1.png \
	images/ipa/unicode-insert_0x02a2.png \
	images/ipa/unicode-insert_0x02b0.png \
	images/ipa/unicode-insert_0x02b2.png \
	images/ipa/unicode-insert_0x02b7.png \
	images/ipa/unicode-insert_0x02bc.png \
	images/ipa/unicode-insert_0x02c8.png \
	images/ipa/unicode-insert_0x02cc.png \
	images/ipa/unicode-insert_0x02d0.png \
	images/ipa/unicode-insert_0x02d1.png \
	images/ipa/unicode-insert_0x02de.png \
	images/ipa/unicode-insert_0x02e0.png \
	images/ipa/unicode-insert_0x02e1.png \
	images/ipa/unicode-insert_0x02e4.png \
	images/ipa/unicode-insert_0x02e5.png \
	images/ipa/unicode-insert_0x02e6.png \
	images/ipa/unicode-insert_0x02e7.png \
	images/ipa/unicode-insert_0x02e8.png \
	images/ipa/unicode-insert_0x02e9.png \
	images/ipa/unicode-insert_0x02fa.png \
	images/ipa/unicode-insert_0x0300.png \
	images/ipa/unicode-insert_0x0301.png \
	images/ipa/unicode-insert_0x0302.png \
	images/ipa/unicode-insert_0x0303.png \
	images/ipa/unicode-insert_0x0304.png \
	images/ipa/unicode-insert_0x0306.png \
	images/ipa/unicode-insert_0x0308.png \
	images/ipa/unicode-insert_0x030a.png \
	images/ipa/unicode-insert_0x030b.png \
	images/ipa/unicode-insert_0x030c.png \
	images/ipa/unicode-insert_0x030f.png \
	images/ipa/unicode-insert_0x0318.png \
	images/ipa/unicode-insert_0x0319.png \
	images/ipa/unicode-insert_0x031c.png \
	images/ipa/unicode-insert_0x031d.png \
	images/ipa/unicode-insert_0x031e.png \
	images/ipa/unicode-insert_0x031f.png \
	images/ipa/unicode-insert_0x0320.png \
	images/ipa/unicode-insert_0x0324.png \
	images/ipa/unicode-insert_0x0325.png \
	images/ipa/unicode-insert_0x0329.png \
	images/ipa/unicode-insert_0x032a.png \
	images/ipa/unicode-insert_0x032c.png \
	images/ipa/unicode-insert_0x032f.png \
	images/ipa/unicode-insert_0x0330.png \
	images/ipa/unicode-insert_0x0334.png \
	images/ipa/unicode-insert_0x0339.png \
	images/ipa/unicode-insert_0x033a.png \
	images/ipa/unicode-insert_0x033b.png \
	images/ipa/unicode-insert_0x033c.png \
	images/ipa/unicode-insert_0x033d.png \
	images/ipa/unicode-insert_0x03b2.png \
	images/ipa/unicode-insert_0x03b8.png \
	images/ipa/unicode-insert_0x03c7.png \
	images/ipa/unicode-insert_0x1dc4.png \
	images/ipa/unicode-insert_0x1dc5.png \
	images/ipa/unicode-insert_0x1dc8.png \
	images/ipa/unicode-insert_0x2016.png \
	images/ipa/unicode-insert_0x203f.png \
	images/ipa/unicode-insert_0x207f.png \
	images/ipa/unicode-insert_0x2197.png \
	images/ipa/unicode-insert_0x2198.png \
	images/ipa/unicode-insert_0x2c71.png \
	images/ipa/unicode-insert_0xa71b.png \
	images/ipa/unicode-insert_0xa71c.png

dist_imagesipa_DATA2X = \
	${dist_imagesipa_DATA1X:%.png=%.svgz}

dist_imagesipa_DATA = \
	${dist_imagesipa_DATA1X} \
	${dist_imagesipa_DATA2X}


imagesoxygendir = $(imagesdir)/oxygen
dist_imagesoxygen_DATA1X = \
	images/oxygen/all-changes-accept.png \
	images/oxygen/all-changes-reject.png \
	images/oxygen/bookmark-goto.png \
	images/oxygen/bookmark-goto_0.png \
	images/oxygen/bookmark-save.png \
	images/oxygen/box-insert.png \
	images/oxygen/buffer-new.png \
	images/oxygen/buffer-reload.png \
	images/oxygen/buffer-toggle-output-sync.png \
	images/oxygen/buffer-update.png \
	images/oxygen/buffer-update_dvi.png \
	images/oxygen/buffer-update_dvi3.png \
	images/oxygen/buffer-update_pdf.png \
	images/oxygen/buffer-update_pdf2.png \
	images/oxygen/buffer-update_pdf3.png \
	images/oxygen/buffer-update_pdf4.png \
	images/oxygen/buffer-update_pdf5.png \
	images/oxygen/buffer-update_ps.png \
	images/oxygen/buffer-view.png \
	images/oxygen/buffer-view_dvi.png \
	images/oxygen/buffer-view_dvi3.png \
	images/oxygen/buffer-view_pdf.png \
	images/oxygen/buffer-view_pdf2.png \
	images/oxygen/buffer-view_pdf3.png \
	images/oxygen/buffer-view_pdf4.png \
	images/oxygen/buffer-view_pdf5.png \
	images/oxygen/buffer-view_ps.png \
	images/oxygen/buffer-write-as.png \
	images/oxygen/buffer-write.png \
	images/oxygen/change-accept.png \
	images/oxygen/change-next.png \
	images/oxygen/change-reject.png \
	images/oxygen/changes-output.png \
	images/oxygen/changes-track.png \
	images/oxygen/tab-group-close.png \
	images/oxygen/closetab.png \
	images/oxygen/copy.png \
	images/oxygen/cut.png \
	images/oxygen/depth-decrement.png \
	images/oxygen/depth-increment.png \
	images/oxygen/dialog-show-new-inset_citation.png \
	images/oxygen/dialog-show-new-inset_graphics.png \
	images/oxygen/dialog-show-new-inset_include.png \
	images/oxygen/dialog-show-new-inset_ref.png \
	images/oxygen/dialog-show_character.png \
	images/oxygen/dialog-show_findreplace.png \
	images/oxygen/dialog-show_print.png \
	images/oxygen/dialog-show_spellchecker.png \
	images/oxygen/dialog-toggle_findreplaceadv.png \
	images/oxygen/dialog-toggle_toc.png \
	images/oxygen/ert-insert.png \
	images/oxygen/file-open.png \
	images/oxygen/float-insert_figure.png \
	images/oxygen/float-insert_table.png \
	images/oxygen/font-bold.png \
	images/oxygen/font-emph.png \
	images/oxygen/font-noun.png \
	images/oxygen/footnote-insert.png \
	images/oxygen/href-insert.png \
	images/oxygen/index-insert.png \
	images/oxygen/label-insert.png \
	images/oxygen/layout-document.png \
	images/oxygen/layout-paragraph.png \
	images/oxygen/layout.png \
	images/oxygen/layout_Description.png \
	images/oxygen/layout_Enumerate.png \
	images/oxygen/layout_Itemize.png \
	images/oxygen/layout_List.png \
	images/oxygen/lyx-quit.png \
	images/oxygen/marginalnote-insert.png \
	images/oxygen/master-buffer-update.png \
	images/oxygen/master-buffer-view.png \
	images/oxygen/math-macro_newmacroname_newcommand.png \
	images/oxygen/math-mode.png \
	images/oxygen/nomencl-insert.png \
	images/oxygen/note-insert.png \
	images/oxygen/note-next.png \
	images/oxygen/paste.png \
	images/oxygen/redo.png \
	images/oxygen/reload.png \
	images/oxygen/spelling-continuously.png \
	images/oxygen/view-split_horizontal.png \
	images/oxygen/view-split_vertical.png \
	images/oxygen/tabular-insert.png \
	images/oxygen/textstyle-apply.png \
	images/oxygen/thesaurus-entry.png \
	images/oxygen/toolbar-toggle_math.png \
	images/oxygen/toolbar-toggle_table.png \
	images/oxygen/undo.png \
	images/oxygen/update-others.png \
	images/oxygen/url-insert.png \
	images/oxygen/vc-revert.png \
	images/oxygen/view-others.png

dist_imagesoxygen_DATA2X = \
	${dist_imagesoxygen_DATA1X:%.png=%.svgz}

dist_imagesoxygen_DATA = \
	images/oxygen/iconsize.png \
	${dist_imagesoxygen_DATA1X} \
	${dist_imagesoxygen_DATA2X}

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
	images/classic/info-insert_buffer_vcs-revision.png  \
	images/classic/label-insert.png  \
	images/classic/layout-document.png  \
	images/classic/layout-paragraph.png  \
	images/classic/layout.png  \
	images/classic/layout_Description.png  \
	images/classic/layout_Enumerate.png  \
	images/classic/layout_Itemize.png  \
	images/classic/layout_List.png  \
	images/classic/layout_LyX-Code.png  \
	images/classic/layout_Chunk.png  \
	images/classic/layout_Section.png  \
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
	images/classic/outline-out.png  \
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
	images/classic/tabular-feature_set-all-lines.png  \
	images/classic/tabular-feature_set-border-lines.png  \
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
	kbd/bg-bds-1251.kmap \
	kbd/brazil.kmap \
	kbd/brazil2.kmap \
	kbd/czech-prg.kmap \
	kbd/czech.kmap \
	kbd/european.kmap \
	kbd/farsi.kmap \
	kbd/francais.kmap \
	kbd/french.kmap \
	kbd/german-2.kmap \
	kbd/german-3.kmap \
	kbd/german.kmap \
	kbd/greek.kmap \
	kbd/hebrew.kmap \
	kbd/koi8-r.kmap \
	kbd/koi8-u.kmap \
	kbd/latvian.kmap \
	kbd/magyar-2.kmap \
	kbd/magyar-3.kmap \
	kbd/magyar.kmap \
	kbd/null.kmap \
	kbd/polish.kmap \
	kbd/polski.kmap \
	kbd/portuges.kmap \
	kbd/romanian.kmap \
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
	kbd/espanol.kmap

layoutsdir = $(pkgdatadir)/layouts
dist_layouts_DATA =\
	layouts/a0poster.layout \
	layouts/aa.layout \
	layouts/aapaper.inc \
	layouts/aapaper.layout \
	layouts/aastex.layout \
	layouts/achemso.layout \
	layouts/acm-sigs.layout \
	layouts/acm-sigs-alt.layout \
	layouts/acm-sigs.inc \
	layouts/acmsiggraph.layout \
	layouts/AEA.layout \
	layouts/agutex.layout \
	layouts/agu-dtd.layout \
	layouts/agu_stdclass.inc \
	layouts/agu_stdcounters.inc \
	layouts/agu_stdlists.inc \
	layouts/agu_stdsections.inc \
	layouts/agu_stdtitle.inc \
	layouts/agums.layout \
	layouts/aguplus.inc \
	layouts/algorithm2e.module \
	layouts/amsart.layout \
	layouts/amsbook.layout \
	layouts/amsdefs.inc \
	layouts/apa.layout \
	layouts/apa6.layout \
	layouts/arab-article.layout \
	layouts/article.layout \
	layouts/article-beamer.layout \
	layouts/basic.module \
	layouts/beamer.layout \
	layouts/beamerposter.layout \
	layouts/bicaption.module \
	layouts/book.layout \
	layouts/braille.module \
	layouts/broadway.layout \
	layouts/chess.layout \
	layouts/cl2emult.layout \
	layouts/ctex-article.layout \
	layouts/ctex-book.layout \
	layouts/ctex-report.layout \
	layouts/customHeadersFooters.module \
	layouts/db_lyxmacros.inc \
	layouts/db_stdclass.inc \
	layouts/db_stdcharstyles.inc \
	layouts/db_stdcounters.inc \
	layouts/db_stdlayouts.inc \
	layouts/db_stdlists.inc \
	layouts/db_stdsections.inc \
	layouts/db_stdstarsections.inc \
	layouts/db_stdstruct.inc \
	layouts/db_stdtitle.inc \
	layouts/dinbrief.layout \
	layouts/docbook-book.layout \
	layouts/docbook-chapter.layout \
	layouts/docbook-section.layout \
	layouts/docbook.layout \
	layouts/doublecol-new.layout \
	layouts/dtk.layout \
	layouts/ectaart.layout \
	layouts/egs.layout \
	layouts/elsart.layout \
	layouts/elsarticle.layout \
	layouts/endnotes.module \
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
	layouts/frletter.layout \
	layouts/g-brief.layout \
	layouts/g-brief2.layout \
	layouts/graphicboxes.module \
	layouts/hanging.module \
	layouts/heb-article.layout \
	layouts/heb-letter.layout \
	layouts/hollywood.layout \
	layouts/IEEEtran.layout \
	layouts/IEEEtran-CompSoc.layout \
	layouts/IEEEtran-TransMag.layout \
	layouts/ijmpc.layout \
	layouts/ijmpd.layout \
	layouts/initials.module \
	layouts/iopart.layout \
	layouts/isprs.layout \
	layouts/jarticle.layout \
	layouts/jasatex.layout \
	layouts/jbook.layout \
	layouts/jgrga.layout \
	layouts/jreport.layout \
	layouts/jsarticle.layout \
	layouts/jsbook.layout \
	layouts/jss.layout \
	layouts/jurabib.module \
	layouts/kluwer.layout \
	layouts/knitr.module \
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
	layouts/memoir.layout \
	layouts/minimalistic.module \
	layouts/moderncv.layout \
	layouts/multicol.module \
	layouts/mwart.layout \
	layouts/mwbk.layout \
	layouts/mwrep.layout \
	layouts/natbib.module \
	layouts/natbibapa.module \
	layouts/noweb.module \
	layouts/numarticle.inc \
	layouts/numreport.inc \
	layouts/numrevtex.inc \
	layouts/paper.layout \
	layouts/pdfcomment.module \
	layouts/pdfform.module \
	layouts/powerdot.layout \
	layouts/RJournal.layout \
	layouts/recipebook.layout \
	layouts/report.layout \
	layouts/revtex.layout \
	layouts/revtex4.layout \
	layouts/revtex4-1.layout \
	layouts/rsphrase.module \
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
	layouts/spie.layout \
	layouts/stdciteformats.inc \
	layouts/stdclass.inc \
	layouts/stdcharstyles.inc \
	layouts/stdcounters.inc \
	layouts/stdcustom.inc \
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
	layouts/theorems-ams-extended.module \
	layouts/theorems-ams-extended-bytype.module \
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
	layouts/tufte-handout.layout

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
	scripts/date.py \
	scripts/ext_copy.py \
	scripts/fen2ascii.py \
	scripts/fig2pdftex.py \
	scripts/fig2pstex.py \
	scripts/fig_copy.py \
	scripts/html2latexwrapper.py \
	scripts/include_bib.py \
	scripts/layout2layout.py \
	scripts/legacy_lyxpreview2ppm.py \
	scripts/listerrors \
	scripts/lyxpak.py \
	scripts/lyxpreview2bitmap.py \
	scripts/lyxpreview_tools.py \
	scripts/prefs2prefs.py \
	scripts/prefs2prefs_lfuns.py \
	scripts/prefs2prefs_prefs.py \
	scripts/tex_copy.py \
	scripts/TeXFiles.py

templatesdir = $(pkgdatadir)/templates
dist_templates_DATA = \
	templates/aa.lyx \
	templates/aastex.lyx \
	templates/ACM-siggraph.lyx \
	templates/ACM-sigplan.lyx \
	templates/ACM-SIGS.lyx \
	templates/AEA.lyx \
	templates/AGUTeX.lyx \
	templates/APA.lyx \
	templates/APA6.lyx \
	templates/beamer-conference-ornate-20min.lyx \
	templates/ctex.lyx \
	templates/de_beamer-conference-ornate-20min.lyx \
	templates/dinbrief.lyx \
	templates/DocBook_article.lyx \
	templates/ectaart.lyx \
	templates/elsarticle.lyx \
	templates/EPS-cropped.lyx \
	templates/es_beamer-conference-ornate-20min.lyx \
	templates/fr_beamer-conference-ornate-20min.lyx \
	templates/frletter.lyx \
	templates/g-brief2.lyx \
	templates/g-brief-de.lyx \
	templates/g-brief-en.lyx \
	templates/hollywood.lyx \
	templates/IEEEtran-CompSoc.lyx \
	templates/IEEEtran-Conference.lyx \
	templates/IEEEtran-Journal.lyx \
	templates/IEEEtran-TransMag.lyx \
	templates/IJMPC.lyx \
	templates/IJMPD.lyx \
	templates/IOP-article.lyx \
	templates/ja_beamer-conference-ornate-20min.lyx \
	templates/JASA.lyx \
	templates/JSS-article.lyx \
	templates/kluwer.lyx \
	templates/koma-letter2.lyx \
	templates/letter.lyx \
	templates/lettre.lyx \
	templates/PDF-cropped.lyx \
	templates/poster-a0poster-colored.lyx \
	templates/poster-a0poster-simple.lyx \
	templates/poster-beamerposter.lyx \
	templates/poster-sciposter.lyx \
	templates/README.new_templates \
	templates/RJournal.lyx \
	templates/revtex4.lyx \
	templates/revtex4-1.lyx \
	templates/slides.lyx

springertemplatesdir = $(pkgdatadir)/templates/springer
dist_springertemplates_DATA = \
	templates/springer/svjour3.lyx

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

CLEANFILES += lyx@program_suffix@.desktop lyx.desktop-temp \
              lyx@program_suffix@.svg lyx@program_suffix@.png
endif
dist_noinst_DATA += lyx.desktop.in images/lyx.svg images/lyx.png

install-data-hook:
	$(CHMOD) 755 $(DESTDIR)$(pkgdatadir)/configure.py
	for i in $(DESTDIR)$(scriptsdir)/*.py $(DESTDIR)$(scriptsdir)/listerrors ; do \
		$(CHMOD) 755 $$i; \
	done

alltests: check alltests-recursive

alltests-recursive:
	cd lyx2lyx; $(MAKE) alltests-recursive

