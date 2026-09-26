#ifndef MATHSTRINGS_H
#define MATHSTRINGS_H

// MathStrings.h: interface for the CMathStringParser class.
//
//////////////////////////////////////////////////////////////////////

namespace MathCalc {

/* Приоритет операций в порядке убывания */
/*	^ - степень, [] - производная
	*, /
	+, -
*/

class CMathStringParser  
{
public:
	enum TokenType { DELIMITER=1, PARAMETER, NUMBER, FUNCTION };

	// constructs expression from a string
	bool ParseString( const MathString& pszExpression, CMathExpression& result );

protected:
	void ParseStringLevel1( CMathExpression& result );
	void ParseStringLevel2( CMathExpression& result );
	void ParseStringLevel3( CMathExpression& result );
	void ParseStringLevel4( CMathExpression& result );
	void ParseStringLevel5( CMathExpression& result );
	void ParseStringLevel6( CMathExpression& result );
	double GetNumberFromToken();
	CMathParameter GetParameterFromToken();
	CMathFunction* GetFunctionFromToken();
	TokenType GetToken();

protected:
	TCHAR token[20];
	TokenType token_type;
	LPCTSTR expr;
};

} // namespace MathCalc
#endif //MATHSTRINGS_H
