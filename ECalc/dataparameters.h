// DataParameters.h: Interface for data processing
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "ECalcMisc.h"

namespace DataProcessing {
//////////////////////////////////////////////////////////////////////
// Data parameters

	using MathCalc::MathString;
	//template class ECALC_API std::allocator<char>;
	//extern template class ECALC_API std::basic_string<char>;

	struct ECALC_API CDataParameter {
		MathString Name;
		MathString Value;

		bool HasSameName( const MathString& Name2Check ) const;

	// Different Value representations
		MathString ToString() const { return Value; }
		int ToInt() const { return _ttoi( Value.c_str() ); }
		double ToDouble() const {
			TCHAR* unused_end;
			return _tcstod( Value.c_str(), &unused_end );
		}

	// Array functions for names like <array_name>[<index>] or <array_name>[<index1>][<index2>]
		bool IsArrayElement() const;
		MathString GetArrayName() const;
		int GetArrayIndex1() const /*throw( std::exception, CInvalidArrayIndex )*/;
		int GetArrayIndex2() const /*throw( std::exception, CInvalidArrayIndex )*/;
	};

// Dummy operations to perform full template instantiation
	bool operator<(const CDataParameter&, const CDataParameter&);
	bool operator==(const CDataParameter&, const CDataParameter&);
#ifdef USE_PRAGMA
#pragma warning( disable : 4231 ) //disable warnings on extern before template instantiation
#endif
	//struct ECALC_API std::_Container_base;
	EC_EXPIMP_TEMPLATE template class ECALC_API std::allocator< CDataParameter >;
	EC_EXPIMP_TEMPLATE template class ECALC_API std::vector< CDataParameter >;
#ifdef USE_PRAGMA
#pragma warning( default : 4231 )
#endif //USE_PRAGMA
	
	class CDataIterator;
	class CArrayInfo;
	class ECALC_API CDataParameters : public std::vector< CDataParameter > 
	{
	public:
		CDataParameters() {};
		CDataParameter* FindParameter( const MathString& Name );
		void FindArraySize( CArrayInfo& ArrayInfo ) const;
		bool ReadFromData( CDataIterator& it, int nForceCount = 0 );
		bool ReadFromData( const MathString& strSource, int nForceCount = 0 );
	};

//////////////////////////////////////////////////////////////////////
// Data iterators

	class ECALC_API CDataIterator {
	public:
		virtual CDataIterator& operator++() = 0;
		virtual operator bool() = 0;
		virtual const MathString& operator*() = 0;
		virtual const MathString* operator->() = 0;
	};

// One-string data iterator
	class ECALC_API CSimpleIterator : public CDataIterator {
		const MathString& m_Data;
		bool m_bPointsToData;
	public:
		CSimpleIterator( const MathString& Data ): m_Data(Data), m_bPointsToData(true) {}
		virtual CDataIterator& operator++() { m_bPointsToData = false; return *this; }
		virtual operator bool() { return m_bPointsToData; }
		virtual const MathString& operator*() { return m_Data; }
		virtual const MathString* operator->() { return &m_Data; }
	};

// Adapter for STL iterators and pointers
	template< class StdIterator >
	class CStdIterator : public CDataIterator {
		StdIterator m_first;
		StdIterator m_last;
	public:
		CStdIterator( StdIterator First, StdIterator Last ) : m_first(First), m_last(Last) {}
		virtual CDataIterator& operator++() { ++m_first; return *this; }
		virtual operator bool() { return m_first != m_last; }
		virtual const MathString& operator*() { return *m_first; } // m_first.operator*();
		virtual const MathString* operator->() { return &*m_first; } // m_first.operator->();
	};

}
