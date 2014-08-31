include $(top_srcdir)/config/common.am

EXTRA_DIST = Doxyfile.in CMakeLists.txt

OUTPUT_DIR = .

doxydoc: Doxyfile
	cd $(OUTPUT_DIR) ; doxygen ./Doxyfile

distclean-local:
	rm -rf $(OUTPUT_DIR)/html $(OUTPUT_DIR)/latex $(OUTPUT_DIR)/main

.PHONY: doxydoc
