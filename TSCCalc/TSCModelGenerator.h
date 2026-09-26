//Generation of symbolic representation of a TSC model for current sizes.
#pragma once
#include "stdafx.h"
#include "Typedefs.h"
#include <C:\diploma\2012\TSC\Libs/TableManager\Entry.h>

namespace GroupedTSC
{
	//базовый класс генератор модели
class TSCCALC_DLLENTRY CTSCModelGenerator :
	public Tsc::TableManager::IDataLoader<Tsc::Formula::CMathExpString<MathString> >
{
protected:
	int mWidth;
	int mHeight;
public:
	CTSCModelGenerator(int _width, int _height);
	virtual ~CTSCModelGenerator(void);
	virtual TModelManager* CreateTableManager() {return 0;};
	virtual IDataLoader<Tsc::Formula::CMathExpString<MathString> >* Clone()
	{
		return new CTSCModelGenerator(mWidth,mHeight);
	}
};

//*************************************
//****   class CTSCModel_2Pos   *******
//модель с двумя положительными рекламами
//*************************************
class TSCCALC_DLLENTRY CTSCModelGenerator_2Pos: public CTSCModelGenerator
{
public:
	CTSCModelGenerator_2Pos(int _width, int _height): CTSCModelGenerator(_width, _height){};
	virtual ~CTSCModelGenerator_2Pos(void){};

	virtual TModelManager* CreateTableManager();
};


//*************************************
//****   class CTSCModel_1Pos   *******
//модель с одной положительной рекламамой
//*************************************
class TSCCALC_DLLENTRY CTSCModelGenerator_1Pos: public CTSCModelGenerator
{
public:
	CTSCModelGenerator_1Pos(int _width, int _height): CTSCModelGenerator(_width, _height){};
	virtual ~CTSCModelGenerator_1Pos(void){};

	virtual TModelManager* CreateTableManager();
};

//edited 11.04
//*************************************
//****   class CTSCModel_1Pos   *******
//модель с одной положительной рекламамой
// для новой модели
//*************************************
class TSCCALC_DLLENTRY CTSCModelGenerator_1Pos_mine : public CTSCModelGenerator_1Pos
{
public:
	CTSCModelGenerator_1Pos_mine(int _width, int _height) : CTSCModelGenerator_1Pos(_width, _height) {};
	virtual ~CTSCModelGenerator_1Pos_mine(void) {};

	virtual TModelManager* CreateTableManager() override;
};


//**********************************************
//****   class CTSCModelGenerator_1Neg   *******
//модель с одной отрицательной рекламамой*******
//**********************************************
class TSCCALC_DLLENTRY CTSCModelGenerator_1Neg: public CTSCModelGenerator
{
public:
	CTSCModelGenerator_1Neg(int _width, int _height): CTSCModelGenerator(_width, _height){};
	virtual ~CTSCModelGenerator_1Neg(void){};

	virtual TModelManager* CreateTableManager();
};


//*************************************************
//****   class CTSCModelGenerator_2NegPos   *******
//модель с двумя рекламами: "-" и "+" *************
//*************************************************
class TSCCALC_DLLENTRY CTSCModelGenerator_2NegPos: public CTSCModelGenerator
{
public:
	CTSCModelGenerator_2NegPos(int _width, int _height): CTSCModelGenerator(_width, _height){};
	virtual ~CTSCModelGenerator_2NegPos(void){};

	virtual TModelManager* CreateTableManager();
};
}//namespace GroupedTSC