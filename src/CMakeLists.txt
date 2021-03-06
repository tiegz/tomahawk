PROJECT( tomahawk )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

IF( "${gui}" STREQUAL "no" )
    SET( QT_DONT_USE_QTGUI TRUE )
ENDIF()
SET( QT_USE_QTSQL TRUE )
SET( QT_USE_QTNETWORK TRUE )
SET( QT_USE_QTXML TRUE )
SET( QT_USE_QTWEBKIT TRUE )

INCLUDE( ${QT_USE_FILE} )
INCLUDE( ${CMAKE_MODULE_PATH}/AddAppIconMacro.cmake )

#SET( CMAKE_BUILD_TYPE "Release" )
SET( CMAKE_VERBOSE_MAKEFILE ON )
SET( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" )
SET( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" )
SET( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" )
SET( TOMAHAWK_INC_DIR "${CMAKE_SOURCE_DIR}/include/" )
IF( NOT CMAKE_BUILD_TYPE STREQUAL "Release" )
    MESSAGE( "Building in debug mode, enabling all debug updates" )
    SET( DEBUG_BUILD ON )
ENDIF()

# build plugins
# use glob, but hardcoded list for now:
#FILE( GLOB plugindirs "src/plugins/*" )
#FOREACH( moddir ${plugindirs} )
#    MESSAGE( status "Building plugin: ${moddir}" )
#    ADD_SUBDIRECTORY( ${moddir} )
#ENDFOREACH( moddir )

SET( tomahawkSources ${tomahawkSources}
     sip/SipHandler.cpp

     infosystem/infosystemcache.cpp
     infosystem/infosystem.cpp
     infosystem/infoplugins/echonestplugin.cpp
     infosystem/infoplugins/lastfmplugin.cpp
     infosystem/infoplugins/musixmatchplugin.cpp

     web/api_v1.cpp

     resolvers/scriptresolver.cpp
     resolvers/qtscriptresolver.cpp

     musicscanner.cpp
     shortcuthandler.cpp
     scanmanager.cpp
     tomahawkapp.cpp
     main.cpp
)

IF(LIBLASTFM_FOUND)
    SET(tomahawkSources ${tomahawkSources}
        scrobbler.cpp
    )
ENDIF(LIBLASTFM_FOUND)

SET( tomahawkSourcesGui ${tomahawkSourcesGui}
     sourcetree/sourcesmodel.cpp
     sourcetree/sourcesproxymodel.cpp
     sourcetree/sourcetreeitem.cpp
     sourcetree/sourcetreeitemwidget.cpp
     sourcetree/sourcetreeview.cpp

     transferview.cpp
     tomahawktrayicon.cpp
     audiocontrols.cpp
     settingsdialog.cpp
     tomahawkwindow.cpp
)

SET( tomahawkHeaders ${tomahawkHeaders}
     "${TOMAHAWK_INC_DIR}/tomahawk/tomahawkapp.h"
     "${TOMAHAWK_INC_DIR}/tomahawk/infosystem.h"

     sip/SipHandler.h

     infosystem/infosystemcache.h
     infosystem/infoplugins/echonestplugin.h
     infosystem/infoplugins/lastfmplugin.h
     infosystem/infoplugins/musixmatchplugin.h

     web/api_v1.h

     resolvers/scriptresolver.h
     resolvers/qtscriptresolver.h

     musicscanner.h
     scanmanager.h
     shortcuthandler.h
)

IF(LIBLASTFM_FOUND)
    SET(tomahawkHeaders ${tomahawkHeaders}
        scrobbler.h
    )
ENDIF(LIBLASTFM_FOUND)


SET( tomahawkHeadersGui ${tomahawkHeadersGui}
     sourcetree/sourcesmodel.h
     sourcetree/sourcesproxymodel.h
     sourcetree/sourcetreeitem.h
     sourcetree/sourcetreeitemwidget.h
     sourcetree/sourcetreeview.h

     transferview.h
     tomahawktrayicon.h
     audiocontrols.h
     settingsdialog.h
     tomahawkwindow.h
)

SET( tomahawkUI ${tomahawkUI}
     tomahawkwindow.ui
     settingsdialog.ui
     proxydialog.ui

     audiocontrols.ui
     sourcetree/sourcetreeitemwidget.ui
)

INCLUDE_DIRECTORIES(
    .
    ${TOMAHAWK_INC_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/thirdparty/liblastfm2/src

    audio
    database
    playlist
    network
    sourcetree
    topbar
    utils
    libtomahawk
    libtomahawk/utils
    infosystem
    mac

    ${THIRDPARTY_DIR}/alsa-playback
    ${THIRDPARTY_DIR}/rtaudio
    ${THIRDPARTY_DIR}/qxt/qxtweb-standalone/qxtweb
    ${THIRDPARTY_DIR}/qtweetlib/qtweetlib/src
    ${THIRDPARTY_DIR}/qtweetlib/tomahawk-custom

    ${TAGLIB_INCLUDES}
    ${QJSON_INCLUDE_DIR}
    ${LIBECHONEST_INCLUDE_DIR}
    ${LIBECHONEST_INCLUDE_DIR}/..
)

SET( OS_SPECIFIC_LINK_LIBRARIES "" )

IF( WIN32 )
  INCLUDE( "CMakeLists.win32.txt" )
ENDIF( WIN32 )
IF( UNIX )
  INCLUDE( "CMakeLists.unix.txt" )
ENDIF( UNIX )

IF( APPLE )
    SET( tomahawkHeaders ${tomahawkHeaders} mac/tomahawkapp_mac.h mac/macshortcuthandler.h  )
    SET( tomahawkSources ${tomahawkSources} mac/tomahawkapp_mac.mm  mac/macshortcuthandler.cpp )

    IF(HAVE_SPARKLE)
        SET( tomahawkHeaders ${tomahawkHeaders} ${SPARKLE}/Headers )
    ENDIF(HAVE_SPARKLE)

ENDIF( APPLE )

IF(GLOOX_FOUND)
  INCLUDE_DIRECTORIES( ${GLOOX_INCLUDE_DIR} )
  SET( tomahawkHeaders ${tomahawkHeaders} xmppbot/xmppbot.h )
  SET( tomahawkSources ${tomahawkSources} xmppbot/xmppbot.cpp )
ENDIF(GLOOX_FOUND)
ADD_SUBDIRECTORY( sip )

kde4_add_app_icon( tomahawkSources "${CMAKE_SOURCE_DIR}/data/icons/tomahawk-icon-*.png" )
qt4_add_resources( RC_SRCS "../resources.qrc" )
qt4_wrap_cpp( tomahawkMoc ${tomahawkHeaders} )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include( ${CMAKE_SOURCE_DIR}/lang/translations.cmake )

SET( final_src ${final_src} ${tomahawkMoc} ${tomahawkSources} ${tomahawkHeaders} ${trans_outfile})

IF( "${gui}" STREQUAL "no" )
ELSE()
    qt4_wrap_ui( tomahawkUI_H ${tomahawkUI} )
    qt4_wrap_cpp( tomahawkMocGui ${tomahawkHeadersGui} )
    SET( final_src ${final_src} ${tomahawkUI_H} ${tomahawkMocGui} ${tomahawkSourcesGui} ${RC_SRCS} )
ENDIF()

IF( UNIX AND NOT APPLE )
    ADD_EXECUTABLE( tomahawk ${final_src} )
ENDIF( UNIX AND NOT APPLE )
IF( APPLE )
    ADD_EXECUTABLE( tomahawk MACOSX_BUNDLE ${final_src} )
    SET_TARGET_PROPERTIES(tomahawk PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist"
)
ENDIF( APPLE )
IF( WIN32 )
    ADD_EXECUTABLE( tomahawk WIN32 ${final_src} )
ENDIF( WIN32 )

MESSAGE( STATUS "OS_SPECIFIC_LINK_LIBRARIES: ${OS_SPECIFIC_LINK_LIBRARIES}" )

SET(LINK_LIBRARIES  "")
IF(LIBLASTFM_FOUND)
    SET(LINK_LIBRARIES ${LINK_LIBRARIES} tomahawk_lastfm2 )
ENDIF(LIBLASTFM_FOUND)
IF(GLOOX_FOUND)
    SET(LINK_LIBRARIES ${LINK_LIBRARIES} ${GLOOX_LIBRARIES} )
ENDIF(GLOOX_FOUND)

TARGET_LINK_LIBRARIES( tomahawk
    ${LINK_LIBRARIES}
    ${TOMAHAWK_LIBRARIES}
    ${OS_SPECIFIC_LINK_LIBRARIES}
    ${QT_LIBRARIES}
    ${MAC_EXTRA_LIBS}
    ${LIBECHONEST_LIBRARY}
    ${QXTWEB_LIBRARIES}
    ${QTWEETLIB_LIBRARIES}
    ${QJSON_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${QTWEETLIB_LIBRARIES}
    ${CLUCENE_LIBRARIES}
)


IF( APPLE )
  IF(HAVE_SPARKLE)
    MESSAGE("Sparkle Found, installing framekwork in bundle")
    INSTALL(DIRECTORY "${SPARKLE}/Versions/Current/Resources" DESTINATION "${CMAKE_BINARY_DIR}/tomahawk.app/Contents/Frameworks/Sparkle.framework")

  ENDIF(HAVE_SPARKLE)
ENDIF( APPLE )

INSTALL( TARGETS tomahawk BUNDLE DESTINATION . RUNTIME DESTINATION bin )

IF(KDE4_FOUND) #install protocol file
  FILE(READ ${CMAKE_SOURCE_DIR}/admin/unix/tomahawk.protocol protocol)
  STRING( REPLACE "/path/to/binary" # match this
              "${CMAKE_INSTALL_PREFIX}/bin/tomahawk" # this is linux (kde) so pretty safe I think
              edited_protocol # save in this variable
              "${protocol}" # from the contents of this var
          )
  FILE( WRITE ${CMAKE_BINARY_DIR}/tomahawk.protocol "${edited_protocol}" )
  INSTALL( FILES ${CMAKE_BINARY_DIR}/tomahawk.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
ENDIF()
#INCLUDE( "CPack.txt" )
