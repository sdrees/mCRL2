configure_file(include/mcrl2/utilities/toolset_version_const.h.in mcrl2/utilities/toolset_version_const.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mcrl2/utilities/toolset_version_const.h
        DESTINATION ${MCRL2_INCLUDE_PATH}/mcrl2/utilities
        COMPONENT Headers)

add_mcrl2_library(utilities
  INSTALL_HEADERS TRUE
  SOURCES
    bitstream.cpp
    command_line_interface.cpp
    logger.cpp
    text_utility.cpp
    toolset_version.cpp
  INCLUDE
    ${Boost_INCLUDE_DIRS}
)

add_subdirectory(example)
