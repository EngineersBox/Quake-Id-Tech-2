#pragma once

#ifndef QUAKE_ENUM_HPP
#define QUAKE_ENUM_HPP

#include <boost/preprocessor.hpp>
#include <iostream>
#include <map>
#include <string>
#include <streambuf>

#define QUAKE_PP_ENUM_STRING_VALUE_PAIR(r, data, i, elem) \
	BOOST_PP_COMMA_IF(i) { BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, elem)), data::BOOST_PP_TUPLE_ELEM(0, elem) }

#define QUAKE_PP_ENUM_NUMBERED_TOSTRING_CASE(r, data, elem) \
	case data::BOOST_PP_TUPLE_ELEM(0, elem) : os << BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, elem)); break;

#define QUAKE_PP_ENUM_NUMBERED_SEQ_X(s, data, elem) BOOST_PP_TUPLE_ELEM(0, elem) = BOOST_PP_TUPLE_ELEM(1, elem)

#define QUAKE_PP_ENUM_NUMBERED(name, enumerators) \
    enum class name \
	    { \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(QUAKE_PP_ENUM_NUMBERED_SEQ_X, ~, enumerators)) \
	    };\
	\
	inline std::ostream& operator<<(std::ostream& os, name v) \
	{ \
		switch (v) \
		{ \
		BOOST_PP_SEQ_FOR_EACH(\
			QUAKE_PP_ENUM_NUMBERED_TOSTRING_CASE, \
			name, \
			enumerators \
		) \
		default: throw std::exception(); \
		} \
		return os; \
	} \
	\
	inline std::istream& operator>>(std::istream& is, name& type) \
	{ \
		static const std::map<std::string, name> map = { \
			BOOST_PP_SEQ_FOR_EACH_I( \
				QUAKE_PP_ENUM_STRING_VALUE_PAIR, \
				name, \
				enumerators \
			) \
		}; \
		static const std::istreambuf_iterator<char> eos; \
		std::string s(std::istreambuf_iterator<char>(is), eos); \
		auto map_itr = map.find(s); \
		if (map_itr == map.end()) \
		{ \
			throw std::exception(); \
		} \
		return is; \
	}

#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem) \
    case data::elem : os << BOOST_PP_STRINGIZE(elem); break;

#define X_DEFINE_ENUM_STRING_VALUE_PAIR(r, data, i, elem) \
		BOOST_PP_COMMA_IF(i) { BOOST_PP_STRINGIZE(elem), data::elem }

#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators) \
    enum class name \
	{ \
        BOOST_PP_SEQ_ENUM(enumerators) \
	}; \
    \
    inline std::ostream& operator<<(std::ostream& os, name v) \
	{ \
		switch (v) \
		{ \
			BOOST_PP_SEQ_FOR_EACH( \
				X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE, \
				name, \
				enumerators \
			) \
			default: throw std::exception(); \
		} \
		return os; \
	} \
    \
	inline std::istream& operator>>(std::istream& is, name& type) \
	{ \
		static const std::map<std::string, name> map = { \
			BOOST_PP_SEQ_FOR_EACH_I( \
				X_DEFINE_ENUM_STRING_VALUE_PAIR, \
				name, \
				enumerators \
			) \
		}; \
	    static const std::istreambuf_iterator<char> eos; \
		std::string s(std::istreambuf_iterator<char>(is), eos); \
		auto map_itr = map.find(s); \
		if (map_itr == map.end()) \
		{ \
		    throw std::exception(); \
		} \
		return is; \
	}

#endif //QUAKE_ENUM_HPP
