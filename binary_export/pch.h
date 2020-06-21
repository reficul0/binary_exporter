#ifndef PCH_H
#define PCH_H

#include <iostream>
#include <fstream>
#include <clocale>
#include <mutex>
#include <string>
#include <tuple>
#include <array>
#include <iterator>

#include <boost/thread.hpp>
#include <boost/thread/thread_pool.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/log/trivial.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <boost/spirit/include/karma.hpp>

// Если значение ACTIVATE_BOOST_SPIRIT_DEBUG <= 0u, то отладка и вывод отладочной
// информации производится не будет. Иначе отладка boost spirit явялется включенной.
#define ACTIVATE_BOOST_SPIRIT_DEBUG 0u

#if ACTIVATE_BOOST_SPIRIT_DEBUG > 0u
#define BOOST_SPIRIT_DEBUG
#endif

/////////////////////////////////////////////////////////////////////////////////////////
///	@short
///		Макрос позволяющий активировать использование Boost.Phoenix V3.
/////////////////////////////////////////////////////////////////////////////////////////
#define BOOST_SPIRIT_USE_PHOENIX_V3 1u

#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>

#endif //PCH_H
