#ifndef ALPHA_SERVER_FROM_STRING_H
#define ALPHA_SERVER_FROM_STRING_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#include <string>

namespace tools
{
	namespace exceptions
	{
		class UndefinedValue
		{
		};
	}

	// throw tools::exceptions::UndefinedValue
	template<typename ParamT>
	ParamT from_string(std::wstring &param_val_as_str);
}

#endif // ALPHA_SERVER_FROM_STRING_H