// MathCalcDebug.cpp: debug stuff
//
//////////////////////////////////////////////////////////////////////

//#include "../Headers/stdafx.h"
#include "MathCalcMisc.h"
#include "MathCalcDebug.h"
#include <tchar.h>

//#include <crtdbg.h>

#ifdef _DEBUG

// ifdef then show notify messages during debug (need with _DEBUG only)
#define TRACK_OBJECTS

//////////////////////////////////////////////////////////////////////

#ifdef TRACK_OBJECTS

namespace {
	int ObjectsCounter = 0;
	inline void OutputMessage( MathCalc::LPCTSTR msg ) 
	{
		//_RPT0( _CRT_WARN, msg );
	}
}

void IncreaseObjectsCounter() { 
	if (!ObjectsCounter)
		OutputMessage( _T("First 'MathCalc' object was created.\n") );
	++ObjectsCounter; 
}
void DecreaseObjectsCounter() {
	if (!--ObjectsCounter) 
		OutputMessage( _T("All 'MathCalc' objects were successfully deleted!\n") );
	if(ObjectsCounter<0)
		OutputMessage( _T("Some 'MathCalc' objects were successfully deleted TWICE!\n") );
}

#else // !TRACK_OBJECTS

void IncreaseObjectsCounter() {}
void DecreaseObjectsCounter() {}

#endif // TRACK_OBJECTS

#endif // _DEBUG
