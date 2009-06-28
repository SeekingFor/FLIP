CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(flip)

OPTION(USE_BUNDLED_SQLITE "Use the bundled SQLite3 library." ON)
IF(APPLE)
	SET(USE_BUNDLED_SQLITE ON)
ENDIF(APPLE)

SET(FLIP_SRC
src/datetime.cpp
src/dbsetup.cpp
src/flipevent.cpp
src/flipeventsource.cpp
src/global.cpp
src/logfile.cpp
src/main.cpp
src/option.cpp
src/optionssetup.cpp
src/stringfunctions.cpp
src/db/sqlite3db/sqlite3db.cpp
src/db/sqlite3db/sqlite3recordset.cpp
src/db/sqlite3db/sqlite3statement.cpp
src/freenet/dbmaintenance.cpp
src/freenet/fcpv2.cpp
src/freenet/freenetconnection.cpp
src/freenet/freenetidentityannouncer.cpp
src/freenet/freenetidentityinserter.cpp
src/freenet/freenetidentityrequester.cpp
src/freenet/freenetmessage.cpp
src/freenet/freenetmessagedownloader.cpp
src/freenet/freenetmessageeditionpoller.cpp
src/freenet/freenetmessagefinder.cpp
src/freenet/freenetmessageinserter.cpp
src/freenet/freenetnewidentityfinder.cpp
src/freenet/freenetunkeyedidentitycreator.cpp
src/irc/ircchannel.cpp
src/irc/ircclientconnection.cpp
src/irc/irccommand.cpp
src/irc/irccommandresponse.cpp
src/irc/ircnick.cpp
src/irc/ircserver.cpp
)

# Get a relative list of include files.
FILE(GLOB_RECURSE FLIP_INC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} include/*.h)

# Create source groups for all sources and includes, in their original directory structure.
FOREACH(SRC ${FLIP_SRC} ${FLIP_INC})
	STRING(REGEX REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "Files" REL_DIR "${SRC}")
	STRING(REGEX REPLACE "[\\\\/][^\\\\/]*$" "" REL_DIR "${REL_DIR}")
	STRING(REGEX REPLACE "^[\\\\/]" "" REL_DIR "${REL_DIR}")
	IF(REL_DIR)
		STRING(REPLACE "/" "\\" REL_DIR "${REL_DIR}")
		SOURCE_GROUP(${REL_DIR} FILES ${SRC})
	ELSE(REL_DIR)
		SOURCE_GROUP(Files FILES ${SRC})
	ENDIF(REL_DIR)
ENDFOREACH(SRC)

ADD_EXECUTABLE(flip ${FLIP_SRC} ${FLIP_INC})

# link dl - For SQLite3 - not for FreeBSD
IF(CMAKE_COMPILER_IS_GNUCC)
	IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	ELSE(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
		TARGET_LINK_LIBRARIES(flip dl)
	ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# add -lcompat only for FreeBSD
IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	IF(CMAKE_COMPILER_IS_GNUCXX)
		TARGET_LINK_LIBRARIES(flip compat)
	ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

# link ws2_32 and iphlpapi for Windows
IF(WIN32)
	TARGET_LINK_LIBRARIES(flip ws2_32)
	TARGET_LINK_LIBRARIES(flip iphlpapi)
ELSE(WIN32)
	ADD_SUBDIRECTORY(libs/librock)
	TARGET_LINK_LIBRARIES(flip librock)
	INCLUDE_DIRECTORIES(libs/librock)
ENDIF(WIN32)

# add -lxnet and -lsocket on solaris
IF(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	TARGET_LINK_LIBRARIES(flip xnet)
	TARGET_LINK_LIBRARIES(flip socket)
ENDIF(CMAKE_SYSTEM_NAME STREQUAL "SunOS")

IF(NOT USE_BUNDLED_SQLITE)
	FIND_LIBRARY(SQLITE3_LIBRARY NAMES sqlite3 sqlite3_s)
ENDIF(NOT USE_BUNDLED_SQLITE)

FIND_LIBRARY(PTHREADS_LIBRARY NAMES pthread pthreads pthreadvc2)
IF(NOT WIN32)
	IF(PTHREADS_LIBRARY)
		TARGET_LINK_LIBRARIES(flip ${PTHREADS_LIBRARY})
	ELSE(PTHREADS_LIBRARY)
		MESSAGE(FATAL_ERROR "Could not find system pthread library.  You must set the location manually.")
	ENDIF(PTHREADS_LIBRARY)
ENDIF(NOT WIN32)

IF(SQLITE3_LIBRARY)
	MESSAGE(STATUS "Linking against system SQLite3 library.")
	TARGET_LINK_LIBRARIES(flip ${SQLITE3_LIBRARY})
ELSE(SQLITE3_LIBRARY)
	IF(NOT USE_BUNDLED_SQLITE)
		MESSAGE(STATUS "Could not find system SQLite3 library.  Will compile from included source.")
	ENDIF(NOT USE_BUNDLED_SQLITE)
	ADD_LIBRARY(sqlite3 libs/sqlite3/sqlite3.c)
	TARGET_LINK_LIBRARIES(flip sqlite3)
	INCLUDE_DIRECTORIES(libs/sqlite3)
ENDIF(SQLITE3_LIBRARY)
