import os
import installer

#AddOption('--libevent',
#                  dest='libevent',
#                  action='store_true',
#                  default=False
#                  )

TopEnv = Environment()

TopEnv['TARGET_SUFFIX'] = '.so.0.5.0' 

def gen_configh(target, source, env):
	defs = {}
	if TopEnv['libevent'] == 'true':
		defs['USE_LIBEVENT'] = 1
	else:
		defs['USE_LIBEVENT'] = 0
		
	if TopEnv['ssl'] == 'true':
		defs['USE_SSL'] = 1
	else:
		defs['USE_SSL'] = 0		
		
	if TopEnv['curl'] == 'true':
		defs['USE_CURL'] = 1
	else:
		defs['USE_CURL'] = 0	
		
	infile = file(str(source[0]), 'r')
	cbuf = infile.read() % defs 
	cfgfile = file(str(target[0]), 'w')
	cfgfile.write(cbuf)
	cfgfile.close()

def gen_oconf(target, source, env):
	vars = load_ednio_options()
	vars.Save('options.conf', env)

def load_ednio_options():
	vars = Variables("options.conf", ARGUMENTS)
	vars.Add('libevent', '', 'false')
	vars.Add('ssl', '', 'false')
	vars.Add('curl', '', 'false')
	vars.Add('mariadb', '', 'false')
	installer.AddOptions(vars)
	vars.Update( TopEnv )
	TopEnv['includedir'] += '/ednio'
	return vars

def install_libsymlink(target, source, env):
	print '=== installing symlink..., target='
		

#def make_symlink(target, source, env):
#	os.symlink(os.path.abspath(str(source[0])), os.path.abspath(str(target[0])) )

build_target = 'ednio'
if 'configure' in COMMAND_LINE_TARGETS:
	build_target = 'configure'

if 'install' in COMMAND_LINE_TARGETS:
	build_target = 'install'

TopEnv.Append(CPPFLAGS=' -O3 -fmessage-length=0 --std=c++0x -fPIC ')
TopEnv['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1
Export('TopEnv')

curdir = Dir('.').srcnode().abspath


# make configuration
if build_target == 'configure':
	if not TopEnv.GetOption('clean'):
		print 'Start configuring ...'
		if os.path.exists('options.conf'):
			os.remove('options.conf')
		
		print 'Start checking required libraries...'
		conf = Configure(TopEnv)
		if ARGUMENTS.get('libevent') == 'true':
			if not conf.CheckLibWithHeader('event', 'event.h', 'c'):
				print '### Error: not found libevent header'
				Exit(1)
		
		if not conf.CheckFunc('eventfd'):
			print '### Error: not found function'
			Exit(1)
		if not conf.CheckFunc('timerfd_create'):
			print '### Error: not found function'
			Exit(1)
		if not conf.CheckFunc('epoll_wait'):
			print '### Error: not found function'
			Exit(1)
		conf.Finish()
		print 'generating config files...'
		
	TopEnv.Command('options.conf', '', gen_oconf )
	load_ednio_options()
	cfgtarget = TopEnv.AlwaysBuild(TopEnv.Command(curdir+'/src/config.h', curdir+'/src/config.h.in', gen_configh) )
	TopEnv.Alias('configure', ['options.conf', cfgtarget])



# main build
if build_target == 'ednio':
	TopEnv['LIBS']= []
	vars = load_ednio_options()
	obj = SConscript('src/SConscript', variant_dir='out', duplicate=0)
	ednio = TopEnv.SharedLibrary('./out/ednio', obj, SHLIBSUFFIX=TopEnv['TARGET_SUFFIX'])
	builder = Builder(action = "ln -s ${SOURCE.file} ${TARGET.file}", chdir=True)
	TopEnv.Append(BUILDERS = {"Symlink" : builder} )
	mylib_link = TopEnv.Symlink("./out/libednio.so", ednio)

# install
if build_target == 'install':
	TopEnv['LIBS']= []
	vars = load_ednio_options()
	obj = SConscript('src/SConscript', variant_dir='out', duplicate=0)
	ednio = TopEnv.SharedLibrary('./out/ednio', obj, SHLIBSUFFIX=TopEnv['TARGET_SUFFIX'])
	builder = Builder(action = "ln -s ${SOURCE.file} ${TARGET.file}", chdir=True)
	TopEnv.Append(BUILDERS = {"Symlink" : builder} )
	install_link = TopEnv.Symlink( TopEnv['libdir']+'/libednio.so' , ednio)
	
	# install target files.
	install = installer.Installer( TopEnv )
	install.AddLibrary(ednio)
	install.AddHeaders('/home/netmind/prj/ednioprj/ednio/src', '*.h', basedir='', recursive=True)
	TopEnv.Alias('install', install_link)
	#TopEnv.AddPostAction(mylib_link, install_libsymlink)
	

