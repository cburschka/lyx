include $(top_srcdir)/config/common.am

bundledir = $(prefix)/Contents

spotlightdir = $(bundledir)/Library/Spotlight/LyX-Metadata.mdimporter/Contents
dist_spotlight_DATA = Info.plist 

spotlightbindir = $(spotlightdir)/MacOS
dist_spotlightbin_DATA = LyX-Metadata

spotlightdatadir = $(spotlightdir)/Resources
dist_spotlightdata_DATA = schema.xml

spotlightenglishdir = $(spotlightdatadir)/English.lproj
dist_spotlightenglish_DATA = InfoPlist.strings schema.strings

# Just in case we are doing a parallel install, make sure the
# dependencies between directories is correct. Is there a better way
# to do that?
install-dist_spotlightbinDATA: install-dist_spotlightDATA
install-dist_spotlightdataDATA: install-dist_spotlightDATA
install-dist_spotlightenglishDATA: install-dist_spotlightdataDATA
