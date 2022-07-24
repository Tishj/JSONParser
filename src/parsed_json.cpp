#include "parsed_json.hpp"
#include "element.hpp"

namespace json {

static void DeletePointerObject(ElementType type, uintptr_t ptr) {
	switch (type) {
		case ElementType::JSON_STRING: {
			auto pointer = (string*)ptr;
			delete(pointer);
			break;
		}
		case ElementType::JSON_ARRAY:
		case ElementType::JSON_OBJECT: {
			auto pointer = (ParsedJSON*)ptr;
			delete(pointer);
			break;
		}
		default: {
			break;
		}
	}
}

ParsedJSON::~ParsedJSON() {
	DeletePointerObject(type, this->value_.pointer);
}

void ParsedJSON::SetName(string name)
{
	this->name = name;
}

void ParsedJSON::SetPtrValue(uintptr_t ptr) {
	DeletePointerObject(type, this->value_.pointer);
	this->value_.pointer = ptr;
}

void ParsedJSON::SetType(ElementType type)
{
	this->type = type;
}

void ParsedJSON::SetValueNumber(int64_t value)
{
	this->value_.bigint = value;
}

void ParsedJSON::SetValueNULL()
{
	this->value_.pointer = 0;
}

void ParsedJSON::SetValueString(string value)
{
	SetPtrValue((uintptr_t)new string(value));
}

void ParsedJSON::SetValueBoolean(bool value)
{
	this->value_.boolean = value;
}

void ParsedJSON::AddChild(ParsedJSON* child)
{
	if (!value_.pointer) {
		value_.pointer = (uintptr_t)child;
	}
	else {
		auto chain = (ParsedJSON*)value_.pointer;
		if (type == ElementType::JSON_ARRAY && chain->type != child->type) {
			throw std::runtime_error("JSON array malformed");
		}
		chain->list.AddBack(&child->list);
	}
}

static element_create_fun_t GetFinalizeFunction(ElementType type) {
	switch (type) {
	case ElementType::JSON_BOOL: return Element::BOOL;
	case ElementType::JSON_INTEGER: return Element::NUMBER;
	case ElementType::JSON_NULL: return Element::NULL_ELEM;
	case ElementType::JSON_STRING: return Element::STRING;
	case ElementType::JSON_OBJECT: return Element::GROUP;
	default:
		throw std::runtime_error("GetFinalizeFunction - type not implemented");
	}
}

//! Returns ownership of this memory
Element* ParsedJSON::Finalize()
{
	element_create_fun_t create = GetFinalizeFunction(this->type);
	return new Element(move(create(move(*this))));
}

} //namespace json
