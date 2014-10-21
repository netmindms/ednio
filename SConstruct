import os
import installer

#AddOption('--libevent',
#                  dest='libevent',
#                  action='store_true',
#                  default=False
#                  )

#if ARGUMENTS.get('config') !='1' and os.path.exists('./src/config.h') is False:
	#print "### Error: config.h file doesn't exist"
	#print "run 'scons config=1 at first'" 
	#Exit()

TopEnv = Environment()

def gen_configh(target, source, env):
	defs = {}
	if TopEnv['USE_LIBEVENT'] == 'true':
		defs['USE_LIBEVENT'] = 1
	else:
		defs['USE_LIBEVENT'] = 0
		
	if TopEnv['USE_SSL'] == 'true':
		defs['USE_SSL'] = 1
	else:
		defs['USE_SSL'] = 0		
		
	if TopEnv['USE_CURL'] == 'true':
		defs['USE_CURL'] = 1
	else:
		defs['USE_CURL'] = 0	
		
	infile = file(str(source[0]), 'r')
	cbuf = infile.read() % defs 
	cfgfile = file(str(target[0]), 'w')
	cfgfile.write(cbuf)
	cfgfile.close()
	infile.close()

build_target = 'ednio'
if 'configure' in COMMAND_LINE_TARGETS:
	build_target = 'configure'

if 'install' in COMMAND_LINE_TARGETS:
	build_target = 'install'


opts = Options('options.conf', ARGUMENTS)
installer.AddOptions( opts )
opts.Update( TopEnv )
opts.Save( 'options.conf', TopEnv )

install = installer.Installer( TopEnv )

TopEnv['USE_LIBEVENT'] = ARGUMENTS.get('libevent', 'false')
TopEnv['USE_SSL'] = ARGUMENTS.get('ssl', 'false')
TopEnv['USE_CURL'] = ARGUMENTS.get('curl', 'false')
TopEnv['LIBS']= []



TopEnv.Append(CPPFLAGS=' -O3 -fmessage-length=0 --std=c++0x -fPIC ')
TopEnv['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1
Export('TopEnv')

curdir = Dir('.').srcnode().abspath

if build_target == 'configure':
	print 'generating config ...'
	print 'curdir = ', curdir
	cfgtarget = TopEnv.AlwaysBuild(TopEnv.Command(curdir+'/src/config.h', curdir+'/src/config.h.in', gen_configh) )
	TopEnv.Alias('configure', cfgtarget)

if 'ednio' in COMMAND_LINE_TARGETS:
	obj = SConscript('src/SConscript', variant_dir='out', duplicate=0)
	ednio = TopEnv.SharedLibrary('./out/ednio', obj)

#install.AddLibrary(ednio)
#install.AddHeaders('/home/netmind/prj/ednioprj/ednio/src', '*.h', basedir='', recursive=True)

#TopEnv.Install('/home/netmind/temp/ednio', ednio)
#TopEnv.Alias('install', '/home/netmind/temp/ednio')


