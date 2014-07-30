import os
AddOption('--libevent',
                  dest='libevent',
                  action='store_true',
                  default=False
                  )
 
TopEnv = Environment()
TopEnv['USE_LIBEVENT'] = ARGUMENTS.get('libevent', 'false')
TopEnv['LIBS']= []

TopEnv.Append(CPPFLAGS='-g -O3 -fmessage-length=0 --std=c++0x -fPIC ')
TopEnv['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1
Export('TopEnv')
obj = SConscript('src/SConscript', variant_dir='out', duplicate=0)
#TopEnv.SharedLibrary('./out/ednio', obj, LIBS=['pthread', 'dl', 'rt'  ] )
TopEnv.SharedLibrary('./out/ednio', obj)
