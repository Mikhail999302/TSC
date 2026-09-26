#include ".\generator.h"
//методы базового класса CGenerator
void TSCGenerator::CGenerator::Generate(void)
{
	InitModel();
	for(int i=size;i-->0;)GenerateOne();
	//TransposeSample();
}

void TSCGenerator::CGenerator::TransposeSample()
{
	int q,r;
	dim=(int)sqrt((double)sample.size());
	for(int i=(int)sample.size();i-->0;)
	{
		if((q=i/dim)<(r=i%dim))std::swap(sample[q*dim+r],sample[r*dim+q]);
	}
}

TSCGenerator::CGenerator::CGenerator(const CGenerator&gen)
{
	distribution=gen.distribution;
	sample=gen.sample;
	size=gen.size;
	alpha=new CUniformDist(gen.alpha->GetSeed());
	isInit=gen.isInit;
	poly=0;
	if(isInit)
	{
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.distribution,gen.poly->GetParam());
	}
}

TSCGenerator::CGenerator& TSCGenerator::CGenerator::operator=(const CGenerator &gen)
{
	if(this==&gen)
		return *this;
	delete alpha;
	distribution=gen.distribution;
	sample=gen.sample;
	size=gen.size>0?gen.size:0;
	alpha=new CUniformDist(gen.alpha->GetSeed());
	delete poly;
	poly=0;
	isInit=gen.isInit;
	if(isInit)
	{
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.distribution,gen.poly->GetParam());
	}
	return *this;
}

void TSCGenerator::CGenerator::SaveSample(const char * fileName,bool isTranspose/*=false*/)
{
	if(isInit)
	{
		std::ofstream fout(fileName);
		int i=0;
		if(isTranspose)TransposeSample();
		dim=(int)sqrt((double)sample.size());
		for(doubles::iterator p=sample.begin();p!=sample.end();p++)
		{
			fout<<(*p)<<" ";
			if(!((++i)%dim))fout<<std::endl;
		}
	}
}

//****************************************
//******  class CGenerator_1Pos   ********
//модель с одним рекламируемым продуктом**
//****************************************
void TSCGenerator::CGenerator_1Pos::InitModel(void)
{
	pi=distribution;
	padv=pi.back();
	pi.pop_back();
	pch=pi.back();
	pi.pop_back();
	double plast=1;
	for(doubles::iterator p=pi.begin();p!=pi.end();++p)plast-=*p;
	pi.push_back(plast);
	delete poly;
	poly=new CChenUFDD(alpha->GetSeed(),pi,(int)pi.size());
	for(doubles::iterator p=sample.begin();p!=sample.end();++p)*p=0;
	dim=(int)sqrt((double)sample.size());
	isInit=true;
}

void TSCGenerator::CGenerator_1Pos::GenerateOne(void)
{
	if(!isInit)InitModel();
	int first=(int)poly->Next();
	if(alpha->Next()<padv)sample[first*dim]++;
	else
	{
		if(alpha->Next()<pch)sample[(unsigned int)(first*dim+poly->Next())]++;
		else sample[first*dim+first]++;
	}
}


TSCGenerator::CGenerator_1Pos::CGenerator_1Pos(const CGenerator_1Pos &gen):CGenerator(gen)
{
	//distribution=gen.distribution;
	//sample=gen.sample;
	//size=gen.size;
	//alpha=new CUniformDist(gen.alpha->GetSeed());
	//isInit=gen.isInit;
	poly=0;
	if(isInit)
	{
		pi=gen.pi;
		padv=gen.padv;
		pch=gen.pch;
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.pi,gen.poly->GetParam());
	}
}

TSCGenerator::CGenerator_1Pos& TSCGenerator::CGenerator_1Pos::operator=(const CGenerator_1Pos &gen)
{
	if(this==&gen)
		return *this;
	delete alpha;
	distribution=gen.distribution;
	sample=gen.sample;
	size=gen.size>0?gen.size:0;
	alpha=new CUniformDist(gen.alpha->GetSeed());
	delete poly;
	poly=0;
	isInit=gen.isInit;
	if(isInit)
	{
		pi=gen.pi;
		padv=gen.padv;
		pch=gen.pch;
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.pi,gen.poly->GetParam());
	}
	return *this;
}

//****************************************
//******  class CGenerator_1Pos_mine  ********
//модель с одним рекламируемым продуктом для новой модели**
//****************************************

TSCGenerator::CGenerator_1Pos_mine::CGenerator_1Pos_mine(const CGenerator_1Pos_mine& gen) :CGenerator_1Pos(gen)
{
	//distribution=gen.distribution;
	//sample=gen.sample;
	//size=gen.size;
	//alpha=new CUniformDist(gen.alpha->GetSeed());
	//isInit=gen.isInit;
	/*poly = 0;
	if (isInit)
	{
		pi = gen.pi;
		padv = gen.padv;
		pch = gen.pch;
		dim = gen.size;
		poly = new CChenUFDD(gen.poly->GetSeed(), gen.pi, gen.poly->GetParam());
	}*/
}

