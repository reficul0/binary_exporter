#ifndef ALPHA_SERVER_BOOST_FILESYSTEM_PATH_HASHING_SUPPORT_H
#define ALPHA_SERVER_BOOST_FILESYSTEM_PATH_HASHING_SUPPORT_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#include <boost/filesystem/path.hpp>
#include <boost/functional/hash.hpp>

namespace std
{
	template <>
	struct hash<boost::filesystem::path>
	{
		size_t operator()(boost::filesystem::path const& ss) const
		{
			return boost::filesystem::hash_value(ss);
		}
	};
}

#endif// ALPHA_SERVER_BOOST_FILESYSTEM_PATH_HASHING_SUPPORT_H