#include <iostream>
#include <server_scaner.hpp>
#include <server_handl.hpp>
#include <iostream>

int main(int argc, char const *argv[])
{
    int port = std::stoi( argv[1] );
    
    SERVER::run_server(port);   //Запускам
    try
    {
        Server srv;
	std::cout << "SVN edit" << std::endl;
    }
    catch (const std::exception&)
    {
        
    }
    return 0;
}
