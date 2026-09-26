#ifndef STDINTERFACES_ICLONE_H
#define STDINTERFACES_ICLONE_H
namespace Tsc {
namespace StdInterfaces {
template <typename ClassPtr>
class IClone
{
public:
	virtual ClassPtr Clone()=0;
};
}  // namespace StdInterfaces
}  // namespace Tsc
#endif /*STDINTERFACES_ICLONE_H*/
