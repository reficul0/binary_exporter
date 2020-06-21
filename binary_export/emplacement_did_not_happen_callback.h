#ifndef ALPHA_SERVER_EMPLACEMENT_DID_NOT_HAPPEN_CALLBACK_H
#define ALPHA_SERVER_EMPLACEMENT_DID_NOT_HAPPEN_CALLBACK_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#include <type_traits>
#include <tuple>
#include <utility>

namespace tools
{
	template<typename ContainerT, typename ValueT, typename CallableT>
	decltype(std::declval<const ContainerT&>().emplace(std::declval<ValueT>()), bool()) callback_if_emplacement_did_not_happen(ContainerT &&container, ValueT &&value, CallableT &&callback)
	{
		typename std::decay<decltype(container)>::type::iterator emplaced_or_existing_value;
		bool did_insertion_happen = true;
		std::tie(emplaced_or_existing_value, did_insertion_happen) = container.emplace(std::forward<ValueT>(value));
		if (!did_insertion_happen)
			std::forward<CallableT>(callback)(emplaced_or_existing_value);

		return !did_insertion_happen;
	}
}

#endif ALPHA_SERVER_EMPLACEMENT_DID_NOT_HAPPEN_CALLBACK_H