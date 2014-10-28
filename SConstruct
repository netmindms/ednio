import os
import installer

PRJ_BUILD_VAR_FILE = '.buildvar.conf' 
PRJ_MAJOR_VER = '0'
PRJ_MINOR_VER = '5.0'
PRJ_LIB_SUFFIX = '.so.' + PRJ_MAJOR_VER + '.' + PRJ_MINOR_VER
PRJ_OUT_LIB = 'out/' + 'libedio' + PRJ_LIB_SUFFIX

TopEnv = Environment()

TopEnv['TARGET_SUFFIX'] = PRJ_LIB_SUFFIX


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

	infile = file(curdir+'/src/ednio_config.h.in', 'r')
	cbuf = infile.read() % defs 
	cfgfile = file(str(target[0]), 'w')
	cfgfile.write(cbuf)
	cfgfile.close()

def gen_oconf(target, source, env):
	vars = load_ednio_options()
	vars.Save(PRJ_BUILD_VAR_FILE, env)

def load_ednio_options():
	vars = Variables(PRJ_BUILD_VAR_FILE, ARGUMENTS)
	vars.Add('libevent', '', 'false')
	vars.Add('ssl', '', 'false')
	vars.Add('curl', '', 'false')
	vars.Add('mariadb', '', 'false')
	installer.AddOptions(vars)
	vars.Update( TopEnv )
	TopEnv['includedir'] = TopEnv['prefix'] + '/include/ednio'
	return vars

build_target = 'ednio'
if 'configure' in COMMAND_LINE_TARGETS:
	build_target = 'configure'

if 'install' in COMMAND_LINE_TARGETS:
	build_target = 'install'

TopEnv['LIBS']= []
TopEnv.Append(CCFLAGS=' -O2 -fmessage-length=0 --std=c++0x -fPIC ')
TopEnv['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1
Export('TopEnv')

curdir = Dir('.').srcnode().abspath
#vars = load_ednio_options()

# make configuration
if build_target == 'configure':
	if not TopEnv.GetOption('clean'):
		print 'Start configuring ...'
		if os.path.exists(PRJ_BUILD_VAR_FILE):
			os.remove(PRJ_BUILD_VAR_FILE)
		
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
		
prjvarfile = TopEnv.Command(PRJ_BUILD_VAR_FILE, '', gen_oconf )
cfgtarget = TopEnv.Command(curdir+'/src/ednio_config.h', prjvarfile, gen_configh)
TopEnv.Alias('configure', [PRJ_BUILD_VAR_FILE, cfgtarget])
TopEnv.AddPostAction(cfgtarget, action="@echo '=== Configure Complete ==='")


# ednio target builder
vars = load_ednio_options()
obj = SConscript('src/SConscript', variant_dir='out', duplicate=0)
target_ednio = TopEnv.SharedLibrary('./out/ednio', obj, SHLIBSUFFIX=PRJ_LIB_SUFFIX)
builder = Builder(action = "ln -s ${SOURCE.file} ${TARGET.file}", chdir=True)
TopEnv.Append(BUILDERS = {"Symlink" : builder} )
mylib_link = TopEnv.Symlink("./out/libednio.so", target_ednio)
TopEnv.Alias('ednio', [target_ednio, mylib_link] )
TopEnv.AddPostAction(mylib_link, action="@echo '=== Build Complete ==='")


# install
# install so and headers,
if not TopEnv.GetOption('clean'):
	if 'install' in COMMAND_LINE_TARGETS and not os.path.exists(curdir+'/out/libednio'+PRJ_LIB_SUFFIX):
		print '### Install Fail: build output not found '
		Exit(1)
install = installer.Installer( TopEnv )
install.AddLibrary(target_ednio)
install.AddHeaders(curdir+'/src', '*.h', basedir='', recursive=True)
# install symbolic
install_symbuilder = Builder(action = ["ln -s ${SOURCE.file} ${TARGET.file}"], chdir=True)
TopEnv.Append(BUILDERS = {"Symlink" : install_symbuilder} )
target_install_link = TopEnv.Symlink( TopEnv['libdir']+'/libednio.so' , target_ednio)
TopEnv.Alias('install', target_install_link)
#TopEnv.AddPostAction(target_install_link, action="@echo ' === Install Complete ===' ")	
TopEnv.AddPostAction("install", action="@echo ' === Install Complete ===' ")

Default('ednio')
