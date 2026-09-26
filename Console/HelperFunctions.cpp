#include "HelperFunctions.h"

void initDumper()
{
	string dumpStr(_T("dump.log"));
	CDump::InitDump(dumpStr.c_str(),(int)dumpStr.size());
	//string filterStr(_T("PLevel"));
	//CDump::GetDumper()->AddFilterClass(filterStr.c_str(),(int)filterStr.size());
	//filterStr=_T("Result");
	//CDump::GetDumper()->AddFilterClass(filterStr.c_str(),(int)filterStr.size());
	//filterStr=_T("Estimates");
	//CDump::GetDumper()->AddFilterClass(filterStr.c_str(),(int)filterStr.size());
	string filterStr=_T("Exception");
	CDump::GetDumper()->AddFilterClass(filterStr.c_str(),(int)filterStr.size());
}

void setDistribution(TSCGenerator::doubles& _distrib)
{
	_distrib.push_back(0.33); //p0 1./5 0.2
	_distrib.push_back(0.33); //p1 1./3 0.33
	//_distrib.push_back(0.1); //p2 0.5*1./3 0.166
	//_distrib.push_back(0.3);
	//_distrib.push_back(0.5*1./3);
	//_distrib.push_back(0.5*1./3);
	//_distrib.push_back(0.6);
	//_distrib.push_back(0.25*0.3);
	//_distrib.push_back(0.25*0.1);
	//_distrib.push_back(0.25*0.3);
	//_distrib.push_back(0.25*0.1);
	//_distrib.push_back(0.25*0.3);
	//_distrib.push_back(0.25*0.1);
	//_distrib.push_back(0.25*0.3);
	//_distrib.push_back(0.1);
	//_distrib.push_back(0.1);
	//_distrib.push_back(0.1);
	//_distrib.push_back(0.1);
	//_distrib.push_back(0.1);
	//_distrib.push_back(0.1);
	_distrib.push_back(0.6); //pch 0.2
	_distrib.push_back(0); //padv 0.1
	//_distrib.push_back(0.3); //px - вероятность влияния рекламы продукта номер 0
}

void setWideDistribution(TSCGenerator::doubles& pbefore_,double& pch_,TSCGenerator::doubles& pafter_)
{
	pch_ = 0.3;
	pbefore_.resize(3);
	pafter_.resize(3);
	pbefore_[0]=1./3;
	pbefore_[1]=1./3;
	pbefore_[2]=1./3;
	pafter_[0]=0.4;
	pafter_[1]=0.6;
	pafter_[2]=0;
}

void setSimleEstimation(CModel * _model,
						vector<boost::shared_ptr<CBoundedSample> >& _sampleEstimates)
{
	_model->ProcessInitialCalculation(false);
	for (int i=0; i<_model->GetEstimateCount(); ++i)
	{
		_sampleEstimates[i]->InsertValue(_model->GetEstimateAt(i).second.Value);
	}
}

void setEstimation(CModel* _model,
				   vector<boost::shared_ptr<CBoundedSample> >& _sampleEstimates,
				   CBoundedSample* _samplePLevel)
{
	_model->ProcessInitialCalculation(true,DataProcessing::CMathOperationIndicator(),true);
	_model->ProcessEstimatesCalculation();
	for (int i=0; i<_model->GetEstimateCount(); ++i)
	{
		_sampleEstimates[i]->InsertValue(_model->GetEstimateAt(i).second.Value);
	}
	
	//Check for chi2 hypotesys with no groupping
	_model->ProcessPLevelCalculation();
	_samplePLevel->InsertValue(_model->GetPLevel().plevel);
}

