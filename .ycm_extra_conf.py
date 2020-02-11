def Settings( **kwargs ):
  return {
    'flags': [ '-I/usr/include/ncurses.h' '-lncurses', '-lm', '-Wall', '-Wextra', '-Wconversion' '-std=c99', '-D_POSIX_C_SOURCE=199309L'],
  } 
