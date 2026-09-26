template<typename Elem>
int Tsc::TableManager::CPlainTextLoader<Elem>::parseLineInElem(const std::string&_from,std::vector<Elem>&_to)
{
	Elem db;
	int idx=0;
	std::istringstream buf(_from);

	while(!buf.eof()&&!buf.fail())
	{
		buf>>db;
		std::cout<<db<<" ";
		++idx;
		_to.push_back(db);
	}
	std::cout<<std::endl;
	return idx;
}

template<typename Elem>
Tsc::TableManager::CTableManager<Elem>* Tsc::TableManager::CPlainTextLoader<Elem>::CreateTableManager()
{
	std::ifstream fin(mFileName.c_str());
	std::string temp;
	int width=0,height=0;
	std::vector<Elem> data;
	char tempchar[256];
	while(fin.getline(tempchar,256))
	{
		temp=tempchar;
		++height;
		int tmp=parseLineInElem(temp,data);
		if(tmp>width)width=tmp;
	}
	CTableManager<Elem> *rez=new CTableManager<Elem>(/*tempdata*/&data.front(),width,height);
	return rez;
}

template<typename Elem>
Tsc::TableManager::CPlainTextLoader<Elem>::~CPlainTextLoader(void)
{
}

template<typename Elem>
Tsc::TableManager::IDataLoader<Elem>* Tsc::TableManager::CPlainTextLoader<Elem>::Clone()
{
	return new CPlainTextLoader(mFileName.c_str());
}