void printEstimateResults(const vector<double>& _distrib,
						  const vector<boost::shared_ptr<CBoundedSample> >& _sampleEstimates,
						  const string &_grouppingName,
						  std::ostream& _out)
{
	auto_ptr<IGroupping> momentGroup(dynamic_cast<IGroupping*>(new CBoundedMomentsGroupping(2)));
	vector<boost::shared_ptr<CBoundedSample> >moments(_distrib.size());
	vector<boost::shared_ptr<CBoundedSample> >deviations(_distrib.size());
	cout<<endl;
	_out<<endl;
	//for p0, pch, padv
	Distributions::PresentSample(_sampleEstimates[0].get(),_out);
	_out<<endl<<endl;
	Distributions::PresentSample(_sampleEstimates[_distrib.size()-2].get(),_out);
	_out<<endl<<endl;
	Distributions::PresentSample(_sampleEstimates[_distrib.size()-1].get(),_out);
	_out<<endl<<endl;

	//for (size_t i=0; i<_distrib.size(); ++i)
	//{
	//	auto_ptr<IGroupping> deviationGroup(dynamic_cast<IGroupping*>(new CBoundedDeviationGroupping(_distrib[i],2)));
	//	moments[i].reset(dynamic_cast<CBoundedSample*>(_sampleEstimates[i]->GetModification(momentGroup.get())));
	//	deviations[i].reset(dynamic_cast<CBoundedSample*>(_sampleEstimates[i]->GetModification(deviationGroup.get())));
	//	cout<<_distrib[i]<<": "<<moments[i]->GetAt(0)<<" "<<moments[i]->GetAt(1)<<" "<<deviations[i]->GetAt(0)<<" "<<deviations[i]->GetAt(1)<<endl;
	//	_out<<_distrib[i]<<": "<<moments[i]->GetAt(0)<<" "<<moments[i]->GetAt(1)<<" "<<deviations[i]->GetAt(0)<<" "<<deviations[i]->GetAt(1)<<endl;
	//}
}

void printPLevelResults(const CBoundedSample* _samplePLevel, const string &_grouppingName, std::ostream& _out)
{
	const int numIntervals = 100;
	auto_ptr<IGroupping> histoGroup(dynamic_cast<IGroupping*>(new CBoundedHistogram(0.,1.,numIntervals)));
	auto_ptr<CBoundedSample> histo(dynamic_cast<CBoundedSample*>(_samplePLevel->GetModification(histoGroup.get())));
	histo->Validator(new Distributions::CPositiveSampleValidator());
	cout<<_grouppingName<<" ";
	_out<<_grouppingName<<" ";
	Distributions::PresentSample(histo.get(),_out);
	_out<<endl;
	//Distributions::PresentSample(_samplePLevel,_out);
	//_out<<endl;
	Distributions::PresentSample(histo.get(),cout);
	cout<<endl;
	//Distributions::PresentSample(_samplePLevel,cout);
	//PLevel::doubles teoretic(numIntervals);
	//PLevel::doubles empiric(numIntervals);
	//for (int i=0; i<numIntervals; ++i)
	//{
	//	teoretic[i] = 1./numIntervals;
	//	empiric[i] = histo->GetValidAt(i);
	//}
	//PLevel::CPLevel plevel;
	//plevel.Init(teoretic,_samplePLevel->GetValidLength(),Tsc::Grouping::NoneGroup<unsigned int,double>());
	//plevel.CalcChiValue(empiric);
	//cout<<" "<<plevel.GetPLevelData().plevel<<" "<<endl;
	//_out<<" "<<plevel.GetPLevelData().plevel<<" "<<endl;
}

vector<double> getGroupedDistrib(const vector<double>& _distrib,const vector<size_t>& _tempGroup)
{
	size_t length=(*max_element(_tempGroup.begin(),_tempGroup.end()));
	vector<double> rez(length+2,0);
	for (size_t i=0; i<_tempGroup.size(); ++i)
	{
		if(_tempGroup[i]==length)
			continue;
		if (i==_tempGroup.size()-1)
		{
			double sum=1;
			for (size_t j=0; j<i; ++j)
			{
				sum-=_distrib[j];
			}
			rez[_tempGroup[i]]+=sum;
			continue;
		}
		rez[_tempGroup[i]]+=_distrib[i];
	}
	rez[length]=_distrib[_distrib.size()-2];
	rez[length+1]=_distrib[_distrib.size()-1];
	return rez;
}

