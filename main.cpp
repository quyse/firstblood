#include "general.hpp"
#include "Game.hpp"
#include <sstream>
#include <iostream>
#include <fstream>

#if defined(PRODUCTION) && defined(___INANITY_WINDOWS)
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, INT)
#else
int main()
#endif
{
	try
	{
		MakePointer(NEW(Game()))->Run();
	}
	catch(Exception* exception)
	{
		std::ostringstream s;
		MakePointer(exception)->PrintStack(s);
		std::cout << s.str() << '\n';
	}

	return 0;
}
