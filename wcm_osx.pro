TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/nc.cpp \
    src/swl/swl_button.cpp \
    src/swl/swl_editline.cpp \
    src/swl/swl_layout.cpp \
    src/swl/swl_menubox.cpp \
    src/swl/swl_menutextinfo.cpp \
    src/swl/swl_popupmenu.cpp \
    src/swl/swl_sbutton.cpp \
    src/swl/swl_scrollbar.cpp \
    src/swl/swl_textlist.cpp \
    src/swl/swl_vlist.cpp \
    src/swl/swl_winbase.cpp \
    src/swl/swl_wincore.cpp \
    src/swl/swl_wincoreMS.cpp \
    src/swl/swl_wincoreUX.cpp \
    src/wal/wal.cpp \
    src/wal/wal_charset.cpp \
    src/wal/wal_exceptions.cpp \
    src/wal/wal_files.cpp \
    src/wal/wal_sys_api.cpp \
    src/wal/wal_tmpls.cpp \
    src/charsetdlg.cpp \
    src/color-style.cpp \
    src/dircalc.cpp \
    src/dlg-ctrl-l.cpp \
    src/globals.cpp \
    src/eloadsave.cpp \
    src/ext-app-ux.cpp \
    src/ext-app.cpp \
    src/fileassociations.cpp \
    src/fileattributes.cpp \
    src/filehighlighting.cpp \
    src/fileopers.cpp \
    src/filesearch.cpp \
    src/fontdlg.cpp \
    src/ftplogon.cpp \
    src/help.cpp \
    src/helpres.cpp \
    src/ltext.cpp \
    src/ncdialogs.cpp \
    src/ncedit.cpp \
    src/ncfonts.cpp \
    src/nchistory.cpp \
    src/ncview.cpp \
    src/ncwin.cpp \
    src/operthread.cpp \
    src/operwin.cpp \
    src/panel-style.cpp \
    src/panel.cpp \
    src/panel_list.cpp \
    src/search-dlg.cpp \
    src/search-tools.cpp \
    src/sftpdlg.cpp \
    src/shell-tools.cpp \
    src/shell.cpp \
    src/shl.cpp \
    src/path-list.cpp \
    src/folder-shortcuts.cpp \
    src/folder-history.cpp \
    src/nceditline.cpp \
    src/smblogon.cpp \
    src/strconfig.cpp \
    src/strmasks.cpp \
    src/t-emulator.cpp \
    src/tcp_sock.cpp \
    src/terminal.cpp \
    src/termwin.cpp \
    src/toolbar.cpp \
    src/unicode_lc.cpp \
    src/usermenu.cpp \
    src/ux_util.cpp \
    src/vfs/vfs-ftp.cpp \
    src/vfs/vfs-sftp.cpp \
    src/vfs/vfs-sftp2.cpp \
    src/vfs/vfs-smb.cpp \
    src/vfs/vfs-tmp.cpp \
    src/vfs/vfs-uri.cpp \
    src/vfs/vfs.cpp \
    src/vfs/vfspath.cpp \
    src/w32util.cpp \
    src/wcm-config.cpp \
    src/swl/swl_staticlabel.cpp \
    src/utf8proc/utf8proc.c \
    src/urlparser/LUrlParser.cpp

HEADERS += \
    src/swl/swl.h \
    src/swl/swl_internal.h \
    src/swl/swl_layout.h \
    src/swl/swl_winbase.h \
    src/swl/swl_wincore.h \
    src/swl/swl_wincore_freetype_inc.h \
    src/swl/swl_wincore_internal.h \
    src/wal/IntrusivePtr.h \
    src/wal/wal.h \
    src/wal/wal_sys_api.h \
    src/wal/wal_tmpls.h \
    src/charsetdlg.h \
    src/color-style.h \
    src/dircalc.h \
    src/eloadsave.h \
    src/ext-app.h \
    src/fileassociations.h \
    src/filehighlighting.h \
    src/fileopers.h \
    src/filesearch.h \
    src/fontdlg.h \
    src/ftplogon.h \
    src/globals.h \
    src/help.h \
    src/libconf.h \
    src/libconf_osx.h \
    src/ltext.h \
    src/mfile.h \
    src/nc.h \
    src/ncdialogs.h \
    src/ncedit.h \
    src/ncfonts.h \
    src/nchistory.h \
    src/ncview.h \
    src/ncwin.h \
    src/operthread.h \
    src/operwin.h \
    src/panel-style.h \
    src/panel.h \
    src/panel_list.h \
    src/Resource.h \
    src/search-dlg.h \
    src/search-tools.h \
    src/sftpdlg.h \
    src/shell-tools.h \
    src/shell.h \
    src/shl.h \
    src/path-list.h \
    src/folder-shortcuts.h \
    src/folder-history.h \
    src/nceditline.h \
    src/smblogon.h \
    src/strconfig.h \
    src/string-util.h \
    src/strmasks.h \
    src/t-emulator.h \
    src/targetver.h \
    src/tcp_sock.h \
    src/terminal.h \
    src/termwin.h \
    src/toolbar.h \
    src/unicode_lc.h \
    src/usermenu.cpp \
    src/ux_util.h \
    src/vfs/vfs-ftp.h \
    src/vfs/vfs-sftp.h \
    src/vfs/vfs-smb.h \
    src/vfs/vfs-tmp.h \
    src/vfs/vfs-uri.h \
    src/vfs/vfs.h \
    src/vfs/vfspath.h \
    src/w32cons.h \
    src/w32util.h \
    src/wcm-config.h

INCLUDEPATH += wcm
INCLUDEPATH += src
INCLUDEPATH += src/swl
INCLUDEPATH += src/wal
INCLUDEPATH += src/vfs
INCLUDEPATH += src/utf8proc
INCLUDEPATH += /opt/X11/include
INCLUDEPATH += /usr/local/include

macx: {
	QMAKE_MACOSX_DEPLOYMENT_TARGET=10.9
	QMAKE_CXXFLAGS += -mmacosx-version-min=10.9
	QMAKE_CFLAGS += -mmacosx-version-min=10.9
	QMAKE_LFLAGS += -mmacosx-version-min=10.9
	QMAKE_MAC_SDK = macosx10.9
	CMAKE_OSX_SYSROOT = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk
	CMAKE_SYSTEM_FRAMEWORK_PATH = /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk
}
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
CONFIG += c++11
QT += gui
CFLAGS += -I /opt/X11/include/freetype2 -D USEFREETYPE
LIBS += -L/usr/local/lib
LIBS += -L/usr/X11R6/lib
LIBS += -lX11
LIBS += -lfreetype
