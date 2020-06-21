#include "pch.h"
#include "configuration_extractor.h"
#include "from_string.h"

namespace configuration
{
	boost::optional<signals_by_files> Extractor::Extract(std::wistream &istream)
	{
		boost::property_tree::wptree propertyTree;
		try
		{
			boost::property_tree::read_xml(istream, propertyTree);
		}
		catch (boost::property_tree::xml_parser_error const &e)
		{
			throw exceptions::CfgParsingError{e};
		}

		// �������� ������� <groups>
		auto groups = propertyTree.get_child_optional(L"groups");
		if (!groups.is_initialized())
		{
			BOOST_LOG_TRIVIAL(error) << "������������ �� �������� ������������ �������� ������� \"groups\". �������� ��� � ������������.";
			return {};
		}

		signals_by_files files_signals;
		for (auto &file : groups.get())
		{
			if (file.first != L"group")
				continue;

			/*
			<groups>
				<group path="file.csv" updation_timeout_ms="500">
				...
				</group>
				...
			</groups>
			*/
			auto file_path = file.second.get_optional<boost::filesystem::path>(L"<xmlattr>.path");
			if (!file_path.is_initialized())
			{
				BOOST_LOG_TRIVIAL(error) << "������������ ������ �� �������� ������������ ������� \"path\". �������� ��� � ������������ �����.";
				continue;
			}
			if (file_path.get().empty())
			{
				BOOST_LOG_TRIVIAL(error) << "������������ ������� ������������ ������ \"path\" ����. ���������� � ���� �� ������ ��������.";
				continue;
			}

			auto updation_timeout_ms_count = file.second.get_optional<std::size_t>(L"<xmlattr>.updation_timeout_ms");
			auto updation_timeout_ms = std::chrono::milliseconds(500);
			if (!updation_timeout_ms_count.is_initialized())
				BOOST_LOG_TRIVIAL(warning) << "������������ ������ � ������ �����" << file_path.get() 
											<< " �� �������� ������������� �������� \"updation_timeout_ms\"."
											<< " ��� �������� ����� ����������� � \"" << updation_timeout_ms.count() << "\".";
			else
				updation_timeout_ms = std::chrono::milliseconds(updation_timeout_ms_count.get());

			decltype(files_signals.begin()) file_signals;
			bool did_insertion_happen = true;
			std::tie(file_signals, did_insertion_happen) = files_signals.emplace(updation_timeout_ms, file_path.get(), configuration::signals{});
			if(!did_insertion_happen)
			{
				BOOST_LOG_TRIVIAL(error) << "� ����� ������������ ������������ ������ � ������" << boost::filesystem::absolute(file_path.get()) << " ����������� ��������� ���."
					<< " ������ ����� ��� ��������� ��-�� ������ ����������������."
					<< " �� ������������ ����� ����������� ������������ ������ ������������ ������ � ����� ������ �����."
					<< " ������������ ����� � �����-�� ������ ����� ����� ���������������.";
				continue;
			} 

			// ���� �����: ������� <signals>
			auto cfg_signals = file.second.get_child_optional(L"signals");
			if (!cfg_signals.is_initialized())
			{
				BOOST_LOG_TRIVIAL(error) << "������������ ������ � ������  �����" << file_path.get() 
											<< " �� �������� ������������� ��������-������� \"signals\"."
											<< " ������������ ������ ����� ���������������.";
				continue;
			}

			/*������ ��������
			<signals>
				<signal id="1" value_func="[1,3]" status_func="[1,2]"/>
				...
			</signals>
			*/
			decltype(auto) signals = std::get<2>(file_signals.get_node()->value());

			for (auto &signal : cfg_signals.get())
			{
				if (signal.first != L"signal")
					continue;

				auto signal_id_attr = signal.second.get_optional<size_t>(L"<xmlattr>.id");
				if (!signal_id_attr.is_initialized())
				{ 
					BOOST_LOG_TRIVIAL(error) << "� ������������ ������ � ������ ����� " << file_path.get() 
												<< " �������-������� \"signals.signal\" �� �������� ������������� �������� \"id\"."															
												<< " ���� ������ �� ����� �������������.";					
					continue;
				}

				auto signal_value_func_attr = signal.second.get_optional<std::wstring>(L"<xmlattr>.value_func");
				if (!signal_value_func_attr.is_initialized())
				{
					BOOST_LOG_TRIVIAL(error) << "� ������������ ������ � ������ ����� " << file_path.get()
						<< " �������-������� \"signals.signal\" �� �������� ������������� �������� \"value_func\"."
						<< " ������������ ������� ����� ���������������.";
					continue;
				}

				auto signal_status_func_attr = signal.second.get_optional<std::wstring>(L"<xmlattr>.status_func");
				if (!signal_status_func_attr.is_initialized())
				{
					BOOST_LOG_TRIVIAL(warning) << "� ������������ ������ � ������ ����� " << file_path.get()
						<< " �������-������� \"signals.signal\" �� �������� ������������� �������� \"status_func\"."
						<< " ������������ ������� ����� ���������������.";
					continue;
				}

				signals.emplace(
					std::make_tuple(
						signal_id_attr.get(), 
						signal_value_func_attr.get(),
						signal_status_func_attr.get()
					)
				);
			}
			if(signals.empty())
			{
				BOOST_LOG_TRIVIAL(error) << "������������ ������ � ������ ����� " << file_path.get()
					<< " �� �������� ���������-����� \"signals.signal\", ���� �� �������� �����������."
					<< " ������������ ������ ����� ���������������.";
				continue;
			}
		}

		return std::move(files_signals);
	}

	boost::optional<signals_by_files> Extractor::Extract(boost::filesystem::path &file_path)
	{
		std::wifstream istream(file_path.wstring(), std::ios::binary);

		return istream.is_open()
				? Extract(istream) 
				: throw exceptions::FileOpeningError{};
	}
}