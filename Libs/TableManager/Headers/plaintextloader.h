#ifndef TABLEMANAGER_HEADERS_PLAINTEXTLOADER_H
#define TABLEMANAGER_HEADERS_PLAINTEXTLOADER_H

#include <vector>
#include <fstream>
#include <sstream>
#include "..\Interfaces\IDataLoader.h"
#include "TableManager.h"

namespace Tsc
{
namespace TableManager
{
template <typename Elem>
class CPlainTextLoader :public IDataLoader<Elem>
{
	std::string mFileName;
private:
	int parseLineInElem(const std::string&_from,std::vector<Elem>&_to);
public:
	CPlainTextLoader(const char* _fileName):mFileName(_fileName){}
	virtual /*implement*/ CTableManager<Elem>* CreateTableManager();
	virtual ~CPlainTextLoader(void);
	virtual /*override*/ IDataLoader<Elem>* Clone();
};
}//namespace TableManager
}//namespace Tsc
#include "..\Implementation\PlainTextLoader.hpp"
#endif //TABLEMANAGER_HEADERS_PLAINTEXTLOADER_H