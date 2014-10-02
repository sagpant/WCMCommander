TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += wcm/nc.cpp \
    wcm/swl/swl_button.cpp \
    wcm/swl/swl_editline.cpp \
    wcm/swl/swl_layout.cpp \
    wcm/swl/swl_menubox.cpp \
    wcm/swl/swl_menutextinfo.cpp \
    wcm/swl/swl_popupmenu.cpp \
    wcm/swl/swl_sbutton.cpp \
    wcm/swl/swl_scrollbar.cpp \
    wcm/swl/swl_textlist.cpp \
    wcm/swl/swl_vlist.cpp \
    wcm/swl/swl_winbase.cpp \
    wcm/swl/swl_wincore.cpp \
    wcm/swl/swl_wincoreMS.cpp \
    wcm/swl/swl_wincoreUX.cpp \
    wcm/wal/wal.cpp \
    wcm/wal/wal_charset.cpp \
    wcm/wal/wal_exceptions.cpp \
    wcm/wal/wal_files.cpp \
    wcm/wal/wal_sys_api.cpp \
    wcm/wal/wal_tmpls.cpp \
    wcm/charsetdlg.cpp \
    wcm/color-style.cpp \
    wcm/dircalc.cpp \
    wcm/globals.cpp \
    wcm/eloadsave.cpp \
    wcm/ext-app-ux.cpp \
    wcm/ext-app.cpp \
    wcm/fileassociations.cpp \
    wcm/filehighlighting.cpp \
    wcm/fileopers.cpp \
    wcm/filesearch.cpp \
    wcm/fontdlg.cpp \
    wcm/ftplogon.cpp \
    wcm/help.cpp \
    wcm/helpres.cpp \
    wcm/ltext.cpp \
    wcm/ncdialogs.cpp \
    wcm/ncedit.cpp \
    wcm/ncfonts.cpp \
    wcm/nchistory.cpp \
    wcm/ncview.cpp \
    wcm/ncwin.cpp \
    wcm/operthread.cpp \
    wcm/operwin.cpp \
    wcm/panel-style.cpp \
    wcm/panel.cpp \
    wcm/panel_list.cpp \
    wcm/search-dlg.cpp \
    wcm/search-tools.cpp \
    wcm/sftpdlg.cpp \
    wcm/shell-tools.cpp \
    wcm/shell.cpp \
    wcm/shl.cpp \
    wcm/shortcuts.cpp \
    wcm/smblogon.cpp \
    wcm/strconfig.cpp \
    wcm/t-emulator.cpp \
    wcm/tcp_sock.cpp \
    wcm/terminal.cpp \
    wcm/termwin.cpp \
    wcm/toolbar.cpp \
    wcm/unicode_lc.cpp \
    wcm/ux_util.cpp \
    wcm/vfs-ftp.cpp \
    wcm/vfs-sftp.cpp \
    wcm/vfs-sftp2.cpp \
    wcm/vfs-smb.cpp \
    wcm/vfs-uri.cpp \
    wcm/vfs.cpp \
    wcm/vfspath.cpp \
    wcm/w32util.cpp \
    wcm/wcm-config.cpp \
    wcm/swl/swl_staticlabel.cpp

HEADERS += \
    wcm/swl/swl.h \
    wcm/swl/swl_internal.h \
    wcm/swl/swl_layout.h \
    wcm/swl/swl_winbase.h \
    wcm/swl/swl_wincore.h \
    wcm/swl/swl_wincore_freetype_inc.h \
    wcm/swl/swl_wincore_internal.h \
    wcm/wal/IntrusivePtr.h \
    wcm/wal/wal.h \
    wcm/wal/wal_sys_api.h \
    wcm/wal/wal_tmpls.h \
    wcm/charsetdlg.h \
    wcm/color-style.h \
    wcm/dircalc.h \
    wcm/eloadsave.h \
    wcm/ext-app.h \
    wcm/fileassociations.h \
    wcm/filehighlighting.h \
    wcm/fileopers.h \
    wcm/filesearch.h \
    wcm/fontdlg.h \
    wcm/ftplogon.h \
    wcm/globals.h \
    wcm/help.h \
    wcm/libconf.h \
    wcm/libconf_osx.h \
    wcm/ltext.h \
    wcm/mfile.h \
    wcm/nc.h \
    wcm/ncdialogs.h \
    wcm/ncedit.h \
    wcm/ncfonts.h \
    wcm/nchistory.h \
    wcm/ncview.h \
    wcm/ncwin.h \
    wcm/operthread.h \
    wcm/operwin.h \
    wcm/panel-style.h \
    wcm/panel.h \
    wcm/panel_list.h \
    wcm/Resource.h \
    wcm/search-dlg.h \
    wcm/search-tools.h \
    wcm/sftpdlg.h \
    wcm/shell-tools.h \
    wcm/shell.h \
    wcm/shl.h \
    wcm/shortcuts.h \
    wcm/smblogon.h \
    wcm/strconfig.h \
    wcm/string-util.h \
    wcm/t-emulator.h \
    wcm/targetver.h \
    wcm/tcp_sock.h \
    wcm/terminal.h \
    wcm/termwin.h \
    wcm/toolbar.h \
    wcm/unicode_lc.h \
    wcm/ux_util.h \
    wcm/vfs-ftp.h \
    wcm/vfs-sftp.h \
    wcm/vfs-smb.h \
    wcm/vfs-uri.h \
    wcm/vfs.h \
    wcm/vfspath.h \
    wcm/w32cons.h \
    wcm/w32util.h \
    wcm/wcm-config.h

INCLUDEPATH += wcm
INCLUDEPATH += wcm/swl
INCLUDEPATH += wcm/wal
INCLUDEPATH += /opt/X11/include
INCLUDEPATH += /usr/local/include

QMAKE_MACOSX_DEPLOYMENT_TARGET=10.9
QMAKE_CXXFLAGS += -mmacosx-version-min=10.9
QMAKE_CFLAGS += -mmacosx-version-min=10.9
QMAKE_LFLAGS += -mmacosx-version-min=10.9
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
QMAKE_CXXFLAGS_WARN_ON += -Wno-unknown-pragmas
QMAKE_CXXFLAGS_WARN_ON += -Wno-reorder
QMAKE_CXXFLAGS_WARN_ON += -Wno-overloaded-virtual
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-const-variable
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-function
QMAKE_CXXFLAGS_WARN_ON += -Wno-sign-compare
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-variable
QMAKE_CXXFLAGS_WARN_ON += -Wno-logical-op-parentheses
QMAKE_CXXFLAGS_WARN_ON += -Wno-bitwise-op-parentheses
QMAKE_CFLAGS += -std=c99
QMAKE_MAC_SDK = macosx10.9
CONFIG += c++11
QT += gui
CFLAGS += -I /opt/X11/include/freetype2 -D USEFREETYPE
LIBS += -L/usr/local/lib
LIBS += -L/usr/X11R6/lib
LIBS += -lX11
LIBS += -lfreetype
CMAKE_OSX_SYSROOT = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk
CMAKE_SYSTEM_FRAMEWORK_PATH = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk
