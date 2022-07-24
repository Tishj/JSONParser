#pragma once

#include <cstdint>
#include <cstddef>

#include "common/string.hpp"
#include "element_type.hpp"

namespace json {

class ParsedJSON;

enum class ParseTokens {
	CBRACE_OPEN,
	CBRACE_CLOSE,
	DQUOTE_OPEN,
	DQUOTE_CLOSE,
	NUMBER,
	BOOLEAN,
	COLON,
	ARRAY_OPEN,
	ARRAY_CLOSE,
	COMMA,
	NULL_TYPE
};

enum class ParseState {
	OUT,	//! Initial state
	WORD,	//! Name of a JSON item
	VALUE,	//! Value of a JSON item
	IN,		//! Between Name and Value
	TAIL	//! Last portion towards the closing
};

enum class StateResult {
	CONTINUE,
	DONE,
	ERROR
};

class Parser {
public:
	Parser(string::iterator& it, const string::iterator& end, ElementType parent = ElementType::JSON_OBJECT) :
		it(it), end(end), parent(parent) {
			current_state = ParseState::OUT;
		}
public:
	ParsedJSON* Produce();
private:
	void	ConvertValue();
	void	UpdateState(ParseState state);

	StateResult StateOut(char c);
	StateResult StateWord(char c);
	StateResult StateValue(char c);
	StateResult StateIn(char c);

	StateResult HandleState(char c);
private:
	string::iterator& it;
	const string::iterator& end;
	ElementType	parent;

	ParseState	previous_state;
	ParseState	current_state;
	ParsedJSON	*product;
	string		temporary;
};

} //namespace json
