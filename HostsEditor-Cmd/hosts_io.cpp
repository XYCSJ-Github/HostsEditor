#include "hosts_io.h"
#include "MyException.h"
#include <fstream>
#include <regex>
#include <sstream>

void hosts_io::loadfile()
{
	if (this->hosts_path.empty()) throw EmptyString(NULL_HOSTS_PATH_STRING);

	std::ifstream h(this->hosts_path, std::ios::in);
	std::stringstream ss;
	ss << h.rdbuf();
	std::string i = ss.str();

	if (i.empty()) throw EmptyString(NULL_HOSTS_DATA);

	this->data = i;

	if (this->data.rfind("\xEF\xBB\xBF", 0) == 0)
	{
		this->data.erase(0, 3);
	}

	h.close();
}

void hosts_io::Analyse()
{
    hosts_group hg = this->find_block(this->data);
    hg.list_to_pair();
    this->groups.push_back(hg);
}

std::string hosts_io::trim(const std::string& str)
{
    size_t first = str.find_first_not_of("\t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of("\t\r\n");
    return str.substr(first, last - first + 1);
}

hosts_group hosts_io::find_block(const std::string& data)
{
    std::regex startPattern(R"(#\s*(\S+)\s+Start)");
    std::regex endPattern(R"(#\s*(\S+)\s+End)");
    std::smatch match;

    std::istringstream iss(data);
    std::string line;
    bool inBlock = false;

    std::string currentStart;
    std::vector<std::string> currentLines;
    std::vector<std::string> ungroupedLines;

    hosts_group hg;

    while (std::getline(iss, line))
    {
        std::string trimmed = trim(line);

        if (std::regex_match(trimmed, match, startPattern))
        {
            if (inBlock)
            {
                std::string endMarker = currentStart;
                size_t startPos = endMarker.rfind("Start");
                if (startPos != std::string::npos)
                {
                    endMarker.replace(startPos, 5, "End");
                }
                hosts_info_group hig
                {
                    .start_marker = currentStart,
                    .end_marker = endMarker,
                    .data_line = currentLines,
                };
                hg.add_host_group(hig);
                currentLines.clear();
            }

            inBlock = true;
            currentStart = trimmed;
            continue;
        }

        if (std::regex_match(trimmed, match, endPattern) && inBlock)
        {
            inBlock = false;
            hosts_info_group hig
            {
                .start_marker = currentStart,
                .end_marker = trimmed,
                .data_line = currentLines
            };
            hg.add_host_group(hig);
            currentLines.clear();
            continue;
        }

        if (inBlock)
        {
            currentLines.push_back(line);
        }
        else
        {
            ungroupedLines.push_back(line);
        }
    }

    if (!ungroupedLines.empty())
    {
        hosts_info_group hig
        {
            .start_marker = "#Default Start",
            .end_marker = "#Default End",
            .data_line = ungroupedLines,
            .is_default_group = true
        };
        hg.add_host_group(hig);
    }

    return hg;
}

void hosts_io::apply()
{
    if (this->hosts_path.empty()) throw EmptyString(NULL_HOSTS_PATH_STRING);

    std::string content;
    for (const auto& g : this->groups)
    {
        content += g.to_string();
    }

    std::ofstream out(this->hosts_path, std::ios::out | std::ios::trunc);
    if (!out.is_open())
    {
        throw FileWriteError();
    }
    out << content;
    out.close();
    if (!out)
    {
        throw FileWriteError();
    }
}

