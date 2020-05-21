lessThan(QT_MINOR_VERSION, 8): error("Qt 5.8 and newer is required.")

TEMPLATE = lib

CONFIG += plugin \
          c++11 \
          link_pkgconfig

QT += core \
      gui \
      waylandclient-private

QMAKE_USE += wayland-client

TARGET = material
target.path += $$[QT_INSTALL_PLUGINS]/wayland-decoration-client
INSTALLS += target

SOURCES += materialdecoration.cpp \
           plugin.cpp

HEADERS += materialdecoration.h
