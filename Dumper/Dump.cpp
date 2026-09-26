#include "Dump.h"
std::auto_ptr<Dumper::CDump> Dumper::CDump::dump(0);

Dumper::CDump::CDump(const char * _fileName,int _length,int _intWidth/*=4*/,int _doubleWidth/*=10*/):
		mFileName(_fileName,_fileName+_length),mOut(mFileName.c_str()),mIntWidth(_intWidth),mDoubleWidth(_doubleWidth)
{
}

void Dumper::CDump::Dump(const char*_str,int _len,const char* _filterClass/*=_T("")*/,int _fcLen/*=1*/)
{
	if(checkFilterClass(_filterClass,_fcLen))
	{
		std::string str(_str,_str+_len);
		mOut<<str;
		mOut.flush();
	}
}

void Dumper::CDump::Dump(const std::string& _str, const std::string& _filterClass /*= ""*/)
{
	if(checkFilterClass(_filterClass))
	{
		mOut<<_str;
		mOut.flush();
	}
}

void Dumper::CDump::Dump(int _value,const char* _filterClass/*=_T("")*/,int _fcLen/*=1*/)
{
	if(checkFilterClass(_filterClass,_fcLen))
	{
		mOut.width(mIntWidth);
		mOut<<_value;
		mOut.flush();
	}
}

void Dumper::CDump::Dump(double _value,const char* _filterClass/*=_T("")*/,int _fcLen/*=1*/)
{
	if(checkFilterClass(_filterClass,_fcLen))
	{
		mOut.width(mDoubleWidth);
		mOut<<_value;
		mOut.flush();
	}
}

Dumper::CDump::~CDump(void)
{
	mOut.flush();
	mOut.close();
}

Dumper::CDump* Dumper::CDump::GetDumper()
{
	return dump.get();
}

void Dumper::CDump::InitDump(const char *_fileName, int _length, int _intWidth/* = 4*/, int _doubleWidth/* = 10*/)
{
	if(!dump.get())dump.reset(new CDump(_fileName,_length,_intWidth,_doubleWidth));
}

void Dumper::CDump::AddFilterClass( const char * _filterClassName,int _len )
{
	using namespace std;
	string filterClassName(_filterClassName);
	//Check that it hasn't been added earlier
	vector<string>::iterator iter=std::find(mFilterClasses.begin(),mFilterClasses.end(),filterClassName);
	if(mFilterClasses.end()==iter)
	{
		mFilterClasses.push_back(filterClassName);
	}
}

bool Dumper::CDump::checkFilterClass( const char * _filterClassName,int _len )const
{
	if(_len<2)return true;
	using namespace std;
	string filterClassName(_filterClassName);
	return (mFilterClasses.end()!=std::find(mFilterClasses.begin(),mFilterClasses.end(),filterClassName));
}

bool Dumper::CDump::checkFilterClass(const std::string & _filterClassName)const
{
	if(_filterClassName.empty())return true;
	return (mFilterClasses.end()!=std::find(mFilterClasses.begin(),mFilterClasses.end(),_filterClassName));
}

void Dumper::CDump::DeleteFilterClass( const char * _filterClassName, int _len )
{
	using namespace std;
	string filterClassName(_filterClassName);
	vector<string>::iterator iter=std::find(mFilterClasses.begin(),mFilterClasses.end(),filterClassName);
	if(mFilterClasses.end()!=iter)
	{
		mFilterClasses.erase(iter);
	}
}