#ifndef LATINO_BASIC_OPERATORKINDS_H
#define LATINO_BASIC_OPERATORKINDS_H

namespace latino {

/// Enumeration specifying the different kinds of C++ overloaded
/// operators.
	enum OverloadedOperatorKind : int {
		OO_None, ///< Not an overloaded operator
#define OVERLOADED_OPERATOR(Name, Spelling, Token, Unary, Binary, MemberOnly)  \
		OO_##Name,
#include "latino/Basic/OperatorKinds.def"
		NUM_OVERLOADED_OPERATORS
	};

/// Retrieve the spelling of the given overloaded operator, without
/// the preceding "operator" keyword.
const char *getOperatorSpelling(OverloadedOperatorKind Operator);

} /* namespace latino */

#endif /* LATINO_BASIC_OPERATORKINDS_H */
