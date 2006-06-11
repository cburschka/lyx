project(boost_signals)

set(boost_signals_sources
	connection.cpp 
	named_slot_map.cpp 
	signal_base.cpp 
	slot.cpp 
	trackable.cpp
)

lyx_add_path(boost_signals_sources ${TOP_SRC_DIR}/boost/libs/signals/src)

add_library(boost_signals STATIC ${boost_signals_sources})
	
