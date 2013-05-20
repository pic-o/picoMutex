/***********************************************************************************************************************************************************
Original Web Link:
http://blog.pic-o.com/

Personal request:
Drop me an email if you do use this / encounter problems =]
picoCreator (Eugene Cheah Hui Qin | pico.creator{at}gmail.com)

License Type:
BSD 3-Clause (This is both "GPL" and "commercial with acknowledgement" compatible)

Copyright (c) 2013, picoCreator (Eugene Cheah Hui Qin | pico.creator{at}gmail.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

+ Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
+ Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
+ Neither the name of ANY ORGANIZATION(S) AFFLIATED WITH THE CREATOR (picoCreator) nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**********************************************************************************************************************************************************/
#include "x_semaphore.hpp"
using namespace std;

/************************************************************************************
*	Class: x_semaphore
*	Cross-platform semaphore wrapper for both windows and posix style
*
*	See http://docs.oracle.com/cd/E19683-01/806-6867/sync-27385/index.html for a rough idea how to use them
************************************************************************************/

/**
* Function: x_semaphore
* Class constructor, sets the initial value count (default=1)
**/
x_semaphore::x_semaphore(unsigned int initValue) {
	#if defined(_WIN32) //windows style
		os_semaphore = CreateSemaphore(NULL, (long)initValue, (long)initValue, NULL);
	#elif defined(__linux__) || defined(__APPLE__) //posix style
		sem_init(&os_semaphore, 0, initValue);
	#endif
}

/**
* Function: lock
* Lock the semaphore once (decremental), this will block and wait for the semaphore to be "non-negative"
**/
void x_semaphore::lock() {
	#if defined(_WIN32) //windows style
		WaitForSingleObject(os_semaphore, INFINITE);
	#elif defined(__linux__) || defined(__APPLE__) //posix style
		sem_wait(&os_semaphore);
	#endif
}

/**
* Function: unlock
* Unlock the semaphore once (incremental)
**/
void x_semaphore::unlock() {
	#if defined(_WIN32) //windows style
		ReleaseSemaphore(os_semaphore,1,NULL);
	#elif defined(__linux__) || defined(__APPLE__) //posix style
		sem_post(&os_semaphore);
	#endif
}

/**
* Function: x_semaphore
* Class destructor, realeses and destroy the respective os semaphore
**/
x_semaphore::~x_semaphore() {
	lock();
	#if defined(_WIN32) //windows style
		ReleaseSemaphore(os_semaphore, 1, NULL);
	#elif defined(__linux__) || defined(__APPLE__) //posix style
		sem_destroy(&os_semaphore);
	#endif
}