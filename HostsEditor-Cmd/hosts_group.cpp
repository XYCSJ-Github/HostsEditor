#include "hosts_group.h"

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
