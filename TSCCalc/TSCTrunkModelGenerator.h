//Generation of symbolic representation of a TSC model without adverticement for current sizes.
#pragma once
#include <C:\diploma\2012\TSC\Libs\TableManager\Entry.h>
#include <C:\diploma\2012\TSC\Libs/Formula\Entry.h>
#include "stdafx.h"
#include "Typedefs.h"

namespace GroupedTSC
{
	class TSCCALC_DLLENTRY CTSCTrunkModelGenerator :
		public Tsc::TableManager::IDataLoader<Tsc::Formula::CMathExpString<MathString> >
	{
		int mWidth;
		int mHeight;
	public:
		CTSCTrunkModelGenerator(int _width, int _height);
		virtual ~CTSCTrunkModelGenerator(void);
		virtual TModelManager* CreateTableManager();
		virtual /*implement*/ IDataLoader<Tsc::Formula::CMathExpString<MathString> >* Clone()
		{
			return new CTSCTrunkModelGenerator(mWidth, mHeight);
		}
	};
}//namespace GroupedTSC