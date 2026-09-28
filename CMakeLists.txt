cmake_minimum_required ( VERSION 3.10 )
project ( CrossUI C )
set ( CMAKE_C_STANDARD 11 )
set ( CMAKE_C_STANDARD_REQUIRED ON )
include ( target_warning_options.cmake )

find_package ( PkgConfig )
if ( PkgConfig_FOUND )
    pkg_check_modules ( GTK3 IMPORTED_TARGET gtk+-3.0 )
    #message ( STATUS "${GTK3_FOUND} - ${GTK3_INCLUDE_DIRS} - ${GTK3_LIBRARIES}" )
endif()

if ( NOT TARGET Platform )
    include ( FetchContent )
    FetchContent_Declare (
        Platform
        GIT_REPOSITORY git@github.com:jpincho/Platform
        GIT_TAG main
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/Platform
        )

    FetchContent_MakeAvailable ( Platform )
endif()

file ( GLOB CROSSUI_SOURCES "*.c" "*.h" )

if ( WIN32 )
    message ( STATUS "${PROJECT_NAME} - Building MS Windows backend" )
    file ( GLOB_RECURSE WINDOWS_SOURCES "Windows/*.c" "Windows/*.h" )
	add_library ( CrossUI_Windows ${CROSSUI_SOURCES} ${WINDOWS_SOURCES})
    target_link_libraries ( CrossUI_Windows PUBLIC Platform )
	target_compile_definitions ( CrossUI_Windows PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN )
	target_link_libraries ( CrossUI_Windows PUBLIC user32 gdi32 comctl32 comdlg32 shell32 ole32 )
    target_warning_options ( CrossUI_Windows HIGH AS_ERRORS )
endif()

if ( GTK3_FOUND )
    message ( STATUS "${PROJECT_NAME} - Building GTK3 backend" )
    file ( GLOB_RECURSE GTK3_SOURCES "GTK3/*.c" "GTK3/*.h" )
	add_library ( CrossUI_GTK3 ${CROSSUI_SOURCES} ${GTK3_SOURCES})
    target_link_libraries ( CrossUI_GTK3 PUBLIC Platform )
	target_link_libraries ( CrossUI_GTK3 PUBLIC PkgConfig::GTK3 )
    target_include_directories ( CrossUI_GTK3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )
    target_warning_options ( CrossUI_GTK3 HIGH AS_ERRORS )
endif()

option ( CROSSUI_BUILD_DEMO "Build the CrossUI demo application" ON )
if ( CROSSUI_BUILD_DEMO )
    message ( STATUS "${PROJECT_NAME} - Building test app")
    if ( WIN32 )
        add_executable ( CrossUIDemo_Windows Test/demo.c )
        target_link_libraries ( CrossUIDemo_Windows PRIVATE CrossUI_Windows )
        target_include_directories ( CrossUIDemo_Windows PRIVATE ${CMAKE_SOURCE_DIR} )
    endif()
    if ( GTK3_FOUND )
        add_executable ( CrossUIDemo_GTK3 Test/demo.c )
        target_link_libraries ( CrossUIDemo_GTK3 PRIVATE CrossUI_GTK3 )
        target_include_directories ( CrossUIDemo_GTK3 PRIVATE ${CMAKE_SOURCE_DIR} )
    endif()
endif()
