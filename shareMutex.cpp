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
#include "shareMutex.hpp" 
#include <exception> // std::exception
#include <stdexcept>

#include <thread>
#include <chrono>

using namespace std;

#define _shareMutex_logicDebug
#define _shareMutex_sleepRange chrono::microseconds(100)

/************************************************************************************
*	Class: shareMutex
*	Allows both; Lock-Free pure shared lock acquisition via atomic variables;
*   and sleep-wait locking when waiting for an exclusive lock (to acquire or realese)
*
*
*   The mutex has been designed for high shared locks, low exclusive locks contention scenerios.
*   And has the downside of the possibility of "eternal" shared lock waiting, with continous exclusive locking.
*   (exclusive locks, takes priority over shared locks acquisition)
************************************************************************************/

//private : various refrence bits used repeatingly in the mutex
uint_least16_t shareMutex::boundaryBits           = 0x1000; // 0001 0000 0000 0000
uint_least16_t shareMutex::boundaryBits_overflow  = 0x2000; // 0010 0000 0000 0000
uint_least16_t shareMutex::boundaryRange          = 0x3000; // 0011 0000 0000 0000
uint_least16_t shareMutex::exclusiveRange         = 0xC000; // 1100 0000 0000 0000
uint_least16_t shareMutex::sharedRange            = 0x0FFF; // 0000 1111 1111 1111
uint_least16_t shareMutex::loneExclusiveLock      = 0x5000; // 0101 0000 0000 0000
uint_least16_t shareMutex::exclusiveLock0         = 0x4000; // 0100 0000 0000 0000
uint_least16_t shareMutex::exclusiveLock1         = 0x8000; // 1000 0000 0000 0000

//private : helper function, that checks for overflow/underflow error of shared locks, and outputs the relative error
void shareMutex::checkForOverflow(uint_least16_t fetchedValue) {
	if( (fetchedValue & shareMutex::boundaryRange) != shareMutex::boundaryBits ) { //Check for overflow
		if( (fetchedValue & shareMutex::boundaryBits_overflow) != 0 ) { //overflowed bits
			throw overflow_error( shareMutex::_exception_sharedLockOverflow );
		}

		if( (fetchedValue & shareMutex::boundaryBits) == 0 ) { //mising boundary bit
			throw underflow_error( shareMutex::_exception_sharedLockUnderflow );
		}
	}
}

bool shareMutex::has_exclusiveLock(uint_least16_t fetchedValue) {
	return ((fetchedValue & shareMutex::exclusiveRange) != 0);
}

bool shareMutex::has_sharedLock(uint_least16_t fetchedValue) {
	return ((fetchedValue & shareMutex::sharedRange) != 0);
}

bool shareMutex::has_anyLock(uint_least16_t fetchedValue) {
	return (fetchedValue != shareMutex::boundaryBits);
}

/**
* Variable: _exception_sharedLockOverflow/exception_sharedLockUnderflow/exclusiveLockUnderflow
* [Static] : Multiple exception variations, for either too many lock or unlock operations
**/
char* shareMutex::_exception_sharedLockOverflow     = "Maximum shared lock of 1023 reached : Unable to issue new shared lock()";
char* shareMutex::_exception_sharedLockUnderflow    = "Shared unlock while no lock attempted : Possibly due to excess shared unlock()";
char* shareMutex::_exception_exclusiveLockUnderflow = "Exclusive unlock while no lock attempted : Possibly due to excess exclusive unlock()";

/**
* Function: shareMutex
* Class constructor, to setup the shareMutex initial boundary bit
**/
shareMutex::shareMutex() {
	mixLock = shareMutex::boundaryBits; //initial boundary (no locks)
}

/**
* Function: has_exclusiveLock
* Checks for any exclusive locks
**/
bool shareMutex::has_exclusiveLock() {
	return has_exclusiveLock(mixLock);
}

/**
* Function: has_sharedLock
* Checks for any shared locks
**/
bool shareMutex::has_sharedLock() {
	return has_exclusiveLock(mixLock);
}

/**
* Function: has_anyLock
* Checks for any locks
**/
bool shareMutex::has_anyLock() {
	return has_anyLock(mixLock);
}

/**
* Function: try_sharedLock
* Attempts to acquire a sharedLock. Returns false on failure (no waiting)
**/
bool shareMutex::try_sharedLock() {
	if( !has_exclusiveLock() ) { //no write lock
		uint_least16_t fetchedValue = mixLock.fetch_add(1) + 1; //++mixLock
		checkForOverflow(fetchedValue);

		if( has_exclusiveLock(fetchedValue) ) { //Checking for write lock
			//write lock existed : reverting back read lock (assume write lock already issued)
			fetchedValue = mixLock.fetch_sub(1) - 1; //--mixLock
			checkForOverflow(fetchedValue);
		} else { //Success: No write lock + No overflow error
			return true;
		}

	}
	return false;
}

