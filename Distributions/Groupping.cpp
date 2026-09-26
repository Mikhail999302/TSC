#include "Groupping.h"
#include "BoundedSample.h"

Distributions::CSampleHolder* Distributions::CHistogram::Group(const CSampleHolder*_sample)const
{
	CSampleHolder* sample = new CSampleHolder(mNumberOfGroup);
	std::vector<double> values;
	values.reserve(_sample->GetLength());
	for(int i=0; i<_sample->GetLength();++i)
	{
		values.push_back(_sample->GetAt(i));
	}
	double maxVal=*std::max_element(values.begin(),values.end());
	double minVal=*std::min_element(values.begin(),values.end());
	double step=(maxVal-minVal+10e-10)/mNumberOfGroup;
	std::vector<int> frequencies(mNumberOfGroup);
	for(int i=0;i<_sample->GetLength();++i)
	{
		++frequencies[(int)floor((values[i]-minVal)/step)];
	}
	for(int i=0;i<mNumberOfGroup;++i)
	{
		int j=sample->InsertValue(frequencies[i]);
		sample->SetValueInformation(minVal+(i+0.5)*step,j);
	}
	return sample;
}

Distributions::CSampleHolder* Distributions::CBoundedHistogram::Group(const CSampleHolder*_sample )const
{
	const CBoundedSample * sourceSample;
	try
	{
		sourceSample = dynamic_cast<const CBoundedSample*>(_sample);
	}
	catch (std::exception /*&ex*/)
	{
		string dumpStr= _T("BoundedHistogram needs CBoundedSample class cast.");
		string filterStr= _T("Exception");
		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
		throw;
	}
	CBoundedSample* sample=new CBoundedSample(mNumberOfGroup);
	std::vector<double> values;
	values.reserve(sourceSample->GetValidLength());
	for(int i=0; i<sourceSample->GetValidLength();++i)
	{
		values.push_back(sourceSample->GetValidAt(i));
	}
#pragma message("Check if getValidLength is zero")
	double maxVal;
	double minVal;
	if(mIsBoundsSet)
	{
		maxVal = mRight;
		minVal = mLeft;
	}
	else
	{
		maxVal = *std::max_element(values.begin(),values.end());
		minVal = *std::min_element(values.begin(),values.end());
	}
	double step = (maxVal-minVal+10e-10)/mNumberOfGroup;
	std::vector<int> frequencies(mNumberOfGroup);
	for(int i=0; i<sourceSample->GetValidLength(); ++i)
	{
		++frequencies[(int)floor((values[i]-minVal)/step)];
	}
	for(int i=0; i<mNumberOfGroup; ++i)
	{
		int j = sample->InsertValue(frequencies[i]);
		sample->SetValueInformation(minVal+(i+0.5)*step,j);
	}
	for(int i=0; i<sourceSample->GetInvalidLength(); ++i)
	{
		sample->InsertInvalid(sourceSample->GetInvalidAt(i));
	}
	return sample;
}