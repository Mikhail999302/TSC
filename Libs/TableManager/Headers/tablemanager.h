#ifndef TABLEMANAGER_HEADERS_TABLEMANAGER_H
#define TABLEMANAGER_HEADERS_TABLEMANAGER_H
#include <vector>
#include <iostream>
#include <algorithm>
#include <math.h>
#include <stdexcept>

namespace Tsc
{
namespace TableManager
{
//2DO:
//Elem should implement operator+()
template<class Elem>
class CTableManager
{
	//[firstLine][SecondLine]...[LastLine]
	int mWidth,mHeight;
	std::vector<Elem> mData;

private:
	CTableManager(void){/*fake*/}

public://ctors,dtor
	CTableManager(const Elem* _data,int _width,int _height);
	CTableManager(const CTableManager& _rhs);
	CTableManager& operator=(const CTableManager& _rhs);
	~CTableManager(void);

public://managing
	CTableManager* CreateJoinLines(const int * _arr)const;//_arr not shorter then mHeight(not checked)
	CTableManager* CreateJoinColumns(const int *_arr)const;//_arr not shorter then mWidth
	CTableManager* CreateJoinCells(const int *_arr,bool _promptSquare=false);//_arr not shorter then mHeight*mWidth
	CTableManager* CreateJoinCrosses(const int *_arr,bool _promptSquare=true);//the table should be square and array not shorter then mWidth

public://out
	int GetWidth()const{return mWidth;}
	int GetHeight()const{return mHeight;}
	const Elem& operator[](int i)const{return mData[i];}
	const Elem& At(int i)const {if(i<mWidth*mHeight&&i>=0)return mData[i];else throw std::logic_error("Not in Array");}

private:
	bool IsSquare()const{return mWidth==mHeight;}
	bool Is1Dim()const{return mHeight==1;}
	bool tryMakeSquare();
	bool tryCreateSquare(int &oWidth,int &oHeight);
};
}//namespace TableManager
}//namespace Tsc
#include "..\Implementation\TableManager.hpp"
#endif //TABLEMANAGER_HEADERS_TABLEMANAGER_H