#include "config.hpp"
#include <boost/program_options.hpp>
#include "logging.hpp"
#include "comm.hpp"
#include <iostream>

using std::string;
using std::endl;
using std::cout;
int main(int argc, char** argv) {
	int port=4000;
	string server_name="starting";
	string comment="";
	unsigned short debug_level=0;
	namespace po = boost::program_options ;
	po::options_description opt("Allowed options");
	opt.add_options()
	("help,h","Produce help message")
	("Demonize,D","Run as daemon")
	("debug,d",po::value<unsigned short> (& debug_level)->default_value(3),"debug level")
	("exec_comment,X",po::value<string> (& comment)->default_value(""),"Execute comment")
//    ;
//    po::options_description hidden("Hidden options");
//    hidden.add_options()
	("port,P",po::value<int> (&port),"Port (can also be given as first parameter)")
	;
	po::positional_options_description positional;
	positional.add("port",1);
	po::options_description all("All");
	all.add(opt);//.add(hidden);
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).
			  options(all).positional(positional).run(), vm);
	po::notify(vm);
	if(vm.count("help") or !vm.count("port")) {
		cout << "./AlarMUD3 <port> <options>" << endl;
		cout << opt << endl;
		exit(0);
	}
	cout << endl;
	Alarmud::log_init("alarmud", debug_level);
	LOG4CXX_TRACE(::logger,"Boost version " << BOOST_VERSION);
	comm_main(argc,argv);
	return 0;
}