TSCGenerator::CGenerator_1Pos_mine& TSCGenerator::CGenerator_1Pos_mine::operator=(const CGenerator_1Pos_mine& gen)
{
	if (this == &gen)
		return *this;
	delete alpha;
	distribution = gen.distribution;
	sample = gen.sample;
	size = gen.size > 0 ? gen.size : 0;
	alpha = new CUniformDist(gen.alpha->GetSeed());
	delete poly;
	poly = 0;
	isInit = gen.isInit;
	if (isInit)
	{
		pi = gen.pi;
		padv = gen.padv;
		pch = gen.pch;
		dim = gen.size;
		poly = new CChenUFDD(gen.poly->GetSeed(), gen.pi, gen.poly->GetParam());
	}
	return *this;
}

void TSCGenerator::CGenerator_1Pos_mine::GenerateOne(void)
{
	if (!isInit)InitModel();
	int first = (int)poly->Next();
	if (alpha->Next() < 1 - pch) sample[first * dim + first]++;
	else
	{
		if (alpha->Next() < padv) sample[first * dim]++;
		else sample[(unsigned int)(first * dim + poly->Next())]++;
	}
}

//****************************************
//******  class CGenerator_2Pos   ********
//модель с двумя положит.рекламами********
//****************************************

void TSCGenerator::CGenerator_2Pos::InitModel(void)
{
	pi = distribution;
	px = pi.back();//вероятность действия рекламы продукта номер 0(в самом конце вектора pi стоит px)
	pi.pop_back();
	padv = pi.back();//вероятность действия рекламы вообще
	pi.pop_back();
	pch = pi.back();
	pi.pop_back();
	double plast = 1;
	for(doubles::iterator p = pi.begin(); p != pi.end(); ++p) plast -= *p;
	pi.push_back(plast);
	delete poly;
	poly = new CChenUFDD(alpha->GetSeed(),pi,(int)pi.size());
	for(doubles::iterator p = sample.begin(); p != sample.end(); ++p)*p=0;
	dim = (int)sqrt((double)sample.size());
	isInit = true;
}

void TSCGenerator::CGenerator_2Pos::GenerateOne(void)//моделирование 1 реализации
{
	//элемент выборки с координатами [i,j] --> это [i*dim+j]
	double a;
	if (!isInit) InitModel();
	int first = (int)poly->Next();
	if( (a = alpha->Next()) < padv)
	{
		if ( (a = alpha->Next()) < px) sample[first*dim]++; //переход в нулевое состояние(сработала реклама продукта номер 0)
		else sample[first*dim + 1]++;//переход в состояние 1(сработала рекл.продукта номер 1)
	}
	else
	{
		if (alpha->Next() < pch) sample[(unsigned int)(first*dim+poly->Next())]++;
		else sample[first*dim+first]++; //остался на месте
	}
}


TSCGenerator::CGenerator_2Pos::CGenerator_2Pos(const CGenerator_2Pos&gen):CGenerator(gen)
{
	poly=0;
	if(isInit)
	{
		pi=gen.pi;
		padv=gen.padv;
		px=gen.px;
		pch=gen.pch;
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.pi,gen.poly->GetParam());
	}
}

TSCGenerator::CGenerator_2Pos& TSCGenerator::CGenerator_2Pos::operator=(const CGenerator_2Pos &gen)
{
	if(this==&gen)
		return *this;
	delete alpha;
	distribution=gen.distribution;
	sample=gen.sample;
	size=gen.size>0?gen.size:0;
	alpha=new CUniformDist(gen.alpha->GetSeed());
	delete poly;
	poly=0;
	isInit=gen.isInit;
	if(isInit)
	{
		pi=gen.pi;
		padv=gen.padv;
		px=gen.px;
		pch=gen.pch;
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.pi,gen.poly->GetParam());
	}
	return *this;
}



//****************************************
//******  class CGenerator_1Neg   ********
//модель с одной отрицательной рекламой **
//****************************************
void TSCGenerator::CGenerator_1Neg::InitModel(void)
{
	pi=distribution;
	padv=pi.back();
	pi.pop_back();
	pch=pi.back();
	pi.pop_back();
	double plast=1;
	for(doubles::iterator p=pi.begin();p!=pi.end();++p)plast-=*p;
	pi.push_back(plast);
	delete poly;
	poly=new CChenUFDD(alpha->GetSeed(),pi,(int)pi.size());
	for(doubles::iterator p=sample.begin();p!=sample.end();++p)*p=0;
	dim=(int)sqrt((double)sample.size());
	isInit=true;
}

