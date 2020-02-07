def Settings( **kwargs ):
  return {
    'flags': [ '-I/usr/include/ncurses.h' '-lncurses', '-lm', '-Wall', '-Wextra', '-std=c99'],
  } 
