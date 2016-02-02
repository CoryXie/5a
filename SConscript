Import('env')

env.Append(CPPPATH=[Dir('.').abspath])

env.Append(YACCFLAGS='--defines=y.tab.h')

env.objs = env.Object(['a.y', 'lex.c', 'bio.c', 'rune.c'])

env.Program('5a', env.objs)
