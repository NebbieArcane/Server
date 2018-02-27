#include "config.hpp"
#include "comm.hpp"
#include "interpreter.hpp"
#include "utils.hpp"
#include <iostream>
#include <boost/program_options.hpp>

using std::string;
using std::endl;
using std::cout;
int main(int argc, char** argv) {
#ifdef DFLT_PORT
	int port=DFLT_PORT;
#else
	int port=4000;
#endif
#ifdef DFLT_DIR
	string dir(DFLT_DIR);
#else
	string dir("lib");
#endif
	string server_name="starting";
	string comment="";
	unsigned short debug_level=0;
	namespace po = boost::program_options ;
	po::options_description opt("Allowed options");
	opt.add_options()
	("help,h","Produce help message")
	("demonize,D","Run as daemon")
	("log_players,L","Logs all players' actions")
	("log_mobs,M","Logs all mobs' actions")
	("disable_DNS,N","Disables DNS")
	("newbie_approve,R","Requests approvation for new players")
	("ansi_off,A","Disables all colors")
	("test_mode,T","Developer mode, disables password checking")
	("verbose_log,v",po::value<unsigned short> (& debug_level)->default_value(3),"Log verbosity level")
	("directory,d",po::value<string>(&dir)->default_value(dir),"Data directory")
	("nospecials,s",po::value<bool>(&no_specials)->default_value(false),"Disable specials procedures")
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
	try {
		po::store(po::command_line_parser(argc, argv).
				  options(all).positional(positional).run(), vm);
	}
	catch (po::unknown_option &e) {
		cout << e.what() << endl;
	}
	po::notify(vm);
	if(vm.count("help") or !vm.count("port")) {
		cout << argv[0] << " <port> <options>" << endl;
		cout << opt << endl;
		exit(0);
	}
	cout << endl;
	if(vm.count("ansi_off")) { SET_BIT(SystemFlags,SYS_NOANSI); }
	if(vm.count("disable_DNS")) { SET_BIT(SystemFlags,SYS_SKIPDNS); }
	if(vm.count("log_players")) { SET_BIT(SystemFlags,SYS_LOGALL); }
	if(vm.count("newbie_approve")) { SET_BIT(SystemFlags,SYS_REQAPPROVE); }
	if(vm.count("log_mobs")) { SET_BIT(SystemFlags,SYS_LOGMOB); }
	if(vm.count("Demonize")) {
		int pid = fork();

		if(pid < 0) {
			LOG_FATAL("Error forking sysD daemon.");
		}
		else if(pid > 0) {
			// Parent exit.
			cout << "Demonized" <<endl;
			exit(0);
		}

		if(setsid() < 0) {
			LOG_FATAL("ERROR setting session : " << strerror(errno));
		}

		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
	}
	Alarmud::log_init("alarmud", debug_level);
	if (chdir(dir.c_str()) < 0) {
		LOG4CXX_FATAL(::logger,"Unable to change dir to " << dir);
		perror("chdir");
		assert(0);
	}
	LOG4CXX_TRACE(::logger,"Boost version " << BOOST_VERSION);
	run(port,dir.c_str());
	return 0;
}
