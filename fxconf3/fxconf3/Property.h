#pragma once

// An elaborated implementation of the simple but elegant 
// property scheme suggested by Charles Cafrelli.
// Any object which wishes to expose any variables it has
// can do so by registering their name as a string.  This
// makes a bridge between the name of the variable at
// compile time (which is obviously gone once compiled)
// and a run-time queryable format of that format.
// Perfect for writing editors etc which pop up editing
// boxes for any possible object without hardcoded
// 1:1 matching between the editor and the core object.
//
// Bert Peers rat@larian.com (c) 2000 Larian Studios
// 01/02/2000 Created
// 02/02/2000 Wrote an article about it :
//				http://www.larian.com/rat/objectproperty.html
//
// Example of how to use it :
//
// class CMyStuff : public CPropertyList
// {
//		long	SomeVar;
//		MyStuff () { static std::string SomeVar_S ("SomeVar"); Register (SomeVar_S, &SomeVar);
// }
//
// CProperty Prop;
// CMyStuff Stuff;
// Prop = Stuff.PropertyList ["SomeVar"];
// if (Prop.Type != PropLong) Error ("Expected a long"); else *(Prop.Long) = 1234;

#pragma warning(disable:4786)

#include <map>
#include <string>

typedef enum {
	PropString, PropWString,
	PropBool,   PropBoolPtr,
	PropFloat,  PropFloatPtr,
	PropDouble, PropDoublePtr,
	PropLong,   PropLongPtr,
	PropInt,    PropIntPtr
} TProperty;

class CProperty
{
public:
	TProperty        Type;
	union
	{
		std::string*  String;
		std::wstring* WString;

		bool*        Bool;
		float*       Float;
		double*      Double;
		long*        Long;
		int*         Int;

		bool**       BoolPtr;
		float**      FloatPtr;
		double**     DoublePtr;
		long**       LongPtr;
		int**        IntPtr;
	};
};

class CPropertyList
{
public:
	std::map<std::string, CProperty> PropertyList;
	bool params_good = true;
	std::string params_err1;
	std::string params_err2;

	void Register(std::string Name, std::string*  String)  { CProperty &P = PropertyList[Name]; P.Type = PropString;   P.String  = String; }
	void Register(std::string Name, std::wstring* String)  { CProperty &P = PropertyList[Name]; P.Type = PropWString;  P.WString = String; }

	void Register(std::string Name, bool*        Bool)    { CProperty &P = PropertyList[Name]; P.Type = PropBool;      P.Bool   = Bool; }
	void Register(std::string Name, float*       Float)   { CProperty &P = PropertyList[Name]; P.Type = PropFloat;     P.Float  = Float; }
	void Register(std::string Name, double*      Double)  { CProperty &P = PropertyList[Name]; P.Type = PropDouble;    P.Double = Double; }
	void Register(std::string Name, long*        Long)    { CProperty &P = PropertyList[Name]; P.Type = PropLong;      P.Long   = Long; }
	void Register(std::string Name, int*         Int)     { CProperty &P = PropertyList[Name]; P.Type = PropInt;       P.Int    = Int; }

	void Register(std::string Name, bool**       Bool)    { CProperty &P = PropertyList[Name]; P.Type = PropBoolPtr;   P.BoolPtr   = Bool; }
	void Register(std::string Name, float**      Float)   { CProperty &P = PropertyList[Name]; P.Type = PropFloatPtr;  P.FloatPtr  = Float; }
	void Register(std::string Name, double**     Double)  { CProperty &P = PropertyList[Name]; P.Type = PropDoublePtr; P.DoublePtr = Double; }
	void Register(std::string Name, long**       Long)    { CProperty &P = PropertyList[Name]; P.Type = PropLongPtr;   P.LongPtr   = Long; }
	void Register(std::string Name, int**        Int)     { CProperty &P = PropertyList[Name]; P.Type = PropIntPtr;    P.IntPtr    = Int; }

	void Unregister(std::string Name) {
		std::map<std::string, CProperty>::iterator I = PropertyList.find(Name);
		if (I != PropertyList.end())
			PropertyList.erase(I);
	}
	void Check(std::string Name, double min_val, double max_val) {
		CProperty &P = PropertyList[Name];
		if (*P.Double < min_val || *P.Double > max_val) {
			params_good = false;
			std::stringstream s;
			s << "Param " << Name << "=" << *P.Double << " wrong";
			params_err1 = s.str();
			std::stringstream s2;
			s2 << "must be " << min_val << ":" << max_val;
			params_err2 = s2.str();
		}
	}
	void Check(std::string Name, int min_val, int max_val) {
		CProperty &P = PropertyList[Name];
		if (*P.Long < min_val || *P.Long > max_val) {
			params_good = false;
			std::stringstream s;
			s << "Param " << Name << "=" << *P.Long << " wrong";
			params_err1 = s.str();
			std::stringstream s2;
			s2 << "must be " << min_val << ":" << max_val;
			params_err2 = s2.str();
		}
	}

};