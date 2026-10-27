add_executable(mewo_sdl
  main.c
)
mewo_set_target_defaults(mewo_sdl C)
target_link_libraries(mewo_sdl PRIVATE mewo SDL3::SDL3)
