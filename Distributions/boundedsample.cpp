#include "BoundedSample.h"
#include <tchar.h>

/////////////////////////CBoundedSample/////////////////////////////////
int Distributions::CBoundedSample::InsertInvalid( double _value/*=0*/ )
{
	mInvalid.push_back(this->CSampleHolder::InsertValue(_value));
	return mInvalid.back();
}

int Distributions::CBoundedSample::GetValidLength()const
{
	return (int)mValid.size();
}

int Distributions::CBoundedSample::GetInvalidLength()const
{
	return (int) mInvalid.size();
}

double Distributions::CBoundedSample::GetValidAt( int _idx )const
{
	return mValues[mValid[_idx]];
}

double Distributions::CBoundedSample::GetInvalidAt( int _idx )const
{
	return mValues[mInvalid[_idx]];
}

/*overridden*/ int Distributions::CBoundedSample::InsertValue( double _value )
{
	int idx=this->CSampleHolder::InsertValue(_value);
	if(mValidator.get()==NULL||mValidator->IsValid(_value))
		mValid.push_back(idx);
	else
		mInvalid.push_back(idx);
	return idx;
}

Distributions::CBoundedSample::CBoundedSample( int _size ) :CSampleHolder(_size),mValidator(NULL),mValid(0),mInvalid(0)
{
}

bool Distributions::CBoundedSample::IsValid( int _idx ) const
{
	return mValidator.get()==NULL||mValidator->IsValid(mValues[_idx]);
}

void Distributions::PresentSample(const CBoundedSample* _sample, ostream & _out)
{
	for (int i=0;i<_sample->GetLength();++i)
	{
		if (_sample->IsValid(i))
		{
			_out<<_sample->GetAt(i)<<" ";
		}
	}
	_out<<_sample->GetInvalidLength()<<" ";
}



/////////////////////////////CBoundedHistogram/////////////////////////////////////