void printEstimates(TSCGenerator::doubles &distrib, 
					std::ostream& out, 
					vector<boost::shared_ptr<CBoundedSample> >& initEstimates, 
					vector<boost::shared_ptr<CBoundedSample> >& noneEstimates, 
					vector<boost::shared_ptr<CBoundedSample> >& standardEstimates, 
					vector<boost::shared_ptr<CBoundedSample> >& shrinkInitialEstimates, 
					vector<boost::shared_ptr<CBoundedSample> >& shrinkEstimates )
{

	vector<CBoundedSample*> p0;
	p0.reserve(5);
	p0.push_back(initEstimates[0].get());
	p0.push_back(noneEstimates[0].get());
	p0.push_back(standardEstimates[0].get());
	p0.push_back(shrinkInitialEstimates[0].get());
	p0.push_back(shrinkEstimates[0].get());
	int len = p0[0]->GetLength();
	for (size_t i=0; i<p0.size(); ++i)
	{
		if(len>p0[i]->GetLength())
			len=p0[i]->GetLength();
	}
	for (int j=0; j<len; ++j)
	{
		for (size_t i=0; i<p0.size(); ++i)
		{
			if (p0[i]->IsValid(j))
			{
				out<<p0[i]->GetAt(j)<<" ";
			} 
			else
			{
				out<<"- ";
			}
		}
		out<<endl;
	}
	out<<endl;

	vector<CBoundedSample*> pch;
	pch.reserve(5);
	pch.push_back(initEstimates[initEstimates.size()-2].get());
	pch.push_back(noneEstimates[noneEstimates.size()-2].get());
	pch.push_back(standardEstimates[standardEstimates.size()-2].get());
	pch.push_back(shrinkInitialEstimates[shrinkInitialEstimates.size()-2].get());
	pch.push_back(shrinkEstimates[shrinkEstimates.size()-2].get());
	len = pch[0]->GetLength();
	for (size_t i=0; i<pch.size(); ++i)
	{
		if(len>pch[i]->GetLength())
			len=pch[i]->GetLength();
	}
	for (int j=0; j<len; ++j)
	{
		for (size_t i=0; i<pch.size(); ++i)
		{
			if (pch[i]->IsValid(j))
			{
				out<<pch[i]->GetAt(j)<<" ";
			} 
			else
			{
				out<<"- ";
			}
		}
		out<<endl;
	}
	out<<endl;

	vector<CBoundedSample* > padv;
	padv.reserve(5);
	padv.push_back(initEstimates[initEstimates.size()-1].get());
	padv.push_back(noneEstimates[noneEstimates.size()-1].get());
	padv.push_back(standardEstimates[standardEstimates.size()-1].get());
	padv.push_back(shrinkInitialEstimates[shrinkInitialEstimates.size()-1].get());
	padv.push_back(shrinkEstimates[shrinkEstimates.size()-1].get());
	len = padv[0]->GetLength();
	for (size_t i=0; i<padv.size(); ++i)
	{
		if(len>padv[i]->GetLength())
			len=padv[i]->GetLength();
	}
	for (int j=0; j<len; ++j)
	{
		for (size_t i=0; i<padv.size(); ++i)
		{
			if (padv[i]->IsValid(j))
			{
				out<<padv[i]->GetAt(j)<<" ";
			} 
			else
			{
				out<<"- ";
			}
		}
		out<<endl;
	}
	out<<endl;


	//printEstimateResults(distrib, initEstimates, "Initial", out);
	//printEstimateResults(distrib, noneEstimates,"Simple",out);
	//printEstimateResults(distrib, standardEstimates,"Standard",out);
	//Tsc::Grouping::ShrinkGroup<size_t, double> grouping;
	//grouping.setSize(0);
	//grouping.setStates(doubles((distrib.size()-1)*(distrib.size()-1)));
	//grouping.setLevel(0);
	//grouping.setMinSize(0);
	//vector<size_t> tempGroup = grouping();
	//doubles groupedDistrib=getGroupedDistrib(distrib,tempGroup);
	//printEstimateResults(groupedDistrib,shrinkInitialEstimates,"Initial shrink",out);
	//printEstimateResults(groupedDistrib, shrinkEstimates,"Shrink",out);
}

