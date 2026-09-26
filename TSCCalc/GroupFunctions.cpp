#include "StdAfx.h"
//#include "GroupFunctions.h"
////Do no groupping 0..size*size-1
//TSCCALC_DLLENTRY GroupedTSC::uint* GroupedTSC::NullGrouping(int size,int &oSize)
//{
//	std::vector<uint> rez(size*size);
//	for(std::vector<uint>::iterator p=rez.begin();p!=rez.end();++p)*p=(uint)(p-rez.begin());
//	oSize=(int) rez.size();
//	return &rez.front();
//}
//
////Do usual groupping 0..n+2
//TSCCALC_DLLENTRY GroupedTSC::uint* GroupedTSC::UsualGroupping(int size,int &oSize)
//{
//	std::vector<uint> rez(size*size);
//	for(int i=0;i<size;++i)
//	{
//		for(int j=0;j<size;++j)
//		{
//			if(i==j)
//				rez[i*size+j]=i;
//			else	
//			{
//				if(!j)
//					rez[i*size+j]=size;
//				else	
//				{
//					if(!i)
//						rez[i*size+j]=size+1;
//					else rez[i*size+j]=size+2;
//				}
//			}
//		}
//	}
//	oSize=(int) rez.size();
//	return &rez.front();
//}
