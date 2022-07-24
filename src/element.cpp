#include "element.hpp"
#include <assert.h>

namespace json {

const ElementType& Element::Type() const
{
	return this->type;
}

Element Element::STRING(ParsedJSON json)
{
	auto element = Element();
	element.type = json.GetType();
	assert(element.type == ElementType::JSON_STRING);
	element.name = json.GetName();
	auto& val = json.GetValue();
	element.value_ = val;
	return move(element);
}

Element Element::BOOL(ParsedJSON json)
{
	auto element = Element();
	element.type = json.GetType();
	assert(element.type == ElementType::JSON_BOOL);
	element.name = json.GetName();
	auto& val = json.GetValue();
	element.value_ = val;
	return move(element);
}

static void FillChildrenVector(vector<unique_ptr<Element>>& children, ParsedJSON* elements) {
	EmbeddedList* list = elements->GetList();
	while (list) {
		auto* parsed = (ParsedJSON*)list->holder;
		children.push_back(move(unique_ptr<Element>(parsed->Finalize())));
		list = list->Next();
		delete(parsed);
	}
}

Element Element::ARRAY(ParsedJSON json)
{
	auto element = Element();
	element.type = json.GetType();
	assert(element.type == ElementType::JSON_ARRAY);
	element.name = json.GetName();
	auto& val = json.GetValue();
	FillChildrenVector(element.children, (ParsedJSON*)val.pointer);
	return move(element);
}

Element Element::GROUP(ParsedJSON json)
{
	auto element = Element();
	element.type = json.GetType();
	assert(element.type == ElementType::JSON_OBJECT);
	element.name = json.GetName();
	auto& val = json.GetValue();
	FillChildrenVector(element.children, (ParsedJSON*)val.pointer);
	return move(element);
}

Element Element::NULL_ELEM(ParsedJSON json)
{
	auto element = Element();
	element.type = json.GetType();
	assert(element.type == ElementType::JSON_NULL);
	element.name = json.GetName();
	auto& val = json.GetValue();
	element.value_ = val;
	return move(element);
}

Element Element::NUMBER(ParsedJSON json)
{
	auto element = Element();
	element.type = json.GetType();
	assert(element.type == ElementType::JSON_INTEGER);
	element.name = json.GetName();
	auto& val = json.GetValue();
	element.value_ = val;
	return move(element);
}

} //namespace json
