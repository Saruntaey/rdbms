#include <assert.h>
#include "sql_enum.h"
#include "math_exp_enum.h"

int app_to_mexpr_enum_converter(int token_code) {
	switch (token_code) {
		case SQL_IDENTIFIER_IDENTIFIER:
		case SQL_IDENTIFIER: return MATH_IDENTIFIER;
		case SQL_BRACKET_START: return MATH_BRACKET_START;
		case SQL_BRACKET_END: return MATH_BRACKET_END;
		case SQL_LESS_THAN_EQ: return MATH_LESS_THAN_EQ;
		case SQL_LESS_THAN: return MATH_LESS_THAN;
		case SQL_GREATER_THAN: return MATH_GREATER_THAN;
		case SQL_GREATER_THAN_EQ: return MATH_GREATER_THAN_EQ;
		case SQL_EQ: return MATH_EQ;
		case SQL_NOT_EQ: return MATH_NOT_EQ;
		case SQL_AND: return MATH_AND;
		case SQL_OR: return MATH_OR;
		case SQL_MATH_MUL: return MATH_MUL;
		case SQL_MATH_PLUS: return MATH_PLUS;
		case SQL_MATH_MINUS: return MATH_MINUS;
		case SQL_MATH_DIV: return MATH_DIV;
		case SQL_MATH_SQRT: return MATH_SQRT;
		case SQL_MATH_SQR: return MATH_SQR;
		case SQL_MATH_MAX: return MATH_MAX;
		case SQL_MATH_MIN: return MATH_MIN;
		case SQL_MATH_SIN: return MATH_SIN;
		case SQL_MATH_COS: return MATH_COS;
		case SQL_MATH_POW: return MATH_POW;
		case SQL_COMMA: return MATH_COMMA;
		case SQL_INTEGER_VALUE: return MATH_INTEGER_VALUE;
		case SQL_DOUBLE_VALUE: return MATH_DOUBLE_VALUE;
		case SQL_STRING_VALUE: return MATH_STRING_VALUE;
		default: return -1;
	}
}
