include($$PWD/../shared/shared.pri)

CONFIG += link_pkgconfig
PKGCONFIG += gtk+-3.0

!isEmpty(LIB_INSTALL_DIR) {
    target.path = $$LIB_INSTALL_DIR/gtk-3.0/$$system(pkg-config gtk+-3.0 --variable=gtk_binary_version)/immodules/
    INSTALLS += target
}
