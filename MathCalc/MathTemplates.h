#ifndef MATHTEMPLATES_H
#define MATHTEMPLATES_H

// MathTemplates.h
//
//////////////////////////////////////////////////////////////////////
#include "Number.h"
#include "MathCalcDebug.h"
#include "MathSerialization.h"
//#include "MathExpression.h"
namespace MathCalc
{

//////////////////////////////////////////////////////////////////////
// External dependencies

class CMathParameter;
struct CNumber;

//////////////////////////////////////////////////////////////////////

template< class PtrType, class OwnerPtrType > 
inline PtrType ReleasePointerAndDeleteOwner(PtrType& Pointer, OwnerPtrType pOwner)
{
	PtrType p = Pointer;
	Pointer = NULL;
	delete pOwner;
	pOwner = NULL;
	return p;
}

//////////////////////////////////////////////////////////////////////
// CMathBase, CMathContainerBase, CMathBasesList

template< class ElementClass > class CMathBasesList : public std::list< ElementClass>
{
	typedef std::list< ElementClass> lst;
public:
	CMathBasesList<ElementClass>(){}
	CMathBasesList<ElementClass>(typename lst::iterator _begin,typename lst::iterator _end):
		lst(_begin,_end){}
	//Finds first equal element in list
	typename lst::iterator FindElement(const ElementClass& Element,
			typename lst::iterator StartFrom)
	{
		for (; StartFrom != this->end(); ++StartFrom)
			if (StartFrom->IsEqual(Element) )
				break;
		return StartFrom;
	}
	typename lst::iterator FindElement(const ElementClass& Element)
	{
		return FindElement(Element, this->begin() );
	}
	typename lst::iterator FindSimilarElement(const ElementClass& Element,
			typename lst::iterator StartFrom)
	{
		for (; StartFrom != this->end(); ++StartFrom)
			if (StartFrom->IsSimilar(Element) )
				break;
		return StartFrom;
	}
	typename lst::iterator FindSimilarElement(const ElementClass& Element)
	{
		return FindSimilarElement(Element, this->begin() );
	}
	void move_back(ElementClass& Element)
	{
		lst::insert( this->end(), ElementClass() )->MoveData(Element);
	}
	void push_back(const ElementClass& Element)
	{
		lst::push_back(Element);
	}
	typename lst::iterator push_back_(const ElementClass& Element)
	{
		lst::push_back(Element);
		return --(this->end());
	}
	void push_back(ElementClass& Element, bool bOwnMode)
	{
		bOwnMode ? move_back(Element) : push_back(Element);
	}
	typename lst::iterator push_back_(ElementClass& Element, bool bOwnMode)
	{
		push_back(Element, bOwnMode);
		return --(this->end());
	}
	void push_back(typename lst::iterator First,
			typename lst::iterator Last)
	{
		lst::insert(this->end(), First, Last);
	}
	typename lst::iterator push_back_(typename lst::iterator First,
			typename std::list<ElementClass>::iterator Last, bool bOwnMode)
	{
		return insert_(this->end(), First, Last, bOwnMode); // returns second list start position
	}
	void insert(typename lst::iterator InsertLocation,
			typename lst::iterator First,
			typename lst::iterator Last, bool bOwnMode)
	{
		if (bOwnMode)
		{
			for (; First != Last; ++First)
				lst::insert( InsertLocation, ElementClass() )->SwapData( *First);
		} else
			lst::insert(InsertLocation, First, Last);
	}
	typename lst::iterator insert_(typename lst::iterator InsertLocation, 
			typename lst::iterator First, 
			typename lst::iterator Last, bool bOwnMode)
	{
		bool ReturnBegin = InsertLocation == this->begin();
		typename CMathBasesList<ElementClass>::iterator res = InsertLocation;
		if ( !ReturnBegin)
			--res;
		if (bOwnMode)
		{
			for (; First != Last; ++First)
				lst::insert( InsertLocation, ElementClass() )->SwapData( *First);
		} else
			lst::insert(InsertLocation, First, Last);
		return ReturnBegin ? this->begin() : ++res;
	}
	void Serialize(CMathSerializer& ar);
};

enum MathContainerType
{	ctNumber = 0, ctElement, ctList};

template< class ElementClass > 
class CMathContainerBase
{
public:
	typedef CMathContainerBase< ElementClass> BaseContainerClass;
	typedef typename CMathBasesList< ElementClass>::iterator ElementsIterator;
	MathContainerType Type;
	//union 
	//{
	CNumber* pNumber;
	ElementClass* pElement;
	CMathBasesList<ElementClass>* pElements;
	//};
	CMathContainerBase(MathContainerType ContainerType, bool bCreateItem = false);
	CMathContainerBase(const CNumber& Number);
	CMathContainerBase(const ElementClass& Element);
	explicit CMathContainerBase(ElementClass* ptr) :
		Type(ctElement), pNumber(0), pElement(ptr), pElements(0)
	{
		//IncreaseObjectsCounter();
	}
	~CMathContainerBase();