void TSCGenerator::CGenerator_1Neg::GenerateOne(void)
{
	if(!isInit)InitModel();
	double one_p0 = 1-pi[0];
	doubles s = pi;
	std::partial_sum(s.begin()+1,s.end(),s.begin()+1);
	s[0] = 0;
	//for (int i = 1; i < dim; ++i)
	//	s[i] /= one_p0;

	int first=(int)poly->Next();
	if(alpha->Next() < padv)
	{
		double a = alpha->Next();
		int j = 1;
		while (a > s[j]/one_p0) ++j;
		++sample[first*dim + j];
	}
	else
	{
		if(alpha->Next() < pch)
			sample[(unsigned int)(first*dim+poly->Next())]++;
		else sample[first*dim+first]++;
	}
}


TSCGenerator::CGenerator_1Neg::CGenerator_1Neg(const CGenerator_1Neg &gen):CGenerator(gen)
{
	//distribution=gen.distribution;
	//sample=gen.sample;
	//size=gen.size;
	//alpha=new CUniformDist(gen.alpha->GetSeed());
	//isInit=gen.isInit;
	poly=0;
	if(isInit)
	{
		pi=gen.pi;
		padv=gen.padv;
		pch=gen.pch;
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.pi,gen.poly->GetParam());
	}
}

TSCGenerator::CGenerator_1Neg& TSCGenerator::CGenerator_1Neg::operator=(const CGenerator_1Neg &gen)
{
	if(this==&gen)
		return *this;
	delete alpha;
	distribution=gen.distribution;
	sample=gen.sample;
	size=gen.size>0?gen.size:0;
	alpha=new CUniformDist(gen.alpha->GetSeed());
	delete poly;
	poly=0;
	isInit=gen.isInit;
	if(isInit)
	{
		pi=gen.pi;
		padv=gen.padv;
		pch=gen.pch;
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.pi,gen.poly->GetParam());
	}
	return *this;
}



//*******************************************
//******  class CGenerator_2NegPos   ********
//модель с двумя рекламами: "-" и "+" *******
//*******************************************
void TSCGenerator::CGenerator_2NegPos::InitModel(void)
{
	pi = distribution;
	px = pi.back();//вероятность действия рекламы продукта номер 0(в самом конце вектора pi стоит px)
	pi.pop_back();
	padv = pi.back();//вероятность действия рекламы вообще
	pi.pop_back();
	pch = pi.back();
	pi.pop_back();
	double plast = 1;
	for(doubles::iterator p = pi.begin(); p != pi.end(); ++p) plast -= *p;
	pi.push_back(plast);
	delete poly;
	poly = new CChenUFDD(alpha->GetSeed(),pi,(int)pi.size());
	for(doubles::iterator p = sample.begin(); p != sample.end(); ++p)*p=0;
	dim = (int)sqrt((double)sample.size());
	isInit = true;
}

void TSCGenerator::CGenerator_2NegPos::GenerateOne(void)
{
	if(!isInit)InitModel();
	double one_p0 = 1-pi[0];
	doubles s = pi;
	std::partial_sum(s.begin()+1,s.end(),s.begin()+1);
	s[0] = 0;
	//for (int i = 1; i < dim; ++i)
	//	s[i] /= one_p0;

	int first=(int)poly->Next();
	if(alpha->Next() < padv) //если реклама подействовала
	{
		if(alpha->Next() < px) //если подействовала отриц.реклама продукта "0"
		{
			double a = alpha->Next();
			int j = 1;
			while (a > s[j]/one_p0) ++j;
			++sample[first*dim + j];
		}
		else  
			++sample[first*dim + 1];  //если подействовала положит.реклама продукта "1"
	}
	else            //если реклама НЕ подействовала
	{
		if(alpha->Next() < pch)
			sample[(unsigned int)(first*dim+poly->Next())]++;
		else sample[first*dim+first]++;
	}
}


TSCGenerator::CGenerator_2NegPos::CGenerator_2NegPos(const CGenerator_2NegPos &gen):CGenerator(gen)
{
	poly=0;
	if(isInit)
	{
		pi=gen.pi;
		padv=gen.padv;
		px=gen.px;
		pch=gen.pch;
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.pi,gen.poly->GetParam());
	}
}

TSCGenerator::CGenerator_2NegPos& TSCGenerator::CGenerator_2NegPos::operator=(const CGenerator_2NegPos &gen)
{
	if(this==&gen)
		return *this;
	delete alpha;
	distribution=gen.distribution;
	sample=gen.sample;
	size=gen.size>0?gen.size:0;
	alpha=new CUniformDist(gen.alpha->GetSeed());
	delete poly;
	poly=0;
	isInit=gen.isInit;
	if(isInit)
	{
		pi=gen.pi;
		padv=gen.padv;
		px=gen.px;
		pch=gen.pch;
		dim=gen.size;
		poly=new CChenUFDD(gen.poly->GetSeed(),gen.pi,gen.poly->GetParam());
	}
	return *this;
}
