#include "parser.hpp"
#include "parsed_json.hpp"
#include <stdexcept>
#include <assert.h>

namespace json
{

static bool IsWhitespace(char c) {
	return c == ' ' || c == '\t' || c == '\n';
}

void Parser::UpdateState(ParseState state)
{
	this->previous_state = current_state;
	current_state = state;
}

StateResult	Parser::StateOut(char c) {
	//! Still out
	if (IsWhitespace(c)) {
		return StateResult::CONTINUE;
	}
	//! Name encountered
	if (c == '"') {
		//! Array can only have unnamed elements
		if (parent == ElementType::JSON_ARRAY) {
			return StateResult::ERROR;
		}
		UpdateState(ParseState::WORD);
		product = new ParsedJSON();
		return StateResult::CONTINUE;
	}
	//! Only array can have unnamed elements
	if (parent != ElementType::JSON_ARRAY) {
		return StateResult::ERROR;
	}
	if (c == '{') {
		product = new ParsedJSON();
		product->SetType(ElementType::JSON_OBJECT);
	}
	else if (c == '[') {
		product = new ParsedJSON();
		product->SetType(ElementType::JSON_ARRAY);
	}
	else {
		return StateResult::ERROR;
	}
	UpdateState(ParseState::VALUE);
	return StateResult::CONTINUE;
}

StateResult Parser::StateWord(char c)
{
	assert(product);
	//! name finished
	if (c == '"') {
		product->SetName(temporary);
		temporary = "";
		UpdateState(ParseState::OUT);
		return StateResult::CONTINUE;
	}
	temporary += c;
	return StateResult::CONTINUE;
}

void Parser::ConvertValue() {
	if (temporary == "false" || temporary == "true") {
		product->SetType(ElementType::JSON_BOOL);
		product->SetValueBoolean(temporary == "true");
	}
	else if (temporary == "null") {
		product->SetType(ElementType::JSON_NULL);
		product->SetValueNULL();
	}
	else if (temporary.find_first_not_of("0123456789") == string::npos) {
		product->SetType(ElementType::JSON_INTEGER);
		product->SetValueNumber(stoi(temporary));
	}
	else {
		throw std::runtime_error("Could not convert value '" + temporary + "'");
	}
}

StateResult Parser::StateValue(char c)
{
	assert(product);
	//No change yet
	if (IsWhitespace(c)) {
		if (previous_state != ParseState::VALUE) {
			return StateResult::DONE;
		}
		return StateResult::CONTINUE;
	}
	auto product_type = product->GetType();
	switch (product_type) {
		case ElementType::JSON_ARRAY:
		case ElementType::JSON_OBJECT: {
			if (c == ',') {
				break;
			}
			if (product_type == ElementType::JSON_ARRAY && c == ']') {
				return StateResult::DONE;
			}
			if (product_type == ElementType::JSON_OBJECT && c == '}') {
				return StateResult::DONE;
			}
			Parser parser(it, end, product_type);
			product->AddChild(parser.Produce());
			break;
		}
		case ElementType::JSON_STRING: {
			if (c == '"') {
				product->SetValueString(temporary);
				temporary = "";
				return StateResult::DONE;
			}
			temporary += c;
			break;
		}
		default: {
			if (c == ',') {
				ConvertValue();
				return StateResult::DONE;
			}
			if (c == '"') {
				if (!temporary.empty()) {
					return StateResult::ERROR;
				}
				product->SetType(ElementType::JSON_STRING);
			}
			else {
				temporary += c;
			}
			break;
		}
	}
	return StateResult::CONTINUE;
}

StateResult Parser::StateIn(char c)
{
	if (IsWhitespace(c)) {
		return StateResult::CONTINUE;
	}
	if (c == ':') {
		UpdateState(ParseState::VALUE);
		return StateResult::CONTINUE;
	}
	return StateResult::ERROR;
}

StateResult	Parser::HandleState(const char c) {
	switch (current_state) {
	case ParseState::OUT: return StateOut(c);
	case ParseState::IN: return StateIn(c);
	case ParseState::VALUE: return StateValue(c);
	case ParseState::WORD: return StateWord(c);
	default: throw std::runtime_error("HandleState - action for state not defined");
	}
}

ParsedJSON* Parser::Produce()
{
	for (; it != end; it++) {
		char c = *it;
		auto res = HandleState(c);
		switch (res) {
			case StateResult::CONTINUE: {
				break;
			}
			case StateResult::ERROR: throw std::runtime_error("Failed to parse");
			case StateResult::DONE: return product;
		}
	}
	return nullptr;
}

} //namespace json
