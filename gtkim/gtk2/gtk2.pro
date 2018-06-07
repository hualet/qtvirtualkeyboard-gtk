include($$PWD/../shared/shared.pri)

CONFIG += link_pkgconfig
PKGCONFIG += gtk+-2.0

message($$LIB_INSTALL_DIR)

!isEmpty(LIB_INSTALL_DIR) {
    target.path = $$LIB_INSTALL_DIR/gtk-2.0/$$system(pkg-config gtk+-2.0 --variable=gtk_binary_version)/immodules/
    INSTALLS += target
}
