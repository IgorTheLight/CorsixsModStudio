/*
Rainman Library
Copyright (C) 2006 Corsix <corsix@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _I_META_TABLE_H_
#define _I_META_TABLE_H_

#include "gnuc_defines.h"
#include <wchar.h>
#include "Api.h"
#include "CMemoryStore.h"

//! A Lua/Rgd variable
/*!
	Represents a variable found in a lua/rgd file.
*/
class RAINMAN_API IMetaNode
{
public:
	//! Constructor
	IMetaNode();

	//! Destructor
	virtual ~IMetaNode();

	//! RGD data types
	/*!
		The various types of data found in RGD files.
		Numbers match up to the type ID used in RGD files.
	*/
	enum eDataTypes
	{
		//! Floating point numeric data [float]
		DT_Float = 0,
		//! Interger numberic data (used in CoH for string references) [unsigned long]
		DT_Integer = 1,
		//! Boolean data [bool]
		DT_Bool = 2,
		//! ASCII text data [char*]
		DT_String = 3,
		//! Unicode text data [wchar_t*]
		DT_WString = 4,
		//! Table data
		DT_Table = 100,
		//! No data (not read or written to RGD, or unknown data type)
		DT_NoData = 254, // Hope Relic doesn't use 254 :)
	};

	//! Data for a Lua/Rgd table variable
	/*!
		The data for a IMetaNode that represents a table.
	*/
	class RAINMAN_API IMetaTable
	{
	public:
		//! Constructor
		IMetaTable(void);

		//! Destructor
		virtual ~IMetaTable(void);

		//! Get the number of children
		/*!
			\return Returns the number of children the table has. May throw a CRainmanException
		*/
		virtual unsigned long VGetChildCount() = 0;

		//! Get a specified child
		/*!
			\param[in] iIndex Range is 0 to VGetChildCount() - 1 (inclusive)
			\return Returns a valid pointer or throws a CRainmanException. You need to delete this pointer when you are done using it.
		*/
		virtual IMetaNode* VGetChild(unsigned long iIndex) = 0;

		//! Get the data type of the reference string
		/*!
			If the table has a Reference([[somefile]]) or Inherit([[somefile]]) then "somefile" is called the reference string
			The reference string can be a IMetaNode::DT_String, a IMetaNode::DT_WString, or there isn't one, it is IMetaNode::DT_NoData.
			\return Returns IMetaNode::DT_String, IMetaNode::DT_WString, IMetaNode::DT_NoData or throws a CRainmanException on error
		*/
		virtual IMetaNode::eDataTypes VGetReferenceType() = 0;

		//! Set the data type of the reference string
		/*!
			\param[in] eType The data type to set the reference to (IMetaNode::DT_String, IMetaNode::DT_WString or IMetaNode::DT_NoData)
			\return Returns nothing, but will throw a CRainmanException on error
			\see VGetReferenceType()
		*/
		virtual void VSetReferenceType(IMetaNode::eDataTypes eType) = 0;

		//! Get the reference string
		/*!
			Use only if VGetReferenceType() returns IMetaNode::DT_String
			\return Returns a valid pointer or throws a CRainmanException
			\see VGetReferenceWString()
		*/
		virtual const char* VGetReferenceString() = 0;

		//! Set the reference string
		/*!
			Make sure to set the data type to IMetaNode::DT_String before
			calling or the call may fail.
			\param[in] sValue The new reference string
			\return Returns nothing, but will throw a CRainmanException on error
			\see VGetReferenceType()
			\see VGetReferenceString()
		*/
		virtual void VSetReferenceString(const char* sValue) = 0;

		//! Get the reference string (unicode)
		/*!
			Use only if VGetReferenceType() returns IMetaNode::DT_WString
			\return Returns a valid pointer or throws a CRainmanException
			\see VGetReferenceType()
			\see VGetReferenceString()
		*/
		virtual const wchar_t* VGetReferenceWString() = 0;

		//! Set the reference string (unicode)
		/*!
			Make sure to set the data type to IMetaNode::DT_WString before
			calling or the call may fail.
			\param[in] wsValue The new reference string
			\return Returns nothing, but will throw a CRainmanException on error
			\see VGetReferenceType()
			\see VGetReferenceWString()
		*/
		virtual void VSetReferenceWString(const wchar_t* wsValue) = 0;

		//! Add a child to the table
		/*!
			The index of the new child need not be VGetChildCount() - 1
			If two IMetaTable objects both refer to the same table, and
			a child is added to one of them, then the other will also become
			aware of the existance of the child.
			\param[in] sName The name of the new child (cannot be the same as an existing child)
			\return A pointer to the newly created child, throws a CRainmanException on error. You need to delete this pointer when you are done using it (
		*/
		virtual IMetaNode* VAddChild(const char* sName) = 0;

		//! Delete a specified child
		/*!
			If two IMetaTable objects both refer to the same table, and a
			child is removed from one of them, then the other will also
			be updated.
			\param[in] iIndex Range is 0 to VGetChildCount() - 1 (inclusive)
			\return Returns nothing, but will throw a CRainmanException on error
		*/
		virtual void VDeleteChild(unsigned long iIndex) = 0;

		virtual bool VSupportsRefresh();
		virtual void VDoRefresh();
	};

	//! Get the data type of the variable
	/*!
		\return Returns the data type of the variabe, will throw a CRainmanException on error
	*/
	virtual eDataTypes VGetType() = 0;

	//! Set the data type of the variable
	/*!
		\param[in] eType The type to set the variable to
		The variable will be converted to the specified data type
		\return Returns nothing, but will throw a CRainmanException on error
	*/
	virtual void VSetType(eDataTypes eType) = 0;

