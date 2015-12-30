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

Example
--------------
This is a simple timer and tcp example.

```cpp
#include <ednio/EdNio.h>

using namespace std;
using namespace edft;

// simple timer and tcp example
int main(int argc, char* argv[]) {
  EdNioInit(); // init ednio
  EdTask task;
  EdTimer timer1;
  EdSocket sock;
  task.setOnListener([&](EdMsg &msg) {
    if(msg.msgid == EDM_INIT) {
      printf("task init event, task started...\n");
      // init socket
      sock.setOnListener([&](EdSocket &s, int event) {
        if(event == SOCK_EVENT_CONNECTED) {
          printf("socket connected\n");
        } else if(event == SOCK_EVENT_READ) {
          printf("socket readable\n");
          char buf[1024];
          sock.recv(buf, 1024);
        } else if(event == SOCK_EVENT_DISCONNECTED) {
          printf("socket disconnected\n");
        } 
      });
      sock.connect("127.0.0.1", 22); // make a tcp connection to 127.0.0.1:22 
      
      // timer1 init
      timer1.setOnListener([&](EdTimer &t) {
        printf("timer1 expired\n");
        task.postExit();
      });
      timer1.set(1000); // start timer. After 1 sec, timer expired. 
    } else if(msg.msgid == EDM_CLOSE) {
      printf("task stopped\n");
      // this is the chane to clear all resources.
      timer1.kill();
      sock.close(); // disconnect socket
    }
    return 0;
  });
  task.runMain(); // start event loop
  return 0;
}
```

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






