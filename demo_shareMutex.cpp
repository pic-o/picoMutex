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

///////////////////////////////////////////////////////////////////////////////////////
// The following is a demo file, showing the usage of shareMutex //
///////////////////////////////////////////////////////////////////////////////////////
#define THIS_IS_MAIN_FILE

#include <iostream> //std::cout
#include <thread> //std::thread
#include <time.h> //std::clock
#include <vector> //std::vector
#include <atomic> //std::atomic
#include <exception>
#include <stdexcept>

#include "shareMutex.hpp"
//#include "x_cacheLineSize.hpp"

using namespace std;

clock_t oValue;
clock_t gValue;
shareMutex gMutex;

void write_thrd() {
	cout << "Pre-exclusive lock, shared lock count : " << gMutex.debug_shareLockCount() << "\n";
	gMutex.exclusiveLock();
	cout << "Post-exclusive lock, exclusive lock count : " << gMutex.debug_exclusiveLockCount() << "\n";
	gValue = clock();
	cout << "New value write : " <<  ((gValue - oValue)/((double) CLOCKS_PER_SEC)) << "\n";
	gMutex.exclusiveUnlock();
}

void reader_thrd(int thrdNum) {
	clock_t prvValue = oValue;

	gMutex.sharedLock();
	cout << "Reader thread[" << thrdNum << "] starting (acquired share lock) : " << ((gValue - oValue)/((double) CLOCKS_PER_SEC)) << "\n";

	while(true) {
		if( gMutex.reload_shareLock() ) { //share lock reloaded? A write lock happened
			
			if( gValue == 0 ) { //reset / end
				cout << "Reader thread[" << thrdNum << "] Detecing value changed to : " << 0 << "(end)\n";
				break;
			}

			if( gValue <= prvValue ) { //value did not change : possible lock error
				throw exception("Value didnt change as expected");
			} else {
				cout << "Reader thread[" << thrdNum << "] Detecing value changed to : " << ((gValue - oValue)/((double) CLOCKS_PER_SEC)) << "\n";
			}
			prvValue = gValue;
		} else {
			if( gValue == 0 ) { //reset / end
				cout << "Reader thread[" << thrdNum << "] Detecing value changed to : " << 0 << "(end)\n";
				break;
			}
		}

		if( clock() < gValue ) {
			throw exception("gValue more then current time : async error?");
		}

		std::this_thread::sleep_for( std::chrono::microseconds(100) ); //sleep wait
	}
	cout << "Unlocking reader thread[" << thrdNum << "]\n";
	gMutex.sharedUnlock();
	return;
}

#define simpleCoutException(msg) cout << msg << "\n"; throw exception(msg); return 1;

int main(int argc, char* argv[]) {
	cout << "shareMutex demo example \n";
	
	int threadMax = 4;
	int writeTries = 4;
	int a;

	cout << "---------------------------------------------------------\n";
	cout << "Note: Current thread_id / size -> " << this_thread::get_id() << " / " << sizeof( thread::id ) << "\n";
	//cout << "Note: Cache line size : " << x_cacheLineSize() << "\n";
	cout << "Note: Size of 'shareMutex' : " << sizeof( shareMutex ) << "\n";
	cout << "Note: Assuming maximum read threads of : " << threadMax << "\n";
	
	cout << "---------------------------------------------------------\n";
	cout << "ATOMIC lock notes : 0 Never lock free, 1 Maybe lock free, 2 always lock free \n";
	cout << "ATOMIC_CHAR_LOCK_FREE : " << ATOMIC_CHAR_LOCK_FREE << "\n";
	cout << "ATOMIC_CHAR16_T_LOCK_FREE : " << ATOMIC_CHAR16_T_LOCK_FREE << "\n";
	cout << "ATOMIC_CHAR32_T_LOCK_FREE : " << ATOMIC_CHAR32_T_LOCK_FREE << "\n";
	cout << "ATOMIC_WCHAR_T_LOCK_FREE : " << ATOMIC_WCHAR_T_LOCK_FREE << "\n";
	cout << "ATOMIC_SHORT_LOCK_FREE : " << ATOMIC_SHORT_LOCK_FREE << "\n";
	cout << "ATOMIC_INT_LOCK_FREE : " << ATOMIC_INT_LOCK_FREE << "\n";
	cout << "ATOMIC_LONG_LOCK_FREE : " << ATOMIC_LONG_LOCK_FREE << "\n";
	cout << "ATOMIC_LLONG_LOCK_FREE : " << ATOMIC_LLONG_LOCK_FREE << "\n";
	cout << "ATOMIC_POINTER_LOCK_FREE : " << ATOMIC_POINTER_LOCK_FREE << "\n";
	cout << "---------------------------------------------------------\n";

	oValue = clock();
	gValue = oValue;
	
	vector<thread*> threadVector;
	threadVector.resize(threadMax);

	cout << "Simple testing (single thread) lock capabilities\n";
	//Nothing lock
	if( gMutex.has_anyLock() || gMutex.has_exclusiveLock() || gMutex.has_sharedLock() ) {
		simpleCoutException("has_XLock() failure when there is NO locks");
	}

	//Read lock try (once)
	if( gMutex.try_sharedLock() == false ) {
		simpleCoutException("try_sharedLock() failure (from NO locks)");
	}
	if( gMutex.debug_shareLockCount() != 1 ) {
		simpleCoutException("debug_shareLockCount() != 1 (from single shareLock)");
	}
	if( gMutex.try_exclusiveLock() != false ) {
		simpleCoutException("try_exclusiveLock() succes (expected failure, from single shareLock)");
	}
	gMutex.sharedUnlock();

	//Ending Nothing lock
	if( gMutex.has_anyLock() || gMutex.has_exclusiveLock() || gMutex.has_sharedLock() ) {
		simpleCoutException("has_XLock() failure when there is NO locks");
	}
	cout << "Simple testing (single thread) ends\n";
	cout << "---------------------------------------------------------\n";

	for( a = 0; a < threadMax; ++a ) {
		cout << "Generating read thread " << a << "\n";
		threadVector[a] = new thread(reader_thrd,a);
	}
	
	std::this_thread::sleep_for( std::chrono::seconds(2) );

	thread * writeThrd;
	for( a = 0; a < writeTries; ++a ) {
		writeThrd =  new thread(write_thrd);
		writeThrd -> join();
		delete(writeThrd);
		std::this_thread::sleep_for( std::chrono::seconds(2) );
	}
	
	gMutex.exclusiveLock();
	gValue = 0;
	cout << "New value write : " << gValue << "(end) \n";
	gMutex.exclusiveUnlock();
	
	for( a = 0; a < threadMax; ++a ) {
		cout << "Joining thread " << a << "\n";
		threadVector[a] -> join();
	}
	cout << "Join complete\n";

	return 0;
}