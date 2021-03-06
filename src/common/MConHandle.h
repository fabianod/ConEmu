﻿
/*
Copyright (c) 2009-present Maximus5
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#pragma once

#include "defines.h"
#include "MSection.h"

/* Console Handles */
class MConHandle
{
	private:
		wchar_t   ms_Name[10];
		HANDLE    mh_Handle;
		MSection  mcs_Handle;
		BOOL      mb_OpenFailed;
		DWORD     mn_LastError;
		DWORD     mn_StdMode;
		HANDLE*   mpp_OutBuffer; // Устанавливается при SetConsoleActiveScreenBuffer

		static const int HANDLE_BUFFER_SIZE = RELEASEDEBUGTEST(0x100,0x1000);   // Must be a power of 2
		struct Event {
			DWORD TID;
			HANDLE h;
			enum EventType {
				e_Empty,
				e_GetHandle,
				e_CloseHandle,
				e_CloseHandleStd,
				e_CreateHandle,
				e_CreateHandleStd,
				e_GetHandlePtr,
			} evt;
			DEBUGTEST(DWORD time;)
		};
		Event m_log[HANDLE_BUFFER_SIZE];
		LONG m_logidx;
		void LogHandle(UINT evt, HANDLE h);

	public:
		operator const HANDLE();

	public:
		void Close();
		void SetBufferPtr(HANDLE* ppOutBuffer);

	public:
		MConHandle(LPCWSTR asName);
		~MConHandle();
};