	// Operations
	bool IsNumber() const
	{
		return Type == ctNumber;
	}
	bool IsElement() const
	{
		return Type == ctElement;
	}
	bool IsList() const
	{
		return Type == ctList;
	}
	bool IsEqual(const CMathContainerBase&) const;
	CNumber BaseEval() const;
	void SwapData(CMathContainerBase&);
	void ConvertToList();
	void TryShrinkList();
	void Serialize(CMathSerializer& ar);

	void MoveData(CMathContainerBase&);
	template< class SubItem > void ExtractDataFromSubItem(SubItem*& pSubItem)
	{
		SubItem* pSubItemCopy = ReleasePointerAndDeleteOwner(pSubItem, pElement);
		MoveData( *pSubItemCopy);
		delete pSubItemCopy;
	}

protected:
	void DestroyData();
	void CopyData(const CMathContainerBase&);
};

//////////////////////////////////////////////////////////////////////
// Templates implementation
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CMathBasesList

//template< class ElementClass > 
//void CMathBasesList<ElementClass>::insert(typename lst::iterator InsertLocation,
//		typename lst::iterator First,typename lst::iterator Last, bool bOwnMode)
//{
//	if (bOwnMode)
//	{
//		for (; First != Last; ++First)
//			lst::insert( InsertLocation, ElementClass() )->SwapData( *First);
//	} else
//		lst::insert(InsertLocation, First, Last);
//}

//template< class ElementClass > 
//typename std::list<ElementClass>::iterator CMathBasesList<ElementClass>::insert_(
//				typename lst::iterator InsertLocation, 
//				typename lst::iterator First, 
//				typename lst::iterator Last, bool bOwnMode)
//{
//	bool ReturnBegin = InsertLocation == this->begin();
//	typename CMathBasesList<ElementClass>::iterator res = InsertLocation;
//	if ( !ReturnBegin)
//		--res;
//	if (bOwnMode)
//	{
//		for (; First != Last; ++First)
//			lst::insert( InsertLocation, ElementClass() )->SwapData( *First);
//	} else
//		lst::insert(InsertLocation, First, Last);
//	return ReturnBegin ? this->begin() : ++res;
//}

template< class ElementClass > 
void CMathBasesList<ElementClass>::Serialize(CMathSerializer& ar)
{
	if (ar.IsStoring())
	{
		ar.WriteCount( (int)this->size() );
		for (typename CMathBasesList<ElementClass>::iterator it = this->begin(); it != this->end(); ++it)
			it->Serialize(ar);
	} else
	{
		this->clear();
		DWORD nNewCount = ar.ReadCount();
		while (nNewCount--)
			push_back_( ElementClass() )->Serialize(ar);
	}
}

//////////////////////////////////////////////////////////////////////
// CMathContainerBase

template< class ElementClass > CMathContainerBase< ElementClass >::CMathContainerBase(
		MathContainerType ContainerType, bool bCreateItem/*=false*/)
{
	pNumber = NULL;
	pElement = NULL;
	pElements = NULL;
	switch (Type = ContainerType)
	{
	case ctNumber:
		pNumber = (bCreateItem ? new CNumber : NULL);
		break;
	case ctElement:
		pElement = (bCreateItem ? new ElementClass : NULL);
		break;
	case ctList:
		pElements = (bCreateItem ? new CMathBasesList<ElementClass> : NULL);
		break;
	}
	IncreaseObjectsCounter();
}

template< class ElementClass > 
CMathContainerBase< ElementClass >::CMathContainerBase(const CNumber& Number)
{
	Type = ctNumber;
	pNumber = new CNumber(Number);
	pElement = NULL;
	pElements = NULL;
	IncreaseObjectsCounter();
}

template< class ElementClass > CMathContainerBase< ElementClass >::CMathContainerBase(
		const ElementClass& Element)
{
	Type = ctElement;
	pNumber = NULL;
	pElement = new ElementClass(Element);
	pElements = NULL;
	IncreaseObjectsCounter();
}

template< class ElementClass > 
inline void CMathContainerBase< ElementClass >::DestroyData()
{
	switch (Type)
	{
	case ctNumber:
		if (pNumber != NULL)
		{
			delete pNumber;
		}
		break;
	case ctElement:
		if (pElement != NULL)
		{
			delete pElement;
		}
		break;
	case ctList:
		if (pElements != NULL)
		{
			delete pElements;
		}
		break;
	}
	pNumber = NULL;
	pElement = NULL;
	pElements = NULL;
}

template< class ElementClass > 
CMathContainerBase< ElementClass >::~CMathContainerBase()
{
	DestroyData();
	DecreaseObjectsCounter();
}

template< class ElementClass > 
bool CMathContainerBase< ElementClass >::IsEqual(
		const CMathContainerBase& Operand) const
{
	if (Type != Operand.Type)
		return false;

	switch (Type)
	{
	case ctNumber:
		return *pNumber == *Operand.pNumber;
	case ctElement:
		return pElement->IsEqual(*Operand.pElement);
	case ctList:
	{
		if (pElements->size() != Operand.pElements->size() )
			return false;

		for (ElementsIterator it = pElements->begin(); it != pElements->end(); ++it)
			if (Operand.pElements->FindElement( *it) == Operand.pElements->end() )
				return false;
		return true;
	}
	}
	return false;
}

template< class ElementClass > 
inline CNumber CMathContainerBase< ElementClass >::BaseEval() const
{
	switch (Type)
	{
	case ctNumber:
		return *pNumber;
	case ctElement:
		return pElement->Eval();
	default:
		return 0;
	}
}

template< class ElementClass > 
inline void CMathContainerBase< ElementClass >::CopyData(const CMathContainerBase& src)
{
	switch (Type = src.Type)
	{
	case ctNumber:
		pNumber = (src.pNumber ? new CNumber(*src.pNumber) : NULL);
		break;
	case ctElement:
		pElement = (src.pElement ? new ElementClass(*src.pElement) : NULL);
		break;
	case ctList:
		if (src.pElements)
		{
			pElements = new CMathBasesList<ElementClass>(src.pElements->begin(), src.pElements->end());
//			pElements->push_back(src.pElements->begin(), src.pElements->end() );
		} 
		else
			pElements = NULL;
		break;
	}
}

template< class ElementClass > 
inline void CMathContainerBase< ElementClass >::MoveData(CMathContainerBase& src)
{
	switch (Type = src.Type)
	{
	case ctNumber:
		pNumber = src.pNumber;
		break;
	case ctElement:
		pElement = src.pElement;
		break;
	case ctList:
		pElements = src.pElements;
		break;
	}
	src.pNumber = NULL;
	src.pElement = NULL;
	src.pElements = NULL;
}

template< class ElementClass > 
void CMathContainerBase< ElementClass >::SwapData(CMathContainerBase& SwapObject)
{
	CMathContainerBase Temp(Type);
	switch (Type)
	{
	case ctNumber:
		Temp.pNumber = pNumber;
		break;
	case ctElement:
		Temp.pElement = pElement;
		break;
	case ctList:
		Temp.pElements = pElements;
		break;
	} // now Temp is a copy of [this] class
	switch (Type = SwapObject.Type)
	{
	case ctNumber:
		pNumber = SwapObject.pNumber;
		break;
	case ctElement:
		pElement = SwapObject.pElement;
		break;
	case ctList:
		pElements = SwapObject.pElements;
		break;
	} // now [this] is a copy of SwapObject
	switch (SwapObject.Type = Temp.Type)
	{
	case ctNumber:
		SwapObject.pNumber = Temp.pNumber;
		break;
	case ctElement:
		SwapObject.pElement = Temp.pElement;
		break;
	case ctList:
		SwapObject.pElements = Temp.pElements;
		break;
	} // now SwapObject is a copy of Temp class
	// release Temp's pointers if any
	switch (Temp.Type)
	{
	case ctNumber:
		Temp.pNumber = NULL;
		break;
	case ctElement:
		Temp.pElement = NULL;
		break;
	case ctList:
		Temp.pElements = NULL;
		break;
	}
}

template< class ElementClass > 
void CMathContainerBase< ElementClass >::ConvertToList()
{
	if (Type == ctNumber)
	{
//		CNumber OldNumber(*pNumber);
		Type = ctList;
		pElements = new CMathBasesList<ElementClass>;
		pElements->push_back(ElementClass(*pNumber));
		delete pNumber;
		pNumber = NULL;
	} 
	else if (Type == ctElement)
	{
//		ElementClass OldElement;
//		OldElement.SwapData(*pElement);
		Type = ctList;
		pElements = new CMathBasesList<ElementClass>;
		pElements->push_back(*pElement, true);
		delete pElement;
		pElement = NULL;
	}
}

template< class ElementClass > 
void CMathContainerBase< ElementClass >::TryShrinkList()
{
	if (Type != ctList)
		return;
	if (pElements->size() == 1)
	{
		if (pElements->front().IsNumber() )
		{
			pNumber = ReleasePointerAndDeleteOwner(pElements->front().pNumber, pElements);
			Type = ctNumber;
		} 
		else
		{
			ElementClass* pListElement = new ElementClass;
			pListElement->SwapData(pElements->front() );
			delete pElements;
			pElements = NULL;
			Type = ctElement;
			pElement = pListElement;
		}
	}
}

template< class ElementClass > 
void CMathContainerBase< ElementClass >::Serialize(CMathSerializer& ar)
{
	if (ar.IsStoring() )
	{
		WriteAs_int(ar, Type);
		switch (Type)
		{
		case ctNumber:
			pNumber->Serialize(ar);
			break;
		case ctElement:
			pElement->Serialize(ar);
			break;
		case ctList:
			pElements->Serialize(ar);
			break;
		}
	} else
	{
		ReadAs_int(ar, Type);
		pNumber = NULL;
		pElement = NULL;
		pElements = NULL;
		switch (Type)
		{
		case ctNumber:
			pNumber = new CNumber;
			pNumber->Serialize(ar);
			break;
		case ctElement:
			pElement = new ElementClass;
			pElement->Serialize(ar);
			break;
		case ctList:
			pElements = new CMathBasesList<ElementClass>;
			pElements->Serialize(ar);
			break;
		}
	}
}

} // namespace MathCalc
#endif //MATHTEMPLATES_H
