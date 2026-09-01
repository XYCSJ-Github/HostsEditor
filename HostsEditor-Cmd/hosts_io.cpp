#include "hosts_io.h"

void hosts_io::loadfile()
{
	if (this->hosts_path.empty()) throw EmptyString(NULL_HOSTS_PATH_STRING);

	std::ifstream h(this->hosts_path, std::ios::in);
	std::string i;
	h >> i;

	if (i.empty()) throw EmptyString(NULL_HOSTS_DATA);

	this->data = i;

	h.close();
}

void hosts_io::Analyse()
{
    hosts_group hg = this->find_block(this->data);
    hg.list_to_pair();
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
    std::regex startPattern(R"(#(\w+)\s+Start)");
    std::regex endPattern(R"(#(\w+)\s+End)");
    std::smatch match;

    std::istringstream iss(data);
    std::string line;
    bool inBlock = false;

    std::string currentStart;
    std::vector<std::string> currentLines;

    hosts_group hg;

    while (std::getline(iss, line))
    {
        std::string trimmed = trim(line);

        if (std::regex_match(trimmed, match, startPattern))
        {
            if (inBlock)
            {
                hosts_info_group hig
                {
                .start_makeer = currentStart,
                .end_makeer = "#" + match[1].str() + " End",
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
                .start_makeer = currentStart,
                .end_makeer = trimmed,
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

        if (inBlock && !currentLines.empty())
        {
            hosts_info_group hig
            {
                .start_makeer = currentStart,
                .end_makeer = "",
                .data_line = currentLines
            };
            hg.add_host_group(hig);
        }
    }

    return hg;
}

