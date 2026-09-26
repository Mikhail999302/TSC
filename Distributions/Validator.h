#pragma once
namespace Distributions
{
	class DLLDIST IValidateSample
	{
	public:
		virtual ~IValidateSample(){}
		virtual bool IsValid(double _value)const =0;
	};

	class DLLDIST CProbabilitySampleValidator:public IValidateSample
	{
	public:
		virtual /*overridden*/ bool IsValid(double _value)const 
		{
			return _value<1&&_value>0;
		}
	};

	class DLLDIST CPositiveSampleValidator:public IValidateSample
	{
	public:
		virtual /*overridden*/ bool IsValid(double _value)const 
		{
			return _value>=0;
		}
	};
}//namespace Distributions