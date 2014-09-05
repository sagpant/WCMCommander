PROJECT(wcm)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# dependencies
FIND_PACKAGE(Threads)
FIND_PACKAGE(X11)

OPTION(WITH_FREETYPE "Enable freetype support" ON)
OPTION(WITH_LIBSSH2 "Enable libssh2 support" ON)
OPTION(WITH_SMBCLIENT "Enable samba support" ON)

# flags
CONFIGURE_FILE(libconf_ux.h.in libconf_ux.h) # noop, flags are passed via compiler args
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(wal swl)
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")

INCLUDE_DIRECTORIES(${X11_X11_INCLUDE_PATH})
SET(wcm_LIBS ${CMAKE_THREAD_LIBS_INIT} ${X11_X11_LIB})

IF(WITH_FREETYPE)
	FIND_PACKAGE(Freetype REQUIRED)
	ADD_DEFINITIONS(-DUSEFREETYPE)
	INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})
	SET(wcm_LIBS ${wcm_LIBS} ${FREETYPE_LIBRARIES})
ENDIF(WITH_FREETYPE)

IF(WITH_LIBSSH2)
	FIND_PACKAGE(SSH2 REQUIRED)
	ADD_DEFINITIONS(-DLIBSSH2_EXIST)
	INCLUDE_DIRECTORIES(${SSH2_INCLUDE_DIR})
	SET(wcm_LIBS ${wcm_LIBS} ${SSH2_LIBRARY})
ENDIF(WITH_LIBSSH2)

IF(WITH_SMBCLIENT)
	FIND_PACKAGE(SMBClient REQUIRED)
	ADD_DEFINITIONS(-DLIBSMBCLIENT_EXIST)
	INCLUDE_DIRECTORIES(${SMBCLIENT_INCLUDE_DIR})
	SET(wcm_LIBS ${wcm_LIBS} ${SMBCLIENT_LIBRARY})
ENDIF(WITH_SMBCLIENT)

# sources
SET(wcm_SOURCES
	nc.cpp
	panel.cpp

	charsetdlg.cpp
	color-style.cpp
	dircalc.cpp
	eloadsave.cpp
	ext-app-ux.cpp
	fileopers.cpp
	filesearch.cpp
	fontdlg.cpp
	ftplogon.cpp
	help.cpp
	helpres.cpp
	ltext.cpp
	ncdialogs.cpp
	ncedit.cpp
	ncfonts.cpp
	nchistory.cpp
	ncview.cpp
	ncwin.cpp
	operthread.cpp
	operwin.cpp
	panel.cpp
	panel_list.cpp
	search-dlg.cpp
	search-tools.cpp
	sftpdlg.cpp
	shell-tools.cpp
	shell.cpp
	shl.cpp
	shortcuts.cpp
	smblogon.cpp
	strconfig.cpp
	swl/swl_button.cpp
	swl/swl_editline.cpp
	swl/swl_layout.cpp
	swl/swl_menubox.cpp
	swl/swl_menutextinfo.cpp
	swl/swl_popupmenu.cpp
	swl/swl_sbutton.cpp
	swl/swl_scrollbar.cpp
	swl/swl_textlist.cpp
	swl/swl_vlist.cpp
	swl/swl_winbase.cpp
	swl/swl_wincore.cpp
	swl/swl_wincoreUX.cpp
	t-emulator.cpp
	tcp_sock.cpp
	terminal.cpp
	termwin.cpp
	toolbar.cpp
	unicode_lc.cpp
	ux_util.cpp
	vfs-ftp.cpp
	vfs-sftp2.cpp
	vfs-smb.cpp
	vfs-uri.cpp
	vfs.cpp
	vfspath.cpp
	wal/wal.cpp
	wal/wal_charset.cpp
	wal/wal_exceptions.cpp
	wal/wal_files.cpp
	wal/wal_sys_api.cpp
	wal/wal_tmpls.cpp
	wcm-config.cpp
)

SET(wcm_HEADERS
	bfile.h
	charsetdlg.h
	color-style.h
	dircalc.h
	eloadsave.h
	ext-app.h
	fileopers.h
	filesearch.h
	fontdlg.h
	ftplogon.h
	help.h
	libconf.h
	ltext.h
	mfile.h
	nc.h
	ncdialogs.h
	ncedit.h
	ncfonts.h
	nchistory.h
	ncview.h
	ncwin.h
	operthread.h
	operwin.h
	panel.h
	panel_list.h
	search-dlg.h
	search-tools.h
	sftpdlg.h
	shell-tools.h
	shell.h
	shl.h
	shortcuts.h
	smblogon.h
	strconfig.h
	swl/swl.h
	swl/swl_internal.h
	swl/swl_layout.h
	swl/swl_winbase.h
	swl/swl_wincore.h
	swl/swl_wincore_internal.h
	t-emulator.h
	tcp_sock.h
	terminal.h
	termwin.h
	toolbar.h
	ux_util.h
	vfs-ftp.h
	vfs-sftp.h
	vfs-smb.h
	vfs-uri.h
	vfs.h
	vfspath.h
	wal/wal.h
	wal/wal_sys_api.h
	wal/wal_tmpls.h
	wcm-config.h
)

# targets
ADD_EXECUTABLE(wcm ${wcm_SOURCES} ${wcm_HEADERS})
TARGET_LINK_LIBRARIES(wcm ${wcm_LIBS})

# install
INSTALL(TARGETS wcm RUNTIME DESTINATION bin)
INSTALL(FILES wcm.desktop DESTINATION share/applications)
INSTALL(FILES small.ico DESTINATION share/pixmaps RENAME wcm.ico)
