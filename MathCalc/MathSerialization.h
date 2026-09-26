#ifndef MATHSERIALIZATION_H
#define MATHSERIALIZATION_H

// MathSerialization.h
//////////////////////////////////////////////////////////////////////

namespace MathCalc {

	class MATHCALC_API CMathSerializer {
	public:
	// Pure interface
		virtual bool IsStoring() = 0;
		virtual bool IsLoading() = 0;
		virtual CMathSerializer& operator>>( char& ) = 0;
		virtual CMathSerializer& operator>>( int& ) = 0;
		virtual CMathSerializer& operator>>( UINT& ) = 0;
		virtual CMathSerializer& operator>>( WORD& ) = 0;
		virtual CMathSerializer& operator>>( DWORD& ) = 0;
		virtual CMathSerializer& operator>>( double& ) = 0;
		virtual CMathSerializer& operator<<( const char& ) = 0;
		virtual CMathSerializer& operator<<( const int& ) = 0;
		virtual CMathSerializer& operator<<( const UINT& ) = 0;
		virtual CMathSerializer& operator<<( const WORD& ) = 0;
		virtual CMathSerializer& operator<<( const DWORD& ) = 0;
		virtual CMathSerializer& operator<<( const double& ) = 0;
	
	// Ready-to-use operations
		CMathSerializer& operator>>( bool& b ) { 
			char temp;
			*this >> temp;
			b = temp != 0;
			return *this;
		}
		CMathSerializer& operator<<( const bool& b ) { return *this << char(b); }
		DWORD ReadCount();
		void WriteCount( DWORD dwCount );
		void ReadString( MathString& );
		void WriteString( const MathString& );
	};

	template< class T > void ReadAs_int( CMathSerializer& ar, T& t ) { 
		int temp;
		ar >> temp;
		t = T(temp);
	}
	template< class T > void WriteAs_int( CMathSerializer& ar, const T& t ) { ar << (int)t; }

	template< class ElementType, class ContainerType, class ElementSerializer >
	void SerializeContainerEx( CMathSerializer& ar, ContainerType& container, ElementSerializer serializer ) {
		if ( ar.IsStoring() ) {
			ar.WriteCount( (DWORD)container.size() );
			for (typename ContainerType::iterator it = container.begin(); it != container.end(); ++it )
				serializer( ar, *it );
		}
		else {
			container.clear();
			DWORD nNewCount = ar.ReadCount();
			while (nNewCount--) {
				container.push_back( ElementType() );
				serializer( ar, container.back() );
			}
		}
	}

	template< class ElementType > 
	struct CElementSerializer {
		void operator() ( CMathSerializer& ar, ElementType& element ) { element.Serialize( ar ); }
	};

	template< class ElementType, class ContainerType >
	inline void SerializeContainer( CMathSerializer& ar, ContainerType& container ) {
		SerializeContainerEx< ElementType, ContainerType, CElementSerializer< ElementType > >
			( ar, container, CElementSerializer< ElementType >() ); 
	}

	template< class ElementType, class ContainerType > 
	struct CContainerSerializer {
		void operator() ( CMathSerializer& ar, ContainerType& container ) 
			{ SerializeContainer< ElementType >( ar, container ); }
	};

} // namespace MathCalc
#endif //MATHSERIALIZATION_H
