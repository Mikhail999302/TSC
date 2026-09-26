#ifndef TABLEMANAGER_IMPLEMENTATION_TABLEMANAGER_HPP
#define TABLEMANAGER_IMPLEMENTATION_TABLEMANAGER_HPP
#include "..\Headers\TableManager.h"

template <typename Elem>
Tsc::TableManager::CTableManager<Elem>::CTableManager(const Elem *_data,int _width,int _height):
		mWidth(_width),mHeight(_height),mData(_data,_data+mHeight*mWidth)
{
}

template <typename Elem>
Tsc::TableManager::CTableManager<Elem>::CTableManager(const CTableManager<Elem>& _rhs):
		mWidth(_rhs.mWidth), mHeight(_rhs.mHeight), mData(_rhs.mData.begin(),_rhs.mData.end())
{
}

template <typename Elem>
Tsc::TableManager::CTableManager<Elem>& Tsc::TableManager::CTableManager<Elem>::operator =(const CTableManager<Elem>&_rhs)
{
	if(this!=&_rhs)
	{
		mHeight=_rhs.mHeight;
		mWidth=_rhs.mWidth;
		mData=_rhs.mData;
	}
	return *this;
}

template <typename Elem>
Tsc::TableManager::CTableManager<Elem>* Tsc::TableManager::CTableManager<Elem>::CreateJoinCells(const int*_arr,bool _promptSquare/*=false*/)
{
	int matSize=mWidth*mHeight;
	std::vector<int> arr(_arr,_arr+matSize);
	int maxIdx=*std::max_element(arr.begin(),arr.end());
	std::vector<Elem> data(maxIdx+1);
	std::vector<int> isFirst(maxIdx+1);
	for(int i=0;i<matSize;++i)
	{
		if(!isFirst[arr[i]])
		{
			data[arr[i]]=mData[i];
			isFirst[arr[i]]=true;
		}
		else
		{
			data[arr[i]]+=mData[i];
		}
	}
	int width=maxIdx+1;
	int height=1;
	if(width!=height&&_promptSquare)
		tryCreateSquare(width,height);
	CTableManager *rezTable=new CTableManager(&data.front(),width,height);
	return rezTable;
}

template <typename Elem>
Tsc::TableManager::CTableManager<Elem>* Tsc::TableManager::CTableManager<Elem>::CreateJoinLines(const int*_arr)const
 {
	std::vector<int> arr(_arr,_arr+mHeight);
	int maxIdx=*std::max_element(arr.begin(),arr.end());
	std::vector<Elem> data((maxIdx+1)*mWidth);
	std::vector<int> isFirst((maxIdx+1)*mWidth);
	for(int i=0;i<mHeight;++i)
	{
		for(int j=0;j<mWidth;++j)
		{
			if(!isFirst[_arr[i]*mWidth+j])
			{
				data[_arr[i]*mWidth+j]=mData[i*mWidth+j];
				isFirst[_arr[i]*mWidth+j]=true;
			}
			else
				data[_arr[i]*mWidth+j]+=mData[i*mWidth+j];
		}
	}
	Elem* rez=new Elem[data.size()];
	for(int i=0;i<(int)data.size();++i)
	{
		rez[i]=data[i];
	}
	CTableManager<Elem> *rezTable=new CTableManager<Elem>(rez,mWidth,maxIdx+1);
	delete[]rez;
	return rezTable;
}

template <typename Elem>
Tsc::TableManager::CTableManager<Elem>* Tsc::TableManager::CTableManager<Elem>::CreateJoinColumns(const int*_arr)const
{
	std::vector<int> arr(_arr,_arr+mWidth);
	int maxIdx=*std::max_element(arr.begin(),arr.end());
	std::vector<Elem> data((maxIdx+1)*mHeight);
	std::vector<int> isFirst((maxIdx+1)*mHeight);
	for(int j=0;j<mHeight;++j)
	{
		//int maxIdx=0;
		for(int i=0;i<mWidth;++i)
		{
			if(!isFirst[j*(maxIdx+1)+_arr[i]])
			{
				data[j*(maxIdx+1)+_arr[i]]=mData[j*mWidth+i];
				isFirst[j*(maxIdx+1)+_arr[i]]=true;
			}
			else
			{
				data[j*(maxIdx+1)+_arr[i]]+=mData[j*mWidth+i];
			}
		}
	}
	Elem* rez=new Elem[data.size()];
	for(int i=0;i<(int)data.size();++i)
	{
		rez[i]=data[i];
	}
	CTableManager<Elem> *rezTable=new CTableManager<Elem>(rez,maxIdx+1,mHeight);
	delete[]rez;
	return rezTable;
}

template <typename Elem>
Tsc::TableManager::CTableManager<Elem>* Tsc::TableManager::CTableManager<Elem>::CreateJoinCrosses(const int*_arr,bool _promptSquare/*=true*/)
{
	if(mHeight!=mWidth&&_promptSquare)
	{
		
		if(!tryMakeSquare()) throw std::logic_error("None quadratic Matrix");
	}
	CTableManager<Elem> *temp=CreateJoinColumns(_arr);
	CTableManager<Elem> *rez=temp->CreateJoinLines(_arr);
	delete temp;
	return rez;
}

template <typename Elem>
bool Tsc::TableManager::CTableManager<Elem>::tryMakeSquare()
{
	if(sqrt((double)mWidth*mHeight)==floor(sqrt((double)mWidth*mHeight)))
		{
			mWidth=(int)sqrt((double)mWidth*mHeight);
			mHeight=mWidth;
			return true;
		}
	return false;
}

template <typename Elem>
bool Tsc::TableManager::CTableManager<Elem>::tryCreateSquare(int &oWidth,int &oHeight)
{
	if(sqrt((double)oWidth*oHeight)==floor(sqrt((double)oWidth*oHeight)))
		{
			oWidth=(int)sqrt((double)oWidth*oHeight);
			oHeight=oWidth;
			return true;
		}
	return false;
}

template <typename Elem>
Tsc::TableManager::CTableManager<Elem>::~CTableManager(void)
{
}

#endif //TABLEMANAGER_IMPLEMENTATION_TABLEMANAGER_HPP