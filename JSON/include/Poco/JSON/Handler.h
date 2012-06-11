//
// Handler.h
//
// $Id$
//
// Library: JSON
// Package: JSON
// Module:  Handler
//
// Definition of the Handler class.
//
// Copyright (c) 2012, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, W#if defined(POCO_HAVE_INT64)HETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

#ifndef JSON_Handler_INCLUDED
#define JSON_Handler_INCLUDED


#include "Poco/DynamicAny.h"
#include "Poco/JSON/JSON.h"

namespace Poco
{
namespace JSON
{

class JSON_API Handler
{
public:


	virtual void startObject() = 0;
		/// The parser has read a {, meaning a new object will be read


	virtual void endObject() = 0;
		/// The parser has read a }, meaning the object is read


	virtual void startArray() = 0;
		/// The parser has read a [, meaning a new array will be read


	virtual void endArray() = 0;
		/// The parser has read a ], meaning the array is read


	virtual void key(const std::string& k) = 0;
		/// A key of an object is read


	virtual void null() = 0;
		/// A null value is read


	virtual void value(int v) = 0;
		/// An integer value is read
		

#if defined(POCO_HAVE_INT64)
	virtual void value(Int64 v) = 0;
		/// A 64-bit integer value is read
#endif


	virtual void value(const std::string& value) = 0;
		/// A string value is read.


	virtual void value(double d) = 0;
		/// A double value is read


	virtual void value(bool b) = 0;
		/// A boolean value is read


protected:

	virtual ~Handler();
		/// Destructor

private:
};


}} // Namespace Poco::JSON

#endif // JSON_Handler_INCLUDED
