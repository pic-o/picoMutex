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
#pragma once
using namespace std;

//Cross platform (X) semaphore object
#if defined(_WIN32) //windows style semaphore object http://msdn.microsoft.com/en-us/library/windows/desktop/ms686946(v=vs.85).aspx
	#include <windows.h>
#elif defined(__linux__) //POSIX style semaphores http://www.csc.villanova.edu/~mdamian/threads/posixsem.html
	#include <semaphore.h>
#elif defined(__APPLE__) //Assuming POSIX style
	#include <semaphore.h>
	#error MAC support is currently UNTESTED. comment this line if this is intentional
#else
	#error Compiling for non-supported platform  
#endif

class x_semaphore {
  private:
	//Cross platform (X) semaphore object
	#if defined(_WIN32) //windows style
		HANDLE os_semaphore;
	#elif defined(__linux__) || defined(__APPLE__) //posix style
		sem_t os_semaphore;
	#endif
	
  public:
	x_semaphore(unsigned int initValue = 1);
	~x_semaphore();

	void lock();
	void unlock();
};