	//! Get the name of the variable
	/*!
		Some variables have a hash, some have a name, some have both. Try this first, and then the hash.
		\return Will return an ASCII string, or return 0. If it returns 0, then
		you should try VGetNameHash() to get the hash of the name.
	*/
	virtual const char* VGetName() = 0;

	//! Set the name of the variable
	/*!
		Will set the name of the variable. If VSetName() throws an exception
		then you should try VSetNameHash() instead.
	*/
	virtual void VSetName(const char* sName) = 0;

	//! Get the hash of the name of the varialbe
	/*!
		RGDs store the hash of the name, and any hash values not
		found in the dictionary will not have an ASCII name. In
		this case you should use the name hash. Luas store the
		name, and not the hash, so use VGetName() for Luas.

		In conclusion, use VGetName() first, and if it returns 0
		then try this.
	*/
	virtual unsigned long VGetNameHash();

	//! Set the hash of the name of the variable
	/*!
		Lua files cannot have a name set by hash, whereas Rgd files can.
		Use VSetName() before you try and use this, as a hash can be obtained from a name but no vice verca
		\param[in] iHash The hash to set the name to
		\return Returns no values, but throws a CRainmanException on error
		\see VGetNameHash()
	*/
	virtual void VSetNameHash(unsigned long iHash);

	// Floats, Bools, Strings , WStrings

	//! Get the value of the variable as a floating point number
	/*!
		Use only if VGetType() returns IMetaNode::DT_Float
		\return Returns the value, or throws a CRainmanException
	*/
	virtual float VGetValueFloat() = 0;

	//! Set the value of the variable to a floating point number
	/*!
		Make sure that the data type is set to IMetaNode::DT_Float
		using VSetType() before calling.
		\param[in] fValue The value
		\return Returns no value, but throws a CRainmanException on error
	*/
	virtual void VSetValueFloat(float fValue) = 0;

	//! Get the value of the variable as an integer number
	/*!
		Use only if VGetType() returns IMetaNode::DT_Integer
		\return Returns the value, or throws a CRainmanException
	*/
	virtual unsigned long VGetValueInteger() = 0;

	//! Set the value of the variable to an integer number
	/*!
		Make sure that the data type is set to IMetaNode::DT_Integer
		using VSetType() before calling.
		\param[in] iValue The value
		\return Returns no value, but throws a CRainmanException on error
	*/
	virtual void VSetValueInteger(unsigned long iValue) = 0;

	//! Get the value of the variable as a boolean
	/*!
		Use only if VGetType() returns IMetaNode::DT_Bool
		\return Returns the value, or throws a CRainmanException
	*/
	virtual bool VGetValueBool() = 0;

	//! Set the value of the variable to a boolean
	/*!
		Make sure that the data type is set to IMetaNode::DT_Bool
		using VSetType() before calling.
		\param[in] bValue The value
		\return Returns no value, but throws a CRainmanException on error
	*/
	virtual void VSetValueBool(bool bValue) = 0;

	//! Get the value of the variable as an ASCII string
	/*!
		Use only if VGetType() returns IMetaNode::DT_String
		\return Returns the value, or throws a CRainmanException
	*/
	virtual const char* VGetValueString() = 0;

	//! Set the value of the variable to an ASCII string
	/*!
		Make sure that the data type is set to IMetaNode::DT_String
		using VSetType() before calling.
		\param[in] sValue The value
		\return Returns no value, but throws a CRainmanException on error
	*/
	virtual void VSetValueString(const char* sValue) = 0;

	//! Get the value of the variable as a unicode string
	/*!
		Use only if VGetType() returns IMetaNode::DT_WString
		\return Returns the value, or throws a CRainmanException
	*/
	virtual const wchar_t* VGetValueWString() = 0;

	//! Set the value of the variable to a unicode string
	/*!
		Make sure that the data type is set to IMetaNode::DT_WString
		using VSetType() before calling.
		\param[in] wsValue The value
		\return Returns no value, but throws a CRainmanException on error
	*/
	virtual void VSetValueWString(const wchar_t* wsValue) = 0;

	//! Get the value of the variable as a table
	/*!
		A variable cannot be set to a metatable (eg. there is no VSetValueMetatable() ), instead you set the data type
		to IMetaNode::DT_Table, do a VGetValueMetatable() and then manipulate the returned pointer.
		\return Will throw an exception or return a valid pointer
		\attention You must delete the return value when you are finished using it
	*/
	virtual IMetaTable* VGetValueMetatable() = 0;

	//! Write this node as application/x-rainman-rgd psuedo mime type
	/*!
		To allow Metanodes to be copy & pasted around, they have to have a common format for interchange.
		This function must be implemented as to return a stream of data which is "application/x-rainman-rgd".
		Format for this data is:<br/>
		<4 bytes unsigned> NameHash <br/>
		<4 bytes unsigned> NameLen (set to 0 = name unknown; must have a valid hash in this case)<br/>
		<N bytes> Name<br/>
		<1 byte> DataType<br/>
		<4 bytes unsigned> DataLength (for string: strlen, for wstring: wcslen, for table: number of children)<br/>
		<N bytes> Data<br/><br/>
		For tables, data looks like:<br/>
		Child 1<br/>
		Child 2<br/>
		...
		\return A fresh memory output stream which will be later deleted by the calling code. May throw a CRainmanException, but will otherwise return a valid pointer
	*/
	virtual CMemoryStore::COutStream* VGetNodeAsRainmanRgd() = 0;

	//! Set this node from a application/x-rainman-rgd psuedo mime type
	/*!
		\see VGetNodeAsRainmanRgd()
	*/
	virtual void SGetNodeFromRainmanRgd(IFileStore::IStream* pInput, bool bSetName = false) = 0;
};

#endif

