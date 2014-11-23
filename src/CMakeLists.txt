# platform-specific options and dependencies
IF(MINGW)
	# libssh2 is bundled, others are not needed
	SET(wcm_LIBS gdi32 wsock32 mpr)
ELSE(MINGW)
	FIND_PACKAGE(Threads)
	FIND_PACKAGE(X11)

	OPTION(WITH_FREETYPE "Enable freetype support" ON)
	OPTION(WITH_LIBSSH2 "Enable libssh2 support" ON)
	OPTION(WITH_SMBCLIENT "Enable samba support" ON)

	INCLUDE_DIRECTORIES(${X11_X11_INCLUDE_PATH})
	SET(wcm_LIBS ${CMAKE_THREAD_LIBS_INIT} ${X11_X11_LIB})

	IF(WITH_FREETYPE)
		FIND_PACKAGE(Freetype)
		IF(NOT ${FREETYPE_FOUND})
			MESSAGE(FATAL_ERROR "Freetype not found. You should either install it or disable WITH_FREETYPE cmake option")
		ENDIF(NOT ${FREETYPE_FOUND})
		ADD_DEFINITIONS(-DUSEFREETYPE)
		INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})
		SET(wcm_LIBS ${wcm_LIBS} ${FREETYPE_LIBRARIES})
		MESSAGE(STATUS "Freetype support enabled")
	ELSE(WITH_FREETYPE)
		MESSAGE(STATUS "Freetype support disabled")
	ENDIF(WITH_FREETYPE)

	IF(WITH_LIBSSH2)
		FIND_PACKAGE(SSH2)
		IF(NOT ${SSH2_FOUND})
			MESSAGE(FATAL_ERROR "libssh2 not found. You should either install it or disable WITH_LIBSSH2 cmake option")
		ENDIF(NOT ${SSH2_FOUND})
		ADD_DEFINITIONS(-DLIBSSH2_EXIST)
		INCLUDE_DIRECTORIES(${SSH2_INCLUDE_DIR})
		SET(wcm_LIBS ${wcm_LIBS} ${SSH2_LIBRARY})
		MESSAGE(STATUS "libssh2 support enabled")
	ELSE(WITH_LIBSSH2)
		MESSAGE(STATUS "libssh2 support disabled")
	ENDIF(WITH_LIBSSH2)

	IF(WITH_SMBCLIENT)
		FIND_PACKAGE(SMBClient)
		IF(NOT ${SMBCLIENT_FOUND})
			MESSAGE(FATAL_ERROR "libsmbclient not found. You should either install it or disable WITH_SMBCLIENT cmake option")
		ENDIF(NOT ${SMBCLIENT_FOUND})
		ADD_DEFINITIONS(-DLIBSMBCLIENT_EXIST)
		INCLUDE_DIRECTORIES(${SMBCLIENT_INCLUDE_DIR})
		SET(wcm_LIBS ${wcm_LIBS} ${SMBCLIENT_LIBRARY})
		MESSAGE(STATUS "smbclient support enabled")
	ELSE(WITH_SMBCLIENT)
		MESSAGE(STATUS "smbclient support disabled")
	ENDIF(WITH_SMBCLIENT)
ENDIF(MINGW)

# flags
CONFIGURE_FILE(libconf_ux.h.in libconf_ux.h) # noop, flags are passed via compiler args
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(BEFORE wal swl)
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")

# sources
IF(MINGW)
	SET(wcm_PLATFORM_SOURCES
		ext-app.cpp
		swl/swl_wincoreMS.cpp
		w32cons.cpp
		w32util.cpp
	)
ELSE(MINGW)
	SET(wcm_PLATFORM_SOURCES
		ext-app-ux.cpp
		fontdlg.cpp
		shell.cpp
		swl/swl_wincoreUX.cpp
		terminal.cpp
	)
ENDIF(MINGW)

SET(wcm_COMMON_SOURCES
	nc.cpp
	panel.cpp

	charsetdlg.cpp
	color-style.cpp
	dircalc.cpp
	dlg-ctrl-l.cpp
	eloadsave.cpp
	globals.cpp
	fileassociations.cpp
	fileopers.cpp
	filesearch.cpp
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
	shl.cpp
	shortcuts.cpp
	smblogon.cpp
	strconfig.cpp
	strmasks.cpp
	swl/swl_button.cpp
	swl/swl_editline.cpp
	swl/swl_layout.cpp
	swl/swl_menubox.cpp
	swl/swl_menutextinfo.cpp
	swl/swl_popupmenu.cpp
	swl/swl_sbutton.cpp
	swl/swl_scrollbar.cpp
	swl/swl_staticlabel.cpp
	swl/swl_textlist.cpp
	swl/swl_vlist.cpp
	swl/swl_winbase.cpp
	swl/swl_wincore.cpp
	t-emulator.cpp
	tcp_sock.cpp
	termwin.cpp
	toolbar.cpp
	unicode_lc.cpp
	usermenu.cpp
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
	charsetdlg.h
	color-style.h
	dircalc.h
	dlg-ctrl-l.h
	eloadsave.h
	ext-app.h
	globals.h
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
	usermenu.h
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

SET(wcm_RESOURCES
	wcm.rc
)

# targets
ADD_EXECUTABLE(wcm ${wcm_COMMON_SOURCES} ${wcm_PLATFORM_SOURCES} ${wcm_HEADERS} ${wcm_RESOURCES})
TARGET_LINK_LIBRARIES(wcm ${wcm_LIBS})

# install
INSTALL(TARGETS wcm RUNTIME DESTINATION bin)
INSTALL(FILES wcm.desktop DESTINATION share/applications)
INSTALL(FILES install-files/share/wcm/icon/wcm.png DESTINATION share/pixmaps)
