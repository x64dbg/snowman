set(MOC_HEADERS
    SnowmanView.h
)

set(SOURCES
    SnowmanView.cpp
    pluginmain.cpp
    pluginmain.h
    QtPlugin.cpp
    QtPlugin.h
    ${MOC_HEADERS}
)

include_directories(${X64DBG_SDK_PATH})

qt4_wrap_cpp(SOURCES ${MOC_HEADERS})
qt5_add_resources(SOURCES x64dbg-module.qrc)

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(EXTENSION .dp64)
else()
    set(EXTENSION .dp32)
endif()

add_library(x64dbg-module SHARED ${SOURCES})
target_link_libraries(x64dbg-module nc-gui nc ${Boost_LIBRARIES} ${QT_LIBRARIES} ${X64DBG_LIBRARY} ${X64BRIDGE_LIBRARY})
set_target_properties(x64dbg-module PROPERTIES OUTPUT_NAME "snowman" PREFIX "" SUFFIX ${EXTENSION})
install(TARGETS x64dbg-module RUNTIME DESTINATION bin LIBRARY DESTINATION lib)

# vim:set et sts=4 sw=4 nospell:
