// binary_export.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"

#include "file_data_block.h"
#include "configuration_extractor.h"

auto ExtractConfiguration(boost::filesystem::path cfg_file)
	-> decltype(configuration::Extractor::Extract(cfg_file))
{
	try
	{
		return configuration::Extractor::Extract(cfg_file);
	}
	catch (configuration::exceptions::FileOpeningError const &)
	{
		BOOST_LOG_TRIVIAL(error) << "Не удалось открыть конфигурационный файл " << boost::filesystem::absolute(cfg_file) << ".";
		return {};
	}
	catch (configuration::exceptions::CfgParsingError const& e)
	{
		BOOST_LOG_TRIVIAL(error) << "Во время разбора конфигурации файла " << boost::filesystem::absolute(cfg_file)
			<< " в строке " << e.line()
			<< " произошла ошибка: " << e.message() << '.';
		return {};
	}
}

auto CreateThreadsThatGenerateAndWriteSignalsValuesToFile(
	decltype(ExtractConfiguration(std::declval<boost::filesystem::path>())) &files_signals
) {
	static auto delete_thread_group = [](boost::thread_group *thread_group)
	{
		if (thread_group)
		{
			thread_group->interrupt_all();
			thread_group->join_all();
		}
		delete thread_group;
		thread_group = nullptr;
	};
	std::unique_ptr<boost::thread_group, decltype(delete_thread_group)> each_generate_signal_values_and_write_them_to_file{ new boost::thread_group{}, delete_thread_group };
	for (decltype(auto) file_signals : files_signals->get<configuration::files_signals_by_file_name>())
		each_generate_signal_values_and_write_them_to_file->create_thread(
			[&file_signals]() mutable
			{
				static auto delete_file = [](std::fstream *file)
				{
					if (file && file->is_open())
						file->close();
					delete file;
					file = nullptr;
				};
				static auto open_file = [](boost::filesystem::path const &path_to_file, std::ios_base::openmode mode)
					-> std::unique_ptr<std::fstream, decltype(delete_file)>
				{
					try
					{
						auto file = std::unique_ptr<std::fstream, decltype(delete_file)>(new std::fstream{ path_to_file.wstring(), mode}, delete_file);
						if (file && file->is_open())
							return std::move(file);
					}
					catch (const std::exception&)
					{
					}
					return std::unique_ptr<std::fstream, decltype(delete_file)>{nullptr, delete_file};
				};

				using signal_value_diapason = std::tuple<uint32_t, uint32_t>;
				static auto get_signal_value_diapason = [](typename std::tuple_element<1, configuration::signal>::type const &value_func)
					-> signal_value_diapason
				{
					signal_value_diapason diapason;

					std::wstringstream ss( value_func );

					using namespace boost::spirit;
					static qi::uint_parser<uint32_t> parse_uint32;
					static qi::uint_parser<uint8_t> parse_uint8;
					qi::rule<boost::spirit::basic_istream_iterator<wchar_t>, signal_value_diapason()> value_diapason_rule = qi::eps > qi::lexeme[
						qi::omit[qi::char_("[") | qi::char_("(")] >> parse_uint32 >> qi::omit[","] >> parse_uint32 >> qi::omit[qi::char_("]") | qi::char_(")")]
					];
					BOOST_SPIRIT_DEBUG_NODES((value_diapason_rule))

					qi::parse(
						boost::spirit::basic_istream_iterator<wchar_t>{ss}, boost::spirit::basic_istream_iterator<wchar_t>{},
						value_diapason_rule, diapason
					);
					return std::move(diapason);
				};
				static auto get_signal_value=[](signal_value_diapason const &diapason)
					->decltype(FileDataBlock::value)
				{
					decltype(auto) min = (std::min)(std::get<0>(diapason), std::get<1>(diapason));
					decltype(auto) max = (std::max)(std::get<0>(diapason), std::get<1>(diapason));
					return min + rand() % (max - min + 1);
				};
				static auto get_signal_data_block = [](auto &signal)
					-> FileDataBlock
				{
					FileDataBlock data_block;
					data_block.index = std::get<0>(signal);
					data_block.value = get_signal_value(
						get_signal_value_diapason(std::get<1>(signal))
					);
					data_block.status = get_signal_value(
						get_signal_value_diapason(std::get<2>(signal))
					); 
					static auto epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
					auto now = boost::posix_time::microsec_clock::local_time();

					auto time_of_day = boost::posix_time::microsec_clock::local_time().time_of_day();
					data_block.timestamp = std::make_pair(
						(now - epoch).total_seconds(),
						now.time_of_day().total_milliseconds() % 1000
					);
					return data_block;
				};

				boost::chrono::milliseconds const updation_timeot{ std::get<0>(file_signals).count() };
				if(std::get<1>(file_signals).has_parent_path())
					boost::filesystem::create_directories(std::get<1>(file_signals).parent_path());
				while(true)
				{
					try { boost::this_thread::sleep_for(updation_timeot); }
					catch (const boost::thread_interrupted&) { return; }

					auto signals_values_file = open_file(std::get<1>(file_signals), std::ios::binary | std::ios::out | std::ios::app);
					if (!signals_values_file)
					{
						BOOST_LOG_TRIVIAL(error) << "Не удается открыть файл " << std::get<1>(file_signals)
												<< ". Следующая попытка открытия будет предпринята через " << updation_timeot.count();
						continue;
					}
					decltype(auto) signals_values_destination = *signals_values_file.get();

					auto signal_values_description_file_path = std::get<1>(file_signals);
					signal_values_description_file_path.replace_extension(".log.csv");
					auto signals_values_description_file = open_file(signal_values_description_file_path, std::ios::binary | std::ios::out | std::ios::app);
					if (!signals_values_description_file)
					{
						BOOST_LOG_TRIVIAL(error) << "Не удается открыть файл для логгирования " << std::get<1>(file_signals)
							<< ". Следующая попытка открытия будет предпринята через " << updation_timeot.count();
						continue;
					}
					decltype(auto) signals_values_description_destination = *signals_values_description_file.get();

					for (decltype(auto) signal : std::get<2>(file_signals).get<configuration::signals_by_id>())
					{
						try { boost::this_thread::interruption_point(); }
						catch (const boost::thread_interrupted&) { return; }

						auto data_block = get_signal_data_block(signal);
						auto const begin = reinterpret_cast<uint8_t*>(&data_block),
							end = begin + sizeof(data_block);
						for (auto i = begin; i != end; ++i)
							signals_values_destination << *i;

						boost::spirit::karma::generate(
							boost::spirit::ostream_iterator(signals_values_description_destination),
							boost::spirit::karma::uint_ << *(';' << boost::spirit::karma::uint_) << ';' << '\r' << '\n',
							std::vector<uint32_t>{ 
								data_block.index,
								data_block.timestamp.first, data_block.timestamp.second,
								data_block.value,
								data_block.status 
							}
						);
					}
				}
			}
		);

	return std::move(each_generate_signal_values_and_write_them_to_file);
}

int main()
{
	setlocale(LC_ALL, "Russian");

	auto files_signals = ExtractConfiguration(L"configuration.xml");
	if (!files_signals.is_initialized())
		return EXIT_SUCCESS;

	auto each_generate_and_write_signals_values_to_file = CreateThreadsThatGenerateAndWriteSignalsValuesToFile(files_signals);

	while (std::cin.get() != 'q') { boost::this_thread::sleep_for(boost::chrono::seconds(1)); }
	return EXIT_SUCCESS;
}