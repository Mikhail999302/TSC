#pragma once
#include <vector>
#include <iostream>
#include <exception>
#include <algorithm>
#include <ostream>
#include <boost\lexical_cast.hpp>
#include "distributions.h"
#include "..\Dumper\Dump.h"
#include ".\SampleHolder.h"

namespace Distributions
{

#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	DLLDISTTEMPLATE template class DLLDIST std::auto_ptr<IValidateSample>;
	//DLLDISTTEMPLATE template class DLLDIST std::allocator<int>;
	//DLLDISTTEMPLATE template class DLLDIST std::vector<int>;
	//DLLDISTTEMPLATE template class DLLDIST std::allocator<unsigned int>;
	//DLLDISTTEMPLATE template class DLLDIST std::vector<unsigned int>;
	//DLLDISTTEMPLATE template class DLLDIST std::allocator<string>;
	//DLLDISTTEMPLATE template class DLLDIST std::vector<string>;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA

	class DLLDIST CBoundedSample: public CSampleHolder
	{
		std::vector<int> mValid;
		std::vector<int> mInvalid;
		std::auto_ptr<IValidateSample> mValidator;
	public:
		CBoundedSample():mValidator(NULL),mValid(0),mInvalid(0){}
		CBoundedSample(int _size);
		virtual /*overridden*/ int InsertValue(double _value);
		virtual int InsertInvalid(double _value=0);
		virtual int GetValidLength()const;
		virtual int GetInvalidLength()const;
		virtual double GetValidAt(int _idx)const;
		virtual double GetInvalidAt(int _idx)const;
		virtual bool IsValid(int _idx)const;
		virtual void Validator(IValidateSample * _val) { mValidator.reset(_val); }
	};

	void DLLDIST PresentSample(const CBoundedSample* _sample, ostream &_out);
}//namespace Distribution