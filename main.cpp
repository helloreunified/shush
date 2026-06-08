#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <sys/wait.h>

std::string shutils[] = {"exit", "cd", "help", "pwd", "exec", "type", "export", "unset"};
struct {
	std::string home, user, hostname;
	std::unordered_map<std::string, std::string> aliastable;
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

int shellcmd(const std::vector<std::string>& tokens)
{
	if (tokens[0]=="unalias") {
		if (tokens[1].size()>2) std::cerr << "Why 2 or more parameters?\n";
		if (!shellenv.aliastable.erase(tokens[1])) {
			std::cerr << "Alias not found.\n";
			return 1;
		}
		return 0;
	}

	if (tokens[0]=="cd") {
		if (tokens.size()<2) {
		int opcode = (chdir(shellenv.home.c_str())==0) ? 0 : 2;
		if (opcode==2) std::cerr << "You're homeless.\n";
		return opcode;
		} else {
			std::string where = tokens[1];
			if (tokens.size()>2) std::cerr << "2 or more directories are specified, ignored\n";
			int opcode = (chdir(where.c_str())==0) ? 0 : 3;
			if (opcode==3) std::cerr << "Directory not found.\n";
			return opcode;
		}
	}

	if (tokens[0]=="help") { // will include help on a specific subject using second parameter
		std::cout << "Will be here!\n";
		return 0;
	}

	if (tokens[0]=="pwd") {
		try {
			std::string cwd = std::filesystem::current_path().string();
			std::cout << cwd << '\n';
			return 0;
		} catch (...) {
			std::cerr << "Current directory may not exist anymore, or is inaccessible.\n";
			return 4;
		}
	} // I've only implemented pwd because I was lazy

	if (tokens[0]=="exec")
		if (tokens.size()<2) {
			std::cout << "Specify a command.\n";
			return 0;
		}
		else {
			std::vector<std::string> __commandvec;
			for (size_t i=1; i<tokens.size(); i++)
				__commandvec.push_back(tokens[i]);

			std::vector<char*> argv; // store pointers to arguments
			for (std::string &have : __commandvec)
				argv.push_back(&have[0]); // grab pointer of argument
			argv.push_back(nullptr); // a terminator

			execvp(argv[0], argv.data()); // file and full command
		}

	if (tokens[0]=="type")
		if (tokens.size()<2) {
			std::cout << "To query what?\n";
			return 0;
		}
		else
		{
			if (tokens.size()>2) std::cerr << "Probably too much parameters, ignoring\n";
			std::string fetch = tokens[1];
	
			if (shellenv.aliastable.find(fetch)!=shellenv.aliastable.end()) {
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

	if (tokens[0]=="export" || tokens[0]=="env")
		if (tokens.size()<2) {
			extern char** environ;
			for (int i=0; environ[i]!=nullptr; i++)
				std::cout << environ[i] << '\n';
			return 0;
		}
		else if (tokens[0]=="export") {
			if (tokens.size()>2) std::cerr << "Too greedy, ignoring later assignments\n";
			std::string expression = tokens[1];

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
		} else {
			// wait so what function I'll use for this one
		}

	if (tokens[0]=="jobs" || tokens[0]=="fg" || tokens[0]=="bg") {
		std::cerr << "Catastrophic error. Not really, though.\n";
		return 7;
	}

	if (tokens[0]=="unset")
		if (tokens.size()<2) {
			std::cout << "What?\n";
			return 0;
		} else {
			if (tokens.size()>2) std::cerr << "Ignoring additional specifications\n";
			std::string fetch = tokens[1];
			if (unsetenv(fetch.c_str())!=0) {
				std::cerr << "Fatal error encountered while removing a variable.\n";
				return 8;
			} else return 0;
		}

	if (tokens[0]=="alias") {
		if (tokens.size()>2) std::cerr << "Ignoring unnecessary specifications.\n";

		size_t delimiter = tokens[1].find("=");
		if (delimiter==std::string::npos) {
			std::cerr << "You're missing out on something!";
			return 9;
		}

		std::string name = tokens[1].substr(0, delimiter);
		std::string val = tokens[1].substr(delimiter+1);

		shellenv.aliastable[name] = val;
	}

	return -1;
}

int echoutil(std::vector<std::string> tokens)
{
	if (tokens.size()<2)
		std::cout << "Example help text\n";
	else
		for (size_t i=1; i<tokens.size(); i++)
			std::cout << tokens[i];
	std::cout << '\n';
	return 0; // will do more stuff here
}

std::pair<std::string, bool> rtvarexp(std::string name) // runtime variable expand
{
	const char* fetch = std::getenv(name.c_str());

	if (fetch!=nullptr)
		return {(std::string)fetch, false};
	else
		return {"", true}; // evaluates to true if variable not found
}

std::vector<std::string> parse(std::string readline)
{
	std::vector<std::string> tokens;
	std::string qtbuff(""), keyword("");
	uint16_t quotestate = 0; // quote buffer and state
	
	for (size_t i=0; i<readline.size(); i++)
	{
		char fetch = readline[i];

		if (quotestate==0) {
			if (fetch=='\"') quotestate=1;
			else if (fetch=='\'') quotestate=2;
			else if (fetch!=' ') keyword += fetch;
			else {
				tokens.push_back(keyword);
				keyword.clear();
			}
		} else

		if (quotestate==1) {
			if (fetch=='\"' || fetch==' ') {
				if (keyword[0]=='$') {
					std::pair<std::string, bool> retrieval = rtvarexp(keyword.substr(1));
					if (retrieval.second) {
						std::cerr << "Error expanding variable " << keyword << ".\n";
						return {};
					}
					qtbuff += retrieval.first;
				}
				else qtbuff += keyword;

				if (fetch=='\"') {
					quotestate=0;
					tokens.push_back(qtbuff);
					qtbuff.clear();
				}	
				else qtbuff += " ";

				keyword.clear();
			}
			else keyword += fetch;
		} else

		if (quotestate==2) {
			if (fetch=='\'') {
				tokens.push_back(qtbuff);
				qtbuff.clear();
			}
			else qtbuff += fetch;
		} else

		{
			std::cerr << "Catastrophic error.\n";
			return {};
		}
	}
	if (!keyword.empty())
		tokens.push_back(keyword);
	if (!qtbuff.empty() || quotestate!=0) {
		std::cerr << "Imcomplete command.\n";
		return {};
	};
	
	return tokens;
}

int cmdexec(const std::vector<std::string>& tokens)
{
	std::vector<char*> argvect;
	for (const std::string& have : tokens)
		argvect.push_back(const_cast<char*>(&have[0]));
	argvect.push_back(nullptr);

	pid_t pid = fork();
	
	if (pid<0) {
		std::cerr << "Failed to create process.\n";
		return -3;
	}
	if (!pid) {
		execvp(argvect[0], argvect.data());

		std::cerr << "Command not found. What did you expect from typing the executable wrong?\n";
		_exit(127);
	}
	else {
		int status = 0;
		pid_t waitcode = waitpid(pid, &status, 0);
		// let's wait for the child, writing exit code to status and use default behavio

		if (waitcode==-1) {
			std::cerr << "I checked down there and there's no such child process.";
			return -2;
		}

		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		
		return -1; // I don't know how one can reach here, but leave it here
	}
}

std::string expandDir(std::string dir)
{
	if (dir.empty()) return dir;
	if (dir[0]=='~')
		return shellenv.home + dir.substr(1);

	return dir;
}

bool isScript(std::string readline)
{
	std::string filepath = expandDir(readline);

	// doesn't exist or is a path
	if (!std::filesystem::exists(filepath) || std::filesystem::is_directory(filepath))
		return false;

	std::ifstream file(filepath); // open file
	if (file.is_open()) { // if it's actually open
		std::string line1("");
		if (!getline(file, line1))
			if (line1.size()>2 // can be with shebang
			&& line1[0] == '#' // check if it's actually a shebang
			&& line1[1] == '!')
			return true;
	}

	// or check file extension
	std::string extension = std::filesystem::path(filepath).extension().string();
	if (extension==".sh" || extension==".py")
		return true;

	return false;
}

int main()
{
	fetchenv();
	std::string readline;
	int lastexit = 0;

	while (true)
	{
		// commandline heading
		std::cout << shellenv.user << "@" << shellenv.hostname << " $ ";

		std::string cwd("//");
		try {
			cwd = std::filesystem::current_path().string();
		} catch (...) { (true || false); }
		std::cout << cwd;

		if (lastexit!=0) std::cout << " [" << lastexit << "]";
		std::cout << " # ";

		// stdin error
		if (!std::getline(std::cin, readline)) return 1;

		// handle input
		if (readline=="exit") {
			lastexit = 0;
			break;
		} else if (isScript(readline)) {
			lastexit = system(expandDir(readline).c_str()); }
		else {
			std::vector<std::string> tokens = parse(readline);
			if (tokens.empty()) continue; // typed nothing
			
			int opcode = shellcmd(tokens);
			if (tokens[0]!="echo")
				lastexit = (opcode==-1) ? cmdexec(tokens) : opcode;
			else
				lastexit = echoutil(tokens);
		}
	}

	// print a newline before exiting
	std::cout << "Shell exited with error code " << lastexit << ".\n";
	std::cerr << errlookup(lastexit) << '\n';
	return 0;
}
