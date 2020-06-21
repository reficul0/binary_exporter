#pragma once

#include <tuple>
#include <unordered_map>
#include <chrono>

#include <boost/filesystem/path.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include "boost_filesystem_path_hashing_support.h"

using namespace boost::multi_index;

namespace configuration
{
	using signal = std::tuple<uint32_t/*id*/, std::wstring/*value*/, std::wstring/*status*/>;

	using signals = boost::multi_index_container<
		signal,
		indexed_by<
			ordered_unique<
				tag<struct signals_by_id>,
				global_fun<
					const signal&, 
					const typename std::tuple_element<0, signal>::type&,
					&std::get<0, uint32_t/*id*/, std::wstring/*value*/, std::wstring/*status*/>
				>
			>
		>
	>;
	using signals_by_file = std::tuple<std::chrono::milliseconds, boost::filesystem::path, signals>;
	using signals_by_files = boost::multi_index_container<
		signals_by_file,
		indexed_by<
			ordered_non_unique<
				tag<struct files_signals_by_updation_timeout>,
				global_fun<
					const signals_by_file&,
					const typename std::tuple_element<0, signals_by_file>::type&,
					&std::get<0, std::chrono::milliseconds, boost::filesystem::path, signals>
				>
			>,
			hashed_unique<
				tag<struct files_signals_by_file_name>,
				global_fun<
					const signals_by_file&,
					const typename std::tuple_element<1, signals_by_file>::type&,
					&std::get<1, std::chrono::milliseconds, boost::filesystem::path, signals>
				>
			>
		>
	>;
}