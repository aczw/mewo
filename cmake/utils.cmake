function(mewo_set_target_warnings target)
  target_compile_options(${target} PRIVATE
    -Wall -Wextra -Werror -Wpedantic
    -Wconversion -Wsign-conversion
    -Wshadow
    -Wimplicit-fallthrough

    "$<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast;-Wno-missing-designated-field-initializers>"
    "$<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes;-Wmissing-prototypes>"
  )
endfunction()

function(mewo_set_target_defaults target language)
  if(language STREQUAL "CXX")
     set_target_properties(${target} PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF
    )
  elseif(language STREQUAL "C")
    set_target_properties(${target} PROPERTIES
      C_STANDARD 11
      C_STANDARD_REQUIRED ON
      C_EXTENSIONS OFF
    )
  else()
    message(FATAL_ERROR "Unknown language option: ${language}")
  endif()

  mewo_set_target_warnings(${target})
endfunction()
