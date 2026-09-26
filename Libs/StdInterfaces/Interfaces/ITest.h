#ifndef ITEST_H_
#define ITEST_H_
namespace Tsc {
namespace StdInterfaces {
class ITest
{
public:
	virtual bool Test()=0;
	virtual ~ITest(){};
};
}// namespace StdInterfaces
}// namespace Tsc
#endif /*ITEST_H_*/
