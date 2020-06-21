#pragma once

#include <fstream>

#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include "emplacement_did_not_happen_callback.h"
#include "from_string.h"
#include "signals_configuration.h"

/*
	Пример конфигурации:
		возможные значения
			has_header: y/yes, n/no, true, false
			type: csv, xml, binary
			fields_delimiter: любая строка, но учитывая, что некоторые значения могут сломать парсер.
			new_line: любая строка, но учитывая, что некоторые значения могут сломать парсер.
	<groups>
		<group path="file.csv" updation_timeout_ms="500">
			<signals>
				<signal id="1" value_func="[1,3]" status_func="[0,255]"/>
				<signal id="1" value_func="[1,1]" status_func="[1,1]"/>
				<signal id="3" value_func="[1,0]" status_func="[0,254]"/>
				<signal id="2" value_func="[0,255]" status_func="[1,3]"/>
				<signal id="4" value_func="[1,3]" status_func="[1,2]"/>
			</signals>
		</group>
	</groups>
*/

namespace configuration
{
	namespace exceptions
	{
		class FileOpeningError
		{
		};
		class CfgParsingError
			: public boost::property_tree::xml_parser_error
		{
		public:
			explicit CfgParsingError(boost::property_tree::xml_parser_error e)
				: xml_parser_error(e)
			{
			}
		};
	}

	struct Extractor
	{
	public:
		static boost::optional<signals_by_files> Extract(std::wistream &istream);
		// throw FileDoesNotExist
		static boost::optional<signals_by_files> Extract(boost::filesystem::path &file_path);
	};
}