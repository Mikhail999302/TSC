#ifndef IGENERATOR_H
#define IGENERATOR_H
#include <vector>
#include "Exports.h"
namespace TSCGenerator
{
	typedef std::vector<double> doubles;
class TSCGEN_DLLENTRY IGenerator
{
public:
	  // Generate pij using current settings but new size
	  virtual void Generate(int size_)=0;
	  virtual void SetSize(int _size)=0;
	  // Generate pij using current settings
	  virtual void Generate(void)=0;
	  //get output
	  virtual const doubles& GetSample(void)=0;
	  virtual void SaveSample(const char * fileName,bool isTtranspose=false)=0;
	  virtual ~IGenerator(){}
};
}//namespace TSCGenerator
#endif