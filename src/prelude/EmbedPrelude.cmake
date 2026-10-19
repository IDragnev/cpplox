# Run in script mode at build time: cmake -DINPUT=... -DTEMPLATE=... -DOUTPUT=... -P EmbedPrelude.cmake
file(READ ${INPUT} PRELUDE_SOURCE)

string(FIND "${PRELUDE_SOURCE}" ")lox\"" delimiterAt)
if(NOT delimiterAt EQUAL -1)
  message(FATAL_ERROR "${INPUT} contains ')lox\"', which ends the raw string it is embedded in.")
endif()

configure_file(${TEMPLATE} ${OUTPUT} @ONLY)
