#include "hosts_io.h"
#include "Logout/Logout.h"

std::string showhosts(std::vector<hosts_group> hg)
{
	std::string outstr;
	for (hosts_group& c : hg)
	{
		for (hosts_info_group& a : c.get_host_group())
		{
			std::string sstr = a.start_makeer;
			size_t spos = sstr.find('#');
			if (spos == std::string::npos);

			size_t start = spos + 1;
			size_t end = sstr.find(' ', start);

			if (end == std::string::npos) end = sstr.length();

			outstr += sstr.substr(start, end - start) + ":\n";
			for (host_pair& b : a.host_pair)
			{
				outstr += "IP:" + b.ip + "Host:" + b.host_name + "Enable:";
				if (b.enable == true)
				{
					outstr += "Yes\n";
				}
				else
				{
					outstr += "No\n";
				}
			}
			outstr += "\n";
		}
	}
	return outstr;
}

int main()
{
	LOG_CREATE_MODEL_NAME("main");

	std::string str = "./ht";
	hosts_io hi;
	try
	{
		hi.setpath(str);
		hi.loadfile();
		hi.Analyse();
	}
	catch (const std::exception& e)
	{
		LOG_WARNING(e.what());
	}
	std::cout << showhosts(hi.get_group());
	
	return 0;
}