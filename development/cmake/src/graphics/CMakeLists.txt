project(graphics)

set(graphics_sources
	GraphicsCache.h
	GraphicsCache.C
	GraphicsCacheItem.h
	GraphicsCacheItem.C
	GraphicsConverter.h
	GraphicsConverter.C
	GraphicsImage.h
	GraphicsImage.C
	GraphicsLoader.h
	GraphicsLoader.C
	GraphicsParams.C
	GraphicsParams.h
	LoaderQueue.h
	LoaderQueue.C
	GraphicsTypes.h
	GraphicsTypes.C
	PreviewImage.h
	PreviewImage.C
	PreviewLoader.h
	PreviewLoader.C
	Previews.h
	Previews.C
)

lyx_add_path(graphics_sources ${TOP_SRC_DIR}/src/graphics)

include_directories(${TOP_SRC_DIR}/src/graphics)

add_library(graphics STATIC ${graphics_sources})

