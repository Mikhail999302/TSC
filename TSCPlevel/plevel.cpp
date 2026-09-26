#include <tchar.h>
#include ".\plevel.h"

const double PLevel::CPLevel::eps=10e-10;

PLevel::CPLevel::~CPLevel(void)
{
}

void PLevel::CPLevel::Init(const doubles& fullTheoretic_, int size_, Tsc::Grouping::IMergeGrouping<uint,double>& _strategy)
{
	fullTheoretic.clear();
	fullTheoretic=fullTheoretic_;
	size=size_;
	mergeStrategy.clear();
	_strategy.setSize(size);
	_strategy.setStates(fullTheoretic);
	_strategy.setLevel(5.0);
	_strategy.setMinSize(1);
	mergeStrategy=_strategy();
	//mergeStrategy=_strategy(size,fullTheoretic,5.0,1);
	//for(uints::const_iterator p=mergeStrategy.begin();p!=mergeStrategy.end();++p)std::cout<<*p<<" ";
	unsigned int maxelem=*max_element(mergeStrategy.begin(),mergeStrategy.end());
	theoretic.clear();
	empiric.clear();
	theoretic.resize(maxelem+1);
	empiric.resize(maxelem+1);
	SumByIdx(fullTheoretic,mergeStrategy,theoretic);
	isInit=true;
}

void PLevel::CPLevel::SumByIdx(const doubles& from,const uints& with,doubles& to)
{
	//unsigned int maxelem=*max_element(with.begin(),with.end());
	for(uints::const_iterator p=with.begin();p!=with.end();++p)
	{
		to[*p]+=*(from.begin()+(p-with.begin()));
	}
}

void PLevel::CPLevel::CalcChiValue(const doubles& fullEmpiric,bool isCheckSize/*=false*/)
{
	if(!isInit)throw Exceptions::NotInitializedException();
	if(isCheckSize)if(fullTheoretic.size()!=fullEmpiric.size())throw Exceptions::IncorrectSizeException();
	//if(!isInit)return;
	//if(isCheckSize)if(fullTheoretic.size()!=fullEmpiric.size())return;
	for(doubles::iterator p=empiric.begin();p!=empiric.end();++p)*p=0;
	SumByIdx(fullEmpiric,mergeStrategy,empiric);
	chiData.df=(int)empiric.size()-1;
	chiData.criterion_value=0;
	double nomi=0,npi=0;
	chiData.minNPi=1.0e10;
	for(doubles::const_iterator p=theoretic.begin();p!=theoretic.end();++p)
	{
		npi=size*(*p);
		if(npi<chiData.minNPi)chiData.minNPi=npi;
		if(npi>eps)
		{
			nomi=*(empiric.begin()+(p-theoretic.begin()))-npi;
			nomi*=nomi;
			chiData.criterion_value+=nomi/npi;
		}
	}
	if(chiData.df>=1)
		chiData.plevel=1-pChi(chiData.criterion_value,chiData.df);
	else 
	{
		chiData.plevel=1;
		std::string dumpStr="To few degree of freedom";
		std::string filterStr="PLevel";
		Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
	}
}

void PLevel::CPLevel::CalcChiValue(const doubles& fullEmpiric,int df,bool isCheckSize/*=false*/)
{
	//if(!isInit)return;
	//if(isCheckSize)if(fullTheoretic.size()!=fullEmpiric.size())return;
	if(!isInit)throw Exceptions::NotInitializedException();
	if(isCheckSize)if(fullTheoretic.size()!=fullEmpiric.size())throw Exceptions::IncorrectSizeException();
	for(doubles::iterator p=empiric.begin();p!=empiric.end();++p)*p=0;

	//Groupped sample dump
	std::string dumpStr=_T("Groupped sample:\n");
	std::string filterStr=_T("PLevel");
	Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
	Dumper::CDump::GetDumper()->Dump(&fullEmpiric.front(),(int)fullEmpiric.size(),5,filterStr.c_str(),(int)filterStr.size());
	Dumper::CDump::GetDumper()->NewLine(filterStr.c_str(),(int)filterStr.size());

	SumByIdx(fullEmpiric,mergeStrategy,empiric);
	chiData.df=df;
	chiData.criterion_value=0;
	double nomi=0,npi=0;
	chiData.minNPi=1.0e10;
	dumpStr=_T("Xi2 contribution\n idx \t ni \t npi \t xi\n");
	Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
	for(doubles::const_iterator p=theoretic.begin();p!=theoretic.end();++p)
	{
		npi=size*(*p);
		if(npi<chiData.minNPi)chiData.minNPi=npi;
		if(npi>eps)
		{
			nomi=*(empiric.begin()+(p-theoretic.begin()))-npi;
			nomi*=nomi;
			nomi/=npi;
			Dumper::CDump::GetDumper()->Dump((int)(p-theoretic.begin()),filterStr.c_str(),(int)filterStr.size());
			Dumper::CDump::GetDumper()->Dump(*(empiric.begin()+(p-theoretic.begin())),filterStr.c_str(),(int)filterStr.size());
			Dumper::CDump::GetDumper()->Dump(npi,filterStr.c_str(),(int)filterStr.size());
			Dumper::CDump::GetDumper()->Dump(nomi,filterStr.c_str(),(int)filterStr.size());
			Dumper::CDump::GetDumper()->NewLine(filterStr.c_str(),(int)filterStr.size());
			chiData.criterion_value+=nomi;
		}
		else
		{
			dumpStr=_T("Error: npi<=0\n");
			filterStr=_T("Exception");
			Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
			chiData.df=0;
			chiData.criterion_value=0;
			chiData.plevel=-1;
			return;
		}
	}
	if(chiData.df>0)
		chiData.plevel=1-pChi(chiData.criterion_value,chiData.df);
	else throw std::exception("DF<1");
}
