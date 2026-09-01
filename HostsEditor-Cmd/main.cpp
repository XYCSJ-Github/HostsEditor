#include "hosts_io.h"
#include "Logout/Logout.h"

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
	
	return 0;
}