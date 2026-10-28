function(mewo_set_target_defaults target language)
  if(language STREQUAL "CXX")
     set_target_properties(${target} PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF
    )
    target_compile_options(${target} PRIVATE
      -Wold-style-cast
      -Wextra-semi
    )
  elseif(language STREQUAL "C")
    set_target_properties(${target} PROPERTIES
      C_STANDARD 11
      C_STANDARD_REQUIRED ON
      C_EXTENSIONS OFF
    )
    target_compile_options(${target} PRIVATE
      -Wstrict-prototypes
      -Wmissing-prototypes
    )
  else()
    message(FATAL_ERROR "Unknown language option: ${language}")
  endif()

  target_compile_options(${target} PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wconversion -Wsign-conversion
    -Wshadow
    -Wimplicit-fallthrough
  )

  target_compile_definitions(${target} PRIVATE
    MEWO_VERSION="${PROJECT_VERSION}"
    MEWO_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}"
    MEWO_VERSION_MINOR="${PROJECT_VERSION_MINOR}"
    MEWO_VERSION_PATCH="${PROJECT_VERSION_PATCH}"
    MEWO_VERSION_BUILD="${PROJECT_VERSION_TWEAK}"
  )
endfunction()
