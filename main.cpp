#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <filesystem>

std::string shutils[] = {"exit", "cd", "help", "pwd", "exec", "type", "export", "unset"};
struct {
	std::string home, user, hostname;
	std::unordered_map<std::string, std::string> alias;
} shellenv; // contain shell-specific environment tables

void fetchenv()
{
	const char* homeptr = std::getenv("HOME");
	const char* userptr = std::getenv("USER");
	shellenv.user = (userptr!=nullptr) ? userptr : "unknown";
	shellenv.home = (homeptr!=nullptr) ? homeptr : "/";

	char hostnamebuffer[256];
	shellenv.hostname = (gethostname(hostnamebuffer, 256)==0) ? hostnamebuffer : "undefined";
}

std::string errlookup(int opcode)
{
	if (!opcode)
		return "Shell exited normally.";
	else if (opcode==1)
		return "Fatal input error.";
	else
		return "Catastrophic error.";
}

bool isbuiltin(std::string file)
{
	for (long unsigned int i=0; i<size(shutils); i++)
		if (file==shutils[i])
			return true;
	return false;
}

std::string parse(std::string __command)
{
	std::string result = __command;
	// will code here
	// will be POSIX-compliant
	return result;
}

std::string foundexe(std::string file)
{
	std::string path = "PATH";
	const char* fetchpath = std::getenv(path.c_str());

	std::stringstream ss(fetchpath);
	std::string tmp;

	while (getline(ss, tmp, ':'))
	{
		// get the full executable path and then append executable name
		std::filesystem::path exepath = std::filesystem::path(tmp)/file;
		if (std::filesystem::exists(exepath))
			return exepath.string(); // now turn it back to string
	}

	return "I found something";
}

int shell(std::string readline)
{
	if (readline.rfind("exit")==0)
		return 1;

	if (readline=="cd") {
		int opcode = (chdir(shellenv.home.c_str())==0) ? 0 : 2;
		if (opcode==2) std::cerr << "You're homeless.\n";
		return opcode;
	}

	if (readline.rfind("cd ", 0)==0) { // found whole "cd " at index 0
		std::string where = readline.substr(3);
		int opcode = (chdir(where.c_str())==0) ? 0 : 3;
		if (opcode==3) std::cerr << "Directory not found.\n";
		return opcode;
	}

	if (readline.rfind("help", 0)==0) {
		std::cout << "No manual available. Worse yet, you're asking me something.\n";
		return 0;	
	}

	if (readline=="pwd") {
		char currworkdir[2048]; // very generous
		if (getcwd(currworkdir, 2048)) {
			std::cout << currworkdir << '\n';
			return 0;
		} else {
			std::cerr << "Current directory may not exist anymore.\n";
			return 4;
		}
	} // I've only implemented pwd because I was lazy

	if (readline.rfind("exec ", 0)==0) {
		std::string fetch = readline.substr(5);

		std::vector<std::string> tokens;
		std::stringstream ss(fetch); // food order list
		std::string val; // temporary buffer
		while (ss >> val)
			tokens.push_back(val);

		std::vector<char*> argv; // store pointers to arguments
		for (std::string &have : tokens)
			argv.push_back(&have[0]); // grab pointer of argument
		argv.push_back(nullptr); // a terminator

		execvp(argv[0], argv.data()); // file and full command
	}

	if (readline.rfind("type ", 0)==0) {
		std::string fetch = readline.substr(5);

		if (shellenv.alias.find(fetch)!=shellenv.alias.end()) {
			std::cout << fetch << " is an alias.\n";
			return 0;
		}

		if (isbuiltin(fetch)) {
			std::cout << fetch << " is a built-in shell command.\n";
			return 0;
		}

		std::string found = foundexe(fetch);
		if (found!="I found something") {
			std::cout << fetch << " is an executable in " << found << '\n';
			return 0;
		}

		std::cout << fetch << " doesn't exist anywhere.\n";
		return 5;
	}

	if (readline.rfind("export ", 0)==0) {
		std::string expression = readline.substr(7);

		size_t delimiter = expression.find("=");
		if (delimiter==std::string::npos) { // not found
			const char* var = std::getenv(expression.c_str());
			if (var!=nullptr) std::cout << expression << "=" << var << '\n';
			else std::cout << expression << "=\n";
			return 0;
		}

		std::string name = expression.substr(0, delimiter);
		std::string val = expression.substr(delimiter+1);

		// force overwrite
		int opcode = setenv(name.c_str(), val.c_str(), 1);
		if (opcode!=0) std::cerr << "Cannot set environment table with error code " << opcode << ".\n";
		else return 0;
		return 6;
	}

	if (readline=="") {
		std::cerr << "Did nothing because you gave me nothing.\n";
		return 7;
	}

	if (readline=="env" || readline=="export") {
		extern char** environ;
		for (int i=0; environ[i]!=nullptr; i++)
			std::cout << environ[i] << '\n';
		return 0;
	}

	if (readline.rfind("unset ", 0)==0) {
		std::string fetch = readline.substr(6);
		if (unsetenv(fetch.c_str())!=0) {
			std::cerr << "Fatal error encountered while removing a variable.\n";
			return 8;
		} else return 0;
	}

	if (readline.rfind("alias ", 0)==0) {
		std::cerr << "Not available." << '\n';
		return 9;
	}

	if (readline.rfind("unalias ", 0)==0) {
		std::cerr << "This feature is behind a paywall. Please purchase to use it.\n";
		return 10;
	}

	return -1;
}

int main()
{
	fetchenv();
	std::string readline;
	int lastexit = 0;

	while (true)
	{
		// commandline heading
		std::cout << shellenv.user << "@" << shellenv.hostname << " >>> ";
		if (lastexit!=0)
			std::cout << "[" << lastexit << "]" << " ";

		// stdin error
		if (!std::getline(std::cin, readline)) return 1;

		int opcode = shell(readline);
		if (opcode==1) { lastexit = 0; break; }
		else if (opcode<0) lastexit = system(readline.c_str());
		else lastexit = opcode;
	}

	// print a newline before exiting
	std::cout << "Shell exited with error code " << lastexit << ".\n";
	std::cerr << errlookup(lastexit) << '\n';
	return 0;
}
