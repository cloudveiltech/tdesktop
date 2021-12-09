# This file is part of Desktop App Toolkit,
# a set of libraries for developing nice desktop applications.
#
# For license and copyright information please follow this link:
# https://github.com/desktop-app/legal/blob/master/LEGAL

if (NOT DESKTOP_APP_QT6)
    find_package(ECM REQUIRED)
    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
    find_package(QtWaylandScanner REQUIRED)
endif()

add_library(lib_waylandshells_xdg_shell STATIC)
add_library(desktop-app::lib_waylandshells::xdg_shell ALIAS lib_waylandshells_xdg_shell)
init_target(lib_waylandshells_xdg_shell)

get_filename_component(src_loc . REALPATH)
set_target_properties(lib_waylandshells_xdg_shell PROPERTIES AUTOMOC ON)

if (DESKTOP_APP_QT6)
    qt6_generate_wayland_protocol_client_sources(lib_waylandshells_xdg_shell
    FILES
        ${src_loc}/xdg-shell.xml
        ${src_loc}/xdg-decoration-unstable-v1.xml
    )
else()
    ecm_add_qtwayland_client_protocol(xdg_shell_protocol_srcs
        PROTOCOL ${src_loc}/xdg-shell.xml
        BASENAME xdg-shell
    )

    ecm_add_qtwayland_client_protocol(xdg_decoration_protocol_srcs
        PROTOCOL ${src_loc}/xdg-decoration-unstable-v1.xml
        BASENAME xdg-decoration-unstable-v1
    )
endif()

nice_target_sources(lib_waylandshells_xdg_shell ${src_loc}
PRIVATE
    main.cpp
    qwaylandxdgdecorationv1.cpp
    qwaylandxdgdecorationv1_p.h
    qwaylandxdgshell.cpp
    qwaylandxdgshell_p.h
    qwaylandxdgshellintegration.cpp
    qwaylandxdgshellintegration_p.h
)

if (NOT DESKTOP_APP_QT6)
    target_sources(lib_waylandshells_xdg_shell
    PRIVATE
        ${xdg_shell_protocol_srcs}
        ${xdg_decoration_protocol_srcs}
    )
endif()

target_include_directories(lib_waylandshells_xdg_shell
PRIVATE
    ${src_loc}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(lib_waylandshells_xdg_shell
PRIVATE
    -UQT_NO_KEYWORDS
)

target_compile_definitions(lib_waylandshells_xdg_shell
PRIVATE
    QT_STATICPLUGIN
)

target_link_libraries(lib_waylandshells_xdg_shell
PRIVATE
    desktop-app::external_qt
)

if (DESKTOP_APP_USE_PACKAGED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
    target_link_libraries(lib_waylandshells_xdg_shell PRIVATE PkgConfig::WAYLAND_CLIENT)
endif()
