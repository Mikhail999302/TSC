#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <algorithm>

#pragma warning(disable : 4290)

#ifdef USE_SOURCE
#define TSCDUMP_DLLENTRY
#define TSCDUMP_DLLENTRYTEMPLATE 
#else
#ifdef DUMPER_EXPORTS
#define TSCDUMP_DLLENTRY __declspec(dllexport)
#define TSCDUMP_DLLENTRYTEMPLATE 
#else
#define TSCDUMP_DLLENTRYTEMPLATE extern
#define TSCDUMP_DLLENTRY __declspec(dllimport)
#endif
#endif

namespace Dumper
{
	class CDump;
	#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
	TSCDUMP_DLLENTRYTEMPLATE template class TSCDUMP_DLLENTRY std::auto_ptr< CDump >;
	TSCDUMP_DLLENTRYTEMPLATE template class TSCDUMP_DLLENTRY std::allocator< std::string >;
	TSCDUMP_DLLENTRYTEMPLATE template class TSCDUMP_DLLENTRY std::vector< std::string >;
	#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation

	//Allow to write every simple type to dump file.
	//Should be initialized by
	//EX:
	//Dumper::CDump::InitDump(<filename>,<stringSize>);

	//To filter elements add filters by AddFilterClass method
	//Filters in use
	//
	//DiscreteModelInit
	//Exception
	//PLevel
	//SimpleEsti
	//Estimates
	//Groupping
	//Formal
	//Result
	//Inverse
//  [9/10/2007 Sergey]
//	Desc: Add filtering
class TSCDUMP_DLLENTRY CDump
{
	std::string mFileName;
	std::ofstream mOut;
	int mIntWidth;
	int mDoubleWidth;
	std::vector<std::string> mFilterClasses;

protected:
	CDump(const char * fileName,int length,int _intWidth=4,int _doubleWidth=12);
	static std::auto_ptr<CDump> dump;

public:
	virtual ~CDump(void);
	//dump string
	void Dump(const char* _str, int _len,const char* _filterClass="",int _fcLen=1);
	//dump std::string
	void Dump(const std::string& _str, const std::string& _filterClass="");
	//dump int
	void Dump(int _value,const char* _filterClass="",int _fcLen=1);
	//dump double
	void Dump(double _value,const char* _filterClass="",int _fcLen=1);

	//dump vector
	template<typename T> void Dump(T *_vector,int _len,int _strLen,const char* _filterClass="",int _fcLen=1)
	{
		if(checkFilterClass(_filterClass,_fcLen))
		{
			for(int i=0;i<_len;++i)
			{
				mOut.width(_strLen);
				mOut<<_vector[i];
			}
			mOut<<std::endl;
			mOut.flush();
		}
	}
	void NewLine(const char* _filterClass="",int _fcLen=1)
		{
			if(_fcLen==1||checkFilterClass(_filterClass,_fcLen))
				mOut<<std::endl;
		}
	int parmIntWidth(int width=0){if(width>0)mIntWidth=width;return mIntWidth;}
	int parmDoubleWidth(int width=0){if(width>0)mDoubleWidth=width;return mDoubleWidth;}

	//by lines [first line][second line]...[last line]
	template<typename T> void Dump(T*_matrix, int _width,int _height,int _strLen,const char* _filterClass="",int _fcLen=1)
	{
		if(checkFilterClass(_filterClass,_fcLen))
		{
			for(int j=0;j<_height;++j)
			{
				for(int i=0;i<_width;++i)
				{
					mOut.width(_strLen);
					mOut<<_matrix[i+_width*j];
				}
				mOut<<std::endl;
			}
			mOut.flush();
		}
	}

//-->Serge
//Date: 10.09.07
//Desc: Add filter class name, if class not specified, dump is done anyway
	//Add filter class name to vector
	void AddFilterClass(const char * _filterClassName,int _len);
	void DeleteFilterClass(const char * _filterClassName, int _len);
protected:
	//Check if current row is in filter class vector
	bool checkFilterClass(const char * _filterClassName,int _len)const;
	bool checkFilterClass(const std::string& _filterClassName)const;
//<--Serge

public:
	static void InitDump(const char * _fileName,int _length,int _intWidth=4,int _doubleWidth=10);
	static CDump* GetDumper();
};
}//namespace Dumper
