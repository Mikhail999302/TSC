#pragma once
#include <vector>
#include <iostream>
#include <exception>
#include <algorithm>
#include <boost\lexical_cast.hpp>
#include "..\Dumper\Dump.h"

#include "distributions.h"
#include "Groupping.h"
#include "Validator.h"

namespace Distributions
{
	using boost::lexical_cast;
	using boost::bad_lexical_cast;
	using namespace std;

	class IGroupping;
	class IValidateSample;

#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA
	DLLDISTTEMPLATE template class DLLDIST std::allocator<double>;
	DLLDISTTEMPLATE template class DLLDIST std::vector<double>;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 ) //disable warnings on extern before template instantiation
#endif // USE_PRAGMA

	class DLLDIST CSampleHolder
	{
	protected:
		std::vector<double> mValues;
		std::vector<std::string> mXValues;
	public:
		CSampleHolder(void);
		CSampleHolder(int _size);
		virtual ~CSampleHolder(void);
		virtual int InsertValue(double _value);
		virtual void SetValueInformation(double _xValue,int _idx);
		virtual void SetValueInformation(const char*_xValue,int _size, int _idx);
		virtual CSampleHolder* GetModification(const IGroupping * _group)const;
		virtual double GetAt(int _idx)const;
		virtual const char * GetInformationAt(int _idx)const;
		virtual int GetLength()const;
	};
}//namespace Distributions