#ifndef MATHCALCDEBUG_H
#define MATHCALCDEBUG_H

//////////////////////////////////////////////////////////////////////
// Debug stuff

#ifdef _DEBUG 
	void IncreaseObjectsCounter();
	void DecreaseObjectsCounter();

#else // ifndef _DEBUG

	#define IncreaseObjectsCounter() ((void)0)
	#define DecreaseObjectsCounter() ((void)0)
#endif
#endif //MATHCALCDEBUG_H
