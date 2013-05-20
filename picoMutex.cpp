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
#include "picoMutex.hpp"
#include <exception>
#include <stdexcept>

#include <thread>
#include <chrono>

using namespace std;

/************************************************************************************
*	Class: picoMutex
*	Light weight mutex. Used mainly for variables that will be rapidly locked / unlocked, with low "hogging" time.
*	This uses "microsleep(100)", for spinwaiting. And hence may be a "waste" of CPU cycles for certain setups
*
*	The system is based on atomic adds, to establish the various locks
*
*	References:
*   - http://en.cppreference.com/w/cpp/atomic/atomic
*	- http://preshing.com/20120226/roll-your-own-lightweight-mutex
*	- http://msdn.microsoft.com/en-sg/library/2ddez55b(v=vs.80).aspx
*	- http://gcc.gnu.org/onlinedocs/gcc-4.4.3/gcc/Atomic-Builtins.html#Atomic-Builtins
************************************************************************************/

/************************************************************************************
* Function: picoMutex
* Class constructor, validates that x_Interlocked in debug mode
************************************************************************************/
picoMutex::picoMutex() {
	counter = 0x00;
}

/************************************************************************************
* Function: is_locked
* Checks and returns true if its already locked
************************************************************************************/
bool picoMutex::is_locked() {
	return (counter != 0x00);
}

/************************************************************************************
* Function: lock
* Attempts and block to achieve lock
************************************************************************************/
void picoMutex::lock() {
	//if( !try_lock() ) {
	//	this_thread::yield();
		int_fast8_t tryInt = 0x00;
		while( counter.compare_exchange_weak(tryInt,0x01) == 0x00 ) {
	//		//sleep(1);
	//		//this_thread::yield();
			std::this_thread::sleep_for( std::chrono::microseconds(100) );
			tryInt = 0x00;
		}
	// }
	return;
}

/************************************************************************************
* Function: try_lock
* Attempts to get a non-blocking lock
************************************************************************************/
bool picoMutex::try_lock() {
	//validates that the counter is 0, before attempting to atomically increase its value to 1
	int_fast8_t tryInt = 0x00;
	return (counter == tryInt && counter.compare_exchange_strong(tryInt,0x01) == 0x00);
	//return false;
}

/************************************************************************************
* Function: unlock
* Unlock the mutex
************************************************************************************/
void picoMutex::unlock() {
	int_fast8_t tryInt = 0x01;
	if( counter.compare_exchange_strong(tryInt,0x00) == 0x01 ) {
		return;
	} else {
		throw new underflow_error(picoMutex::_exceptionUnlockUnderflow);
	}
}

/************************************************************************************
* Variable: _exceptionUnlockUnderflow
* [static] : Error output given when an attempt to unlock a previously non-locked picoMutex is attempted
************************************************************************************/
char* picoMutex::_exceptionUnlockUnderflow = (char*)"unlock attempted, on non-locked picoMutex";

/************************************************************************************
* Class: picoMutex_lockGuard
* lock guard varient for picoMutex
************************************************************************************/

/************************************************************************************
* Function: picoMutex_lockGuard
* Lock guard constructor accepting a picoMutex pointer
************************************************************************************/
picoMutex_lockGuard::picoMutex_lockGuard( picoMutex* inMutex ) {
	if(!inMutex) {
		throw new runtime_error(picoMutex_lockGuard::_exceptionNullPicoMutex);
	}

	toGuard = inMutex;
	toGuard -> lock();
}

picoMutex_lockGuard::~picoMutex_lockGuard() {
	toGuard -> unlock();
}

/************************************************************************************
* Variable: _exceptionNullPicoMutex
* [static] : Error output given when the argument passed to lockGuard is NULL
************************************************************************************/
char* picoMutex_lockGuard::_exceptionNullPicoMutex = (char*)"lockGuard requires valid picoMutex (non null)";