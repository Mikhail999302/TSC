#include ".\widegenerator.h"

void TSCGenerator::CWideGenerator::Generate(void)
{
	InitModel();
	for(int i=size;i-->0;)
		GenerateOne();
}

void TSCGenerator::CWideGenerator::InitModel(void)
{
	delete PBeforeGen;
	delete PAfterGen;
	PBeforeGen = new CChenUFDD(alpha->GetSeed(),PBefore,(int)PBefore.size());
	PAfterGen = new CChenUFDD(alpha->GetSeed(),PAfter,(int)PAfter.size());
	for(doubles::iterator iter=sample.begin(); iter!=sample.end(); ++iter)
		*iter=0;
	dim=(int)sqrt((double)sample.size());
	isInit=true;
}

void TSCGenerator::CWideGenerator::GenerateOne(void)
{
	if(!isInit)
		InitModel();
	int first = (int)PBeforeGen->Next();
	if(alpha->Next()<pch)
		++(sample[(unsigned int)(first*dim+PAfterGen->Next())]);
	else ++(sample[first*dim+first]);
}

void TSCGenerator::CWideGenerator::TransposeSample()
{
	int q,r;
	dim=(int)sqrt((double)sample.size());
	for(int i=(int)sample.size();i-->0;)
	{
		if((q=i/dim)<(r=i%dim))
			std::swap(sample[q*dim+r],sample[r*dim+q]);
	}
}

TSCGenerator::CWideGenerator::CWideGenerator(const CWideGenerator&gen):
		sample(gen.sample),size(gen.size),alpha(new CUniformDist(gen.alpha->GetSeed())), PBefore(gen.PBefore),
			PAfter(gen.PAfter),isInit(gen.isInit),pch(gen.pch),dim(0),PBeforeGen(0),PAfterGen(0)
{
	if(isInit)
	{		
		dim = gen.dim;
		PBeforeGen = new CChenUFDD(gen.PBeforeGen->GetSeed(),gen.PBefore,gen.PBeforeGen->GetParam());
		PAfterGen = new CChenUFDD(gen.PAfterGen->GetSeed(),gen.PAfter,gen.PAfterGen->GetParam());
	}
}

TSCGenerator::CWideGenerator& TSCGenerator::CWideGenerator::operator=(const CWideGenerator &gen)
{
	if(this==&gen)
		return *this;
	delete alpha;
	sample = gen.sample;
	size = gen.size>0?gen.size:0;
	PBefore = gen.PBefore;
	PAfter = gen.PAfter;
	pch = gen.pch;
	alpha = new CUniformDist(gen.alpha->GetSeed());
	delete PBeforeGen;
	PBeforeGen = 0;
	delete PAfterGen;
	PAfterGen = 0;
	isInit = gen.isInit;
	if(isInit)
	{
		dim=gen.size;
		PBeforeGen=new CChenUFDD(gen.PBeforeGen->GetSeed(),gen.PBefore,gen.PBeforeGen->GetParam());
		PAfterGen=new CChenUFDD(gen.PAfterGen->GetSeed(),gen.PAfter,gen.PAfterGen->GetParam());
	}
	return *this;
}

void TSCGenerator::CWideGenerator::SaveSample(const char * fileName,bool isTranspose/*=false*/)
{
	if(isInit)
	{
		std::ofstream fout(fileName);
		int i=0;
		if(isTranspose)
			TransposeSample();
		dim=(int)sqrt((double)sample.size());
		for(doubles::iterator p=sample.begin();p!=sample.end();++p)
		{
			fout<<(*p)<<" ";
			if(!((++i)%dim))fout<<std::endl;
		}
	}
}