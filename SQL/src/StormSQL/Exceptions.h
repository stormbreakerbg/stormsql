#ifndef _H_EXCEPTIONS_INCLUDED
#define _H_EXCEPTIONS_INCLUDED

#include <string>
#include <stdexcept>

using namespace std;

namespace StormSQL
{
	class Exception
		: public runtime_error
	{
	public:
		Exception(const string& message)
			: runtime_error(message)
		{
		}
	};

	class UnknownIdentifierException
		: public Exception
	{
	public:
		UnknownIdentifierException(string type)
			: Exception("Unknown identifier " + type)
		{
		}
	};

	class TableSchemaOldVersion
		: public Exception
	{
	public:
		TableSchemaOldVersion()
			: Exception("Your table schema is created using an older version of StormSQL")
		{
		}
	};

	class FieldExists
		: public Exception
	{
	public:
		FieldExists()
			: Exception("This field already exists")
		{
		}
	};
	
	class FieldDoesntExist
		: public Exception
	{
	public:
		FieldDoesntExist()
			: Exception("This field doesn't exist")
		{
		}
	};

	class ColumnIndexOutOfBounds
		: public Exception
	{
	public:
		ColumnIndexOutOfBounds()
			: Exception("Column index specified is out of bounds of the table")
		{
		}
	};

	class InvalidFieldType
		: public Exception
	{
	public:
		InvalidFieldType()
			: Exception("Invalid field type")
		{
		}
	};

	class FieldDataTooLarge
		: public Exception
	{
	public:
		FieldDataTooLarge()
			: Exception("The data is too large to fit in this field type")
		{
		}
	};

	class NotAllColumnsSet
		: public Exception
	{
	public:
		NotAllColumnsSet()
			: Exception("Not all columns have set values")
		{
		}
	};

	class RowAlreadyInserted
		: public Exception
	{
	public:
		RowAlreadyInserted()
			: Exception("This row is already inserted")
		{
		}
	};

	class TableDoesntExist
		: public Exception
	{
	public:
		TableDoesntExist()
			: Exception("This table doesn't exist")
		{
		}
	};

	class TableExists
		: public Exception
	{
	public:
		TableExists()
			: Exception("A table with this name already exists")
		{
		}
	};

	class NoColumnsInTable
		: public Exception
	{
	public:
		NoColumnsInTable()
			: Exception("This table has no columns")
		{
		}
	};

	class NameTooLong
		: public Exception
	{
	public:
		NameTooLong()
			: Exception("The name is too long")
		{
		}
	};
}
#endif

