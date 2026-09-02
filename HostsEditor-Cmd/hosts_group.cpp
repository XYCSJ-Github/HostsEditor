#include "hosts_group.h"
#include <sstream>

std::string hosts_group::to_string() const
{
	std::string out;
	bool first = true;
	for (const auto& group : this->hosts)
	{
		if (!first) out += "\n";
		first = false;

		if (!group.is_default_group)
		{
			out += group.start_marker + "\n";
		}

		for (const auto& p : group.host_pair)
		{
			if (p.ip.empty() || p.host_name.empty()) continue;
			out += (p.enable ? "" : "#");
			out += p.ip + " " + p.host_name + "\n";
		}

		if (!group.is_default_group)
		{
			out += group.end_marker + "\n";
		}
	}
	return out;
}

void hosts_group::list_to_pair()
{
	for (auto& group : this->hosts)
	{
		group.host_pair.clear();
		for (const auto& line : group.data_line)
		{
			host_pair hp{};
			std::string cur = line;
			if (cur.find('#') != std::string::npos)
			{
				hp.enable = false;
				cur.erase(cur.find('#'), 1);
			}
			else
			{
				hp.enable = true;
			}

			std::istringstream iss(cur);
			std::vector<std::string> tok;
			std::string t;
			while (iss >> t) tok.push_back(t);

			if (tok.size() < 2) continue;
			hp.ip = tok[0];
			hp.host_name = tok[1];
			group.host_pair.push_back(hp);
		}
	}
}
