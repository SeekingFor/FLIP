CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0003 OLD)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(polarssl)

SET(POLARSSL_SRC
library/base64.c
library/bignum.c
library/havege.c
library/rsa.c
library/sha1.c
library/sha2.c
library/sha4.c
library/timing.c
)

INCLUDE_DIRECTORIES(./include)
ADD_LIBRARY(polarssl ${POLARSSL_SRC})