void printEstimates0(TSCGenerator::doubles &distrib, 
					std::ostream& out, 
					vector<boost::shared_ptr<CBoundedSample> >& initEstimates, 
					vector<boost::shared_ptr<CBoundedSample> >& noneEstimates
					)
{

	vector<CBoundedSample*> p0;
	p0.reserve(5);
	p0.push_back(initEstimates[0].get());
	p0.push_back(noneEstimates[0].get());
	int len = p0[0]->GetLength();
	for (size_t i=0; i<p0.size(); ++i)
	{
		if(len>p0[i]->GetLength())
			len=p0[i]->GetLength();
	}
	for (int j=0; j<len; ++j)
	{
		for (size_t i=0; i<p0.size(); ++i)
		{
			if (p0[i]->IsValid(j))
			{
				out<<p0[i]->GetAt(j)<<" ";
			} 
			else
			{
				out<<"- ";
			}
		}
		out<<endl;
	}
	out<<endl;

	vector<CBoundedSample*> pch;
	pch.reserve(5);
	pch.push_back(initEstimates[initEstimates.size()-2].get());
	pch.push_back(noneEstimates[noneEstimates.size()-2].get());
	len = pch[0]->GetLength();
	for (size_t i=0; i<pch.size(); ++i)
	{
		if(len>pch[i]->GetLength())
			len=pch[i]->GetLength();
	}
	for (int j=0; j<len; ++j)
	{
		for (size_t i=0; i<pch.size(); ++i)
		{
			if (pch[i]->IsValid(j))
			{
				out<<pch[i]->GetAt(j)<<" ";
			} 
			else
			{
				out<<"- ";
			}
		}
		out<<endl;
	}
	out<<endl;

	vector<CBoundedSample* > padv;
	padv.reserve(5);
	padv.push_back(initEstimates[initEstimates.size()-1].get());
	padv.push_back(noneEstimates[noneEstimates.size()-1].get());
	len = padv[0]->GetLength();
	for (size_t i=0; i<padv.size(); ++i)
	{
		if(len>padv[i]->GetLength())
			len=padv[i]->GetLength();
	}
	for (int j=0; j<len; ++j)
	{
		for (size_t i=0; i<padv.size(); ++i)
		{
			if (padv[i]->IsValid(j))
			{
				out<<padv[i]->GetAt(j)<<" ";
			} 
			else
			{
				out<<"- ";
			}
		}
		out<<endl;
	}
	out<<endl;


	//printEstimateResults(distrib, initEstimates, "Initial", out);
	//printEstimateResults(distrib, noneEstimates,"Simple",out);
	//printEstimateResults(distrib, standardEstimates,"Standard",out);
	//Tsc::Grouping::ShrinkGroup<size_t, double> grouping;
	//grouping.setSize(0);
	//grouping.setStates(doubles((distrib.size()-1)*(distrib.size()-1)));
	//grouping.setLevel(0);
	//grouping.setMinSize(0);
	//vector<size_t> tempGroup = grouping();
	//doubles groupedDistrib=getGroupedDistrib(distrib,tempGroup);
	//printEstimateResults(groupedDistrib,shrinkInitialEstimates,"Initial shrink",out);
	//printEstimateResults(groupedDistrib, shrinkEstimates,"Shrink",out);
}