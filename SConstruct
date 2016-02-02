# Let's define a common build environment first...
common_env = Environment()
common_env.Append(CPPDEFINES={'VERSION': 1})

# Our release build is derived from the common build environment...
release_env = common_env.Clone()
# ... and adds a RELEASE preprocessor symbol ...
release_env.Append(CPPDEFINES=['RELEASE'])
release_env.Append(CCFLAGS = ['-O2'])
# ... and release builds end up in the "build/release" dir
release_env.VariantDir('build/release', '.', duplicate=0)
release_env.objs = list()

# We define our debug build environment in a similar fashion...
debug_env = common_env.Clone()
debug_env.Append(CPPDEFINES=['DEBUG'])
debug_env.Append(CCFLAGS = ['-ggdb'])
debug_env.VariantDir('build/debug', '.', duplicate=0)
debug_env.objs = list()

# Now that all build environment have been defined, let's iterate over
# them and invoke the lower level SConscript files.
for mode, env in dict(release=release_env, 
    	       	      debug=debug_env).iteritems():
    env.SConscript('build/%s/SConscript' % mode, {'env': env})


