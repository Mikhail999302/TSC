#include "SampleHolder.h"

Distributions::CSampleHolder::CSampleHolder(void)
{
}

Distributions::CSampleHolder::CSampleHolder(int _size)
{
	mValues.reserve(_size);
}

Distributions::CSampleHolder::~CSampleHolder(void)
{
}

int Distributions::CSampleHolder::InsertValue( double _value )
{
	mValues.push_back(_value);
	int curIdx=(int)mValues.size()-1;
	try
	{
		mXValues.push_back(boost::lexical_cast<std::string>(curIdx));
	}
	catch(bad_lexical_cast &)
	{
		std::cout<<"Bad lexical cast";
		throw;
	}
	return curIdx;
}

void Distributions::CSampleHolder::SetValueInformation( double _xValue,int _idx )
{
	try
	{
		mXValues[_idx]=boost::lexical_cast<std::string,double>(_xValue);
	}
	catch(bad_lexical_cast &)
	{
		std::cout<<"Bad lexical cast";
		throw;
	}
	catch(exception &ex)
	{
		std::cout<<ex.what();
	}
}

void Distributions::CSampleHolder::SetValueInformation( const char*_xValue,int _size, int _idx )
{
	try
	{
		mXValues[_idx]=_xValue;
	}
	catch (exception &ex)
	{
		std::cout<<ex.what();
	}
}

Distributions::CSampleHolder* Distributions::CSampleHolder::GetModification(const IGroupping * _group )const
{
	return _group->Group(this);
}

const char * Distributions::CSampleHolder::GetInformationAt( int _idx )const
{
	return mXValues[_idx].c_str();
}

double Distributions::CSampleHolder::GetAt( int _idx )const
{
	return mValues[_idx];
}

int Distributions::CSampleHolder::GetLength()const
{
	return (int)mValues.size();
}