add_executable(mewo_web wrappers.c)
mewo_set_target_defaults(mewo_web C)

target_link_libraries(mewo_web PRIVATE mewo)
target_link_options(mewo_web PRIVATE
  -sMODULARIZE=1 -sEXPORT_ES6=1
  -sENVIRONMENT=web
  -sSINGLE_FILE=1
  -sALLOW_MEMORY_GROWTH=1
  -sEXPORTED_RUNTIME_METHODS=stringToNewUTF8,UTF8ToString
  -sEXPORTED_FUNCTIONS=_free
  --no-entry
  --emit-tsd "${PROJECT_SOURCE_DIR}/hosts/web/src/mewo/generated/mewo.d.ts"
)
set_source_files_properties(wrappers.c PROPERTIES
  COMPILE_OPTIONS -Wno-missing-prototypes
)
set_target_properties(mewo_web PROPERTIES
  OUTPUT_NAME mewo
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/hosts/web/src/mewo/generated"
)
