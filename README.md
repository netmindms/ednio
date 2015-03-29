ednio
=====

# Event Driven Non Block IO Task Library for C++

Overview
--------

ednio is a simple C++ library that provides event driven scheme on thread.

This provides as follwing features.

- Event Driven Task having event loop and default event callback procedure.
- Easy non block socket API
- Easy Timer API that notify expiration asynchronously.
- Easy non block mode pipe API
- Support non block, asynchronous wrapper API of openssl, curl.

How to build
------------

To build the sources, you need cmake build tool. 
On Ubuntu, you can install it as following.

	$ sudo apt-get install cmake

After you install cmake, do following instructions on top folder of source to build

	$ mkdir mybuild
	$ cd mybuild
	$ cmake ..
	$ make


How to install
--------------
You may want to install this library. To do so, run 'make install' after build.

	$ make install


Typical Usage
-------------
- make your own event driven task by inheriting EdTask class and override OnEventProc() to capture events.
- write initial codes on code block processing EDM_INIT event. EDM_INIT message is sent by ednio framework when task just start.
- Add your specific messages by defining message id since EDM_USER
- call EdTask::run() to start task.
- write cleanup codes on code block processing EDM_CLOSE. EDM_CLOSE message is sent by ednio framework before task terminates.


Socket, Timer, Pipe
-------------------
ednio API basically run asynchronously with non block IO.
It means that you has your codes not blocked with best effort.
Otherwise, events on task(thread) will be delayed.
ednio socket, timer, pipe APIs run on non block mode.
You can get their events as two styles.

First, Use call back interface.
Callback interface is an instance of callback abstract class of socket, timer, pipe.

Second, Override OnXXX virtual functions of each event classes.

To understand detail, refer to example codes.

Task
----
To use ednio event driven APIs, you should make one EdTask at least.
EdTask is a thread having event dispatch loop.
It is responsible for monitoring events and triggering callback for each event.
EdTask has its own specific event dispatching loop. But if you want, you can use libevent in place of ednio event loop. This libevent compatible mode is useful in case you should use any opensource library dependent on libevent.(For example, hiredis library)
To make EdTask run with libevent, call EdTask::run(MODE_LIBEVENT).
Also, You need to build sources with following build argument.

	$ scons configure libevent=true
Make sure that libevent library is installed on your system in advance as well.



IPC between EdTasks
-------------------
ednio allows to make some instances of EdTask.
If you want to communicate between tasks, you can use EdTask::postMsg() or EdTask::sendMsg().
postMsg return immediately after adding the message to que. But sendMsg wait until the message is processed by target task.
Never call sendMsg on the same task(that is, target task), Doing so causes permanent dead-lock on that task.


Thread safe and Thread Local Storage
------------------------------------
Most of ednio API are not thread-safe.(except sendMsg and postMsg)
It is intenional in designe concept. 
I believe the best solution for thread sychronization problem is not making the situation which synchronization is needed.
ednio prefers single-thread, multi-instance model.
EdEvent classes(EdSocket, EdTimer, EdPipe) refer context object stored in thread local storage to determine to run on which task. Therefore, If you opened an event object on A EdTask, callback will be on A EdTask. Also, it is not a good choice to refer event object directly on other task. If you need the situation, use IPC functions.





