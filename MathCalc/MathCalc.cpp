// MathCalc.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

/*#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			//_CrtSetDbgFlag( _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF );
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

/*
void* operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	return _malloc_dbg(nSize, _NORMAL_BLOCK, lpszFileName, nLine);
}

#if _MSC_VER >= 1200
void operator delete(void* pData, LPCSTR, int)
{
	::operator delete(pData);
}
#endif
*/