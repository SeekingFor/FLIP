CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

# Basic build file to build thread and mutex classes of dlib

PROJECT(dlib)

SET(DLIB_SRC
dlib/stack_trace.cpp
dlib/threads/threaded_object_extension.cpp
dlib/threads/threads_kernel_1.cpp
dlib/threads/threads_kernel_2.cpp
dlib/threads/threads_kernel_shared.cpp
)

INCLUDE_DIRECTORIES(.)
ADD_LIBRARY(dlib ${DLIB_SRC})
