#pragma once
#include <C:\diploma\2012\TSC\Libs/TableManager/Entry.h>
#include "..\TSCGenerator\IGenerator.h"
#include "..\Dumper\Dump.h"

namespace Tsc
{
namespace TableManager
{
template <typename Elem>
class CTableManagerFactory
{
public://static
	static CTableManager<Elem>* CreateFromFile(const char* _fileName)
	{
		std::string fileName(_fileName);
		std::string::size_type ptPos;
		std::auto_ptr<IDataLoader<Elem> > loader(0);
		CTableManager<Elem> *rez=0;
		try
		{
			if((ptPos=fileName.find('.'))!=fileName.npos)
			{
				std::string ext(fileName.substr(ptPos));
				if(!ext.compare(".txt"))loader.reset(new Tsc::TableManager::CPlainTextLoader<Elem>(fileName.c_str()));
				else throw std::exception("Not correct ext");
			}
			rez=loader->CreateTableManager();
		}
		catch(std::exception&ex)
		{
			std::string dumpStr=ex.what();
			std::string filterStr="Exception";
			Dumper::CDump::GetDumper()->Dump(dumpStr,filterStr);
			throw;
		}
		return rez;
	}
	static CTableManager<Elem>* CreateFromGenerator(TSCGenerator::IGenerator* _generator)
	{
		//_generator->Generate();
		int len=(int)_generator->GetSample().size();
		int width=(int)sqrt((double)len);
		int height=(int)sqrt((double)len);
		return new Tsc::TableManager::CTableManager<Elem>(&_generator->GetSample().front(),width,height);
	}
};
typedef CTableManagerFactory<double> TSampleManagerFactory;
typedef CTableManagerFactory<Tsc::Formula::CMathExpString<MathCalc::MathString> > TModelManagerFactory;
}//namespace TableManager
}//namespace Tsc