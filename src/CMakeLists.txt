# platform-specific options and dependencies
IF(MINGW)
	# libssh2 is bundled, others are not needed
	SET(wcm_LIBS gdi32 wsock32 mpr)
ELSE(MINGW)
	FIND_PACKAGE(Threads)
	FIND_PACKAGE(X11)

	OPTION(WITH_FREETYPE "Enable freetype support" OFF)
	OPTION(WITH_LIBSSH2 "Enable libssh2 support" OFF)
	OPTION(WITH_SMBCLIENT "Enable samba support" OFF)

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
SET(BINDIR bin CACHE STRING "directory where to install binary files")
SET(DESKTOPDIR share/applications CACHE STRING "directory where to install .desktop files")
SET(PIXMAPDIR share/pixmaps CACHE STRING "directory where to install pixmaps")
SET(DATADIR share/wcm CACHE STRING "directory where to install data")

CONFIGURE_FILE(libconf_ux.h.in libconf_ux.h) # noop, flags are passed via compiler args
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(BEFORE wal swl vfs utf8proc .)
ADD_DEFINITIONS(-D_FILE_OFFSET_BITS=64)
ADD_DEFINITIONS(-DUNIX_CONFIG_DIR_PATH="${CMAKE_INSTALL_PREFIX}/${DATADIR}")

IF(MSVC)
	INCLUDE_DIRECTORIES(../libssh2)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /EHsc /RTCc /RTC1 /GS /GR /Oy /Qpar /MP /MTd")
	SET(CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:WINDOWS /NODEFAULTLIB:libcmt /ignore:4099")
	SET(wcm_LIBS
		${wcm_LIBS}
		Ws2_32
		Mpr
		../../libssh2/static32/libssh2
		../../libssh2/static32/ssleay32
		../../libssh2/static32/libeay32
		../../libssh2/static32/zlib
	)

ELSE(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas")
ENDIF(MSVC)

# sources
IF(MINGW OR MSVC)
	SET(wcm_PLATFORM_SOURCES
		ext-app.cpp
		swl/swl_wincoreMS.cpp
		w32cons.cpp
		w32util.cpp
	)
ELSE(MINGW OR MSVC)
	SET(wcm_PLATFORM_SOURCES
		ext-app-ux.cpp
		fontdlg.cpp
		shell.cpp
		swl/swl_wincoreUX.cpp
		terminal.cpp
		utf8proc/utf8proc.c
	)
ENDIF(MINGW OR MSVC)

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
	fileattributes.cpp
	filehighlighting.cpp
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
	path-list.cpp
	folder-shortcuts.cpp
	folder-history.cpp
	smblogon.cpp
	strconfig.cpp
	strmasks.cpp
	swl/swl_button.cpp
	swl/swl_editline.cpp
	swl/swl_combobox.cpp
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
	vfs/vfs-ftp.cpp
	vfs/vfs-sftp2.cpp
	vfs/vfs-smb.cpp
	vfs/vfs-tmp.cpp
	vfs/vfs-uri.cpp
	vfs/vfs.cpp
	vfs/vfspath.cpp
	wal/wal.cpp
	wal/wal_charset.cpp
	wal/wal_exceptions.cpp
	wal/wal_files.cpp
	wal/wal_sys_api.cpp
	wal/wal_tmpls.cpp
	wcm-config.cpp
	urlparser/LUrlParser.cpp
)

SET(wcm_HEADERS
	charsetdlg.h
	color-style.h
	dircalc.h
	dlg-ctrl-l.h
	eloadsave.h
	ext-app.h
	globals.h
	fileassociations.h
	fileattributes.h
	filehighlighting.h
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
	path-list.h
	folder-shortcuts.h
	folder-history.h
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
	vfs/vfs-ftp.h
	vfs/vfs-sftp.h
	vfs/vfs-smb.h
	vfs/vfs-tmp.h
	vfs/vfs-uri.h
	vfs/vfs.h
	vfs/vfspath.h
	wal/wal.h
	wal/wal_sys_api.h
	wal/wal_tmpls.h
	wcm-config.h
	urlparser/LUrlParser.h
)

SET(wcm_RESOURCES
	wcm.rc
)

# targets
ADD_EXECUTABLE(wcm ${wcm_COMMON_SOURCES} ${wcm_PLATFORM_SOURCES} ${wcm_HEADERS} ${wcm_RESOURCES})
TARGET_LINK_LIBRARIES(wcm ${wcm_LIBS})

# install
IF(NOT MSVC)
	INSTALL(TARGETS wcm RUNTIME DESTINATION ${BINDIR})
	INSTALL(FILES ${PROJECT_SOURCE_DIR}/install-files/share/applications/wcm.desktop DESTINATION ${DESKTOPDIR})
	INSTALL(FILES ${PROJECT_SOURCE_DIR}/install-files/share/pixmaps/wcm.png DESTINATION ${PIXMAPDIR})
	INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/install-files/share/wcm/ DESTINATION ${DATADIR})
ENDIF(NOT MSVC)

# groups
IF(MSVC)
	SET(wcm_ALL_FILES
		${wcm_COMMON_SOURCES}
		${wcm_PLATFORM_SOURCES}
		${wcm_HEADERS}
		${wcm_RESOURCES}
	)
	MACRO(SETUP_SOURCE_GROUPS SRC_FILES)
		FOREACH(FILE ${SRC_FILES}) 
			GET_FILENAME_COMPONENT(PARENT_DIR "${FILE}" PATH)
			STRING(REGEX REPLACE "(\\./)?(src|include)/?" "" GROUP "${PARENT_DIR}")
			STRING(REPLACE "/" "\\" GROUP "${GROUP}")
			IF ("${FILE}" MATCHES ".*\\.cpp")
				SET(GROUP "Source Files\\${GROUP}")
			ELSEIF("${FILE}" MATCHES ".*\\.h")
				SET(GROUP "Header Files\\${GROUP}")
			ELSEIF("${FILE}" MATCHES ".*\\.rc")
				SET(GROUP "Windows Resources\\${GROUP}")
			ENDIF()
			SOURCE_GROUP("${GROUP}" FILES "${FILE}")
		ENDFOREACH()
	ENDMACRO()
	SETUP_SOURCE_GROUPS("${wcm_ALL_FILES}")
ENDIF(MSVC)
