set(HEADERS
  ddsndgeometry.h
  hlsoundlib.h
  mixer.h
  sdlsound.h
  sndrender.h
  soundload.h
  ssl_lib.h)
set(CPPS
  hlsoundlib.cpp
  sndrender.cpp
  soundload.cpp
  ddsoundload.cpp
  ssl_lib.cpp
  mixer.cpp
  sdlgeometry.cpp
  sdlsound.cpp
)

add_library(sndlib STATIC ${HEADERS} ${CPPS})
target_link_libraries(sndlib PRIVATE
  cfile
  ddebug
  ddio
  logger
  mem
  stream_audio
  physics
  SDL3::SDL3
)
target_include_directories(sndlib PUBLIC
  $<BUILD_INTERFACE:
  ${PROJECT_SOURCE_DIR}/sndlib
  >
  ${SDL3_INCLUDE_DIRS}
)