/**
* Function: sharedLock
* Attempts and wait till a shared lock is acquired
**/
void shareMutex::sharedLock() {
	while(try_sharedLock() == false) {
		//failed lock?
		this_thread::sleep_for( _shareMutex_sleepRange );
	}
}

/**
* Function: sharedUnlock
* Unlocks one shared lock.
**/
void shareMutex::sharedUnlock() {
	uint_least16_t fetchedValue = mixLock.fetch_sub(1) - 1; //reverting back read lock, saving resultant (stored) value
	checkForOverflow(fetchedValue);
}

/**
* Function: try_exclusiveLock
* Attempts to acquire a exclusiveLock. Returns false on failure (no waiting)
**/
bool shareMutex::try_exclusiveLock() {
	uint_least16_t noLocks = shareMutex::boundaryBits;
	return mixLock.compare_exchange_strong(noLocks, shareMutex::loneExclusiveLock);
}

/**
* Function: exclusiveLock
* Attempts and wait till an exclusive lock is acquired
**/
void shareMutex::exclusiveLock() {
	uint_least16_t fetchedValue = shareMutex::boundaryBits;

	//Direct exclusive lock achieved (no existing shared/exclusive lock)
	if(mixLock.compare_exchange_strong(fetchedValue, shareMutex::loneExclusiveLock)) {
		return; 
	}

	//There is pre-existing locks
	while(true) { //loop to acquire outer lock
		if( (fetchedValue & shareMutex::exclusiveLock1) == 0 ) { //no outer write lock
			fetchedValue = mixLock.fetch_or( shareMutex::exclusiveLock1 );

			if( (fetchedValue & shareMutex::exclusiveLock1) == 0 ) { //outer lock acquried succesfully
				break; //break out of outer exclusive lock acquire loop
			}
		}

		this_thread::sleep_for( _shareMutex_sleepRange ); //sleep
		fetchedValue = mixLock; //get possibly new value (validate at start of loop)
	}

	//Wait and acquire inner exclusive lock
	while(true) {
		if( (fetchedValue & shareMutex::exclusiveLock0) == 0 ) { //no inner lock
			fetchedValue = mixLock.fetch_xor( shareMutex::exclusiveRange ); //flip the exclusive locks

			//To comment out, after testing proof this NEVER happens (im sure logically)
			if( (fetchedValue & shareMutex::exclusiveRange) != shareMutex::exclusiveLock1 ) {
				throw exception("Exclusive Lock design error");
			}

			break;
		}

		this_thread::sleep_for( _shareMutex_sleepRange ); //sleep
		fetchedValue = mixLock; //get possibly new value (validate at start of loop)
	}

	while(true) {
		if( !has_sharedLock(fetchedValue) ) { //no shared lock
			return;
		}
		this_thread::sleep_for( _shareMutex_sleepRange ); //sleep
		fetchedValue = mixLock; //get possibly new value (validate at start of loop)
	}
}

/**
* Function: exclusiveUnlock
* Unlocks the current write lock
**/
void shareMutex::exclusiveUnlock() {
	uint_least16_t fetchedValue = mixLock.fetch_xor( shareMutex::exclusiveLock0 );

	if( (fetchedValue & shareMutex::exclusiveLock0) != shareMutex::exclusiveLock0 ) {
		throw underflow_error( shareMutex::_exception_exclusiveLockUnderflow );
	}
}

/**
* Function: reload_shareLock
* Realeses a single read lock, if a write lock was detected : And wait to acquire a new read lock
*
* Return true : if read lock was required; false, if no write lock was found
**/
bool shareMutex::reload_shareLock() {
	if( has_exclusiveLock() ) {
		sharedUnlock();
		sharedLock();
	}
	return false;
}

/**
* Function: debug_shareLockCount
* Gets and return the current shareLock count. Note that this number is not "reliable" 
* (as any output would not be "live"). However it is still useful, in debugging during crashes / stalls
* (detecting phantom locks for example)
**/
uint_least16_t shareMutex::debug_shareLockCount() {
	return (mixLock & shareMutex::sharedRange);
}

/**
* Function: debug_exclusiveLockCount
* Gets and return the current exclusiveLock count. Note that this number is not "reliable" 
* (as any output would not be "live"). However it is still useful, in debugging during crashes / stalls
* (detecting phantom locks for example)
**/
uint_least16_t shareMutex::debug_exclusiveLockCount() {
	uint_least16_t exZone = (mixLock & shareMutex::exclusiveRange);

	return (  
		(((exZone & exclusiveLock0) == exclusiveLock0)? 1 : 0) + 
		(((exZone & exclusiveLock1) == exclusiveLock1)? 1 : 0)  
	);
}