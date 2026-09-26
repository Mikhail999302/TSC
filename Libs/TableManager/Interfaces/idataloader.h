#ifndef TABLEMANAGER_INTERFACES_IDATALOADER_H
#define TABLEMANAGER_INTERFACES_IDATALOADER_H
#include <C:\diploma\2012\TSC\Libs\StdInterfaces\Entry.h>
#include "..\Headers\TableManager.h"
namespace Tsc {
namespace TableManager {
template<class Elem> class IDataLoader: public Tsc::StdInterfaces::IClone<IDataLoader<Elem>*>
{
public:
	virtual CTableManager<Elem>* CreateTableManager()=0;
	virtual ~IDataLoader(){}
};
}//namespace TableManager
}//namespace Tsc
#endif //TABLEMANAGER_INTERFACES_IDATALOADER_H
