project(frontends)

include_directories(${TOP_SRC_DIR}/src/frontends)

add_subdirectory(controllers) 
add_subdirectory(qt4) 

set(frontends_sources
	Alert.C
	Alert.h
	Alert_pimpl.h
	Dialogs.C
	Dialogs.h
	FileDialog.h
	LyXKeySym.h
	LyXKeySymFactory.h
	LyXScreenFactory.h
	LyXView.C
	LyXView.h
	Menubar.h
	Painter.C
	Painter.h
	Timeout.C
	Timeout.h
	Toolbars.C
	Toolbars.h
	WorkArea.h
	WorkAreaFactory.h
	font_metrics.h
	guiapi.h
	guiapi.C
	key_state.h
	lyx_gui.h
	mouse_state.h
	nullpainter.C
	nullpainter.h
	screen.C
	screen.h
)

lyx_add_path(frontends_sources ${TOP_SRC_DIR}/src/frontends)

add_library(frontends STATIC ${frontends_sources})



