// trivial_json_printer.hpp implements basic functionality for printing JSON formated strings
// written to support VC 2013 
#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <stack>
#include <vector>

// Prototypes used in the templates (so it works on gcc)
void StartObject(std::stringstream &ss);
void EndObject(std::stringstream &ss);
void StartArray(std::stringstream &ss);
void EndArray(std::stringstream &ss);

// Templates

// WriteValue template is the base template for using the 'trivial_json_printer.hpp' headers. 
// There are prepared templates for booleans, numbers, strings (std::string) and vectors of T that implement WriteValue
// For a given struct or class (typename T) specialize this template. Your class is most likely going to be of JSON Object type when parsed.
// Example:
//      struct MyStruct {
//          int A;
//          std::string B;
//      };
//
//      template <>
//      void WriteValue<MyStruct>(std::stringstream &ss, MyStruct v) {
//          StartObject(ss);
//          AddJSONPropertyAndValue(ss, "A", v.A);
//          AddJSONPropertyAndValue(ss, "B", v.B, false); // FALSE DO NOT TERMINATE WITH COMMA
//          EndObject(ss);
//		}
// This will add support for your custom types. If your type composes or "has-a" another type implement WriteValue template for that type and as well.
// For all types you want to serialize implement WriteValue template specialization.
template <typename T>
void WriteValue(std::stringstream &ss, T value) {
	ss << value;
}

template <typename T>
void WriteValue(std::stringstream &ss, std::vector<T> vec) {
	StartArray(ss);
	if (vec.size() > 0) {
		const int lastIndex = vec.size() - 1;
		for (int i = 0; i < lastIndex; i++) {
			WriteValue(ss, vec[i]);
			ss << ",";
		}
		WriteValue(ss, vec[lastIndex]);
	}
	EndArray(ss);
}

template <>
void WriteValue<bool>(std::stringstream &ss, bool value) {
	if (value) {
		ss << "true";
	}
	else {
		ss << "false";
	}
}

template <>
void WriteValue<std::string>(std::stringstream &ss, std::string value) {
	ss << "\"" << value.c_str() << "\"";
}

template <>
void WriteValue<const char *>(std::stringstream &ss, const char * value) {
	ss << "\"" << value << "\"";
}

template <typename T>
void WriteNameValue(std::stringstream &ss, std::string name, T value) {
	ss << "\"" << name.c_str() << "\":";
	WriteValue(ss, value);
}

template <typename T>
void AddJSONPropertyAndValue(std::stringstream &ss, std::string name, T value, bool comma = true) {
	WriteNameValue(ss, name, value);
	if (comma) ss << ",";
}

// implementation
void StartObject(std::stringstream &ss) {
	ss << "{";
}

void EndObject(std::stringstream &ss) {
	ss << "}";
}

void StartArray(std::stringstream &ss) {
	ss << "[";
}

void EndArray(std::stringstream &ss) {
	ss << "]";
}

// pretty print function will format jsonStr to human readable json.
// formatting is trivialy implemented and might not work in all cases
std::string getPrettyString(std::string jsonStr, int indentBy = 2) {
	std::stringstream ss;
	std::stack<char> tags;
	bool nameOrValue = false;
	char lastC = '\0';
	for (int i = 0; i < jsonStr.size(); i++) {
		const auto c = jsonStr[i];

		if (lastC == '\"') {
			nameOrValue = !nameOrValue;
		}

		if (lastC == '[' && c != ']') {
			tags.push(lastC);
			ss << std::endl;
			ss << std::setw(indentBy * tags.size());
		}
		if (c == ']' && !tags.empty() && tags.top() == '[') {
			tags.pop();
			ss << std::endl;
			ss << std::setw(indentBy * tags.size());
		}

		if (lastC == '{' && c != '}') {
			tags.push(lastC);
			ss << std::endl;
			ss << std::setw(indentBy * tags.size());
		}
		if (lastC != '{' && c == '}' && !tags.empty() && tags.top() == '{') {
			tags.pop();
			ss << std::endl;
			ss << std::setw(indentBy * tags.size());
		}
		if (lastC == ',' && !nameOrValue) {
			ss << std::endl;
			ss << std::setw(indentBy * tags.size());
		}
		if (lastC == ':') {
			ss << ' ';
		}
		// append
		ss << c;
		lastC = c;
	}
	return ss.str();
}
