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
#include <stdint.h>
#include <atomic>

using namespace std;

class shareMutex {
  private:
	//First 2 bit : Write lock
	//Next  2 bit : Boundary bits (detect overflow)
	//Last 12 bit : Read lock counting, up to 1023
	atomic<uint_least16_t> mixLock;
	static uint_least16_t boundaryBits;
	static uint_least16_t boundaryBits_overflow;
	static uint_least16_t boundaryRange;
	static uint_least16_t exclusiveRange;
	static uint_least16_t sharedRange;
	static uint_least16_t loneExclusiveLock;
	static uint_least16_t exclusiveLock0;
	static uint_least16_t exclusiveLock1;

	inline void checkForOverflow(uint_least16_t fetchedValue);
	
	inline bool has_sharedLock(uint_least16_t fetchedValue); //
	inline bool has_exclusiveLock(uint_least16_t fetchedValue); //
	inline bool has_anyLock(uint_least16_t fetchedValue); //

  public:
	  
	static char* _exception_sharedLockOverflow;
	static char* _exception_sharedLockUnderflow;
	static char* _exception_exclusiveLockUnderflow;
	
	shareMutex();
	
	bool has_exclusiveLock(); //
	bool has_sharedLock(); //
	bool has_anyLock(); //
	
	bool try_sharedLock(); //
	void sharedLock(); //
	void sharedUnlock(); //
	
	bool try_exclusiveLock(); //
	void exclusiveLock(); //
	void exclusiveUnlock(); //
	
	bool reload_shareLock(); //

	uint_least16_t debug_shareLockCount();
	uint_least16_t debug_exclusiveLockCount();
};