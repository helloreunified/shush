/*
Copyright (c) 2026 helloreunified

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
#include "linenoise.h"

std::string shellutils[] = {"exit", "cd", "help", "pwd", "exec", "type", "export", "unset"};
struct {
	std::string home, user, hostname;
	std::unordered_map<std::string, std::string> aliastable;
} shellenv; // contain shell-specific environment tables

void loadalias()
{
	std::filesystem::path where = std::filesystem::path(shellenv.home) / ".shushcfg" / "alias.shush";
	std::ifstream file(where.string());
	if (file.is_open()) {
		std::string buffer;
		while (getline(file, buffer)) {
			size_t delimiter = buffer.find("=");
			if (delimiter==std::string::npos)
				std::cerr << "Malformed alias found. Will not include it.\n";
			else {
				std::string name = buffer.substr(0, delimiter);
				std::string val = buffer.substr(delimiter+1);
				shellenv.aliastable[name] = val;
			}
		}
		file.close();
	} else
		std::cerr << "Alias table file not found. All alias were not loaded.\n";
}

bool writealias()
{
	std::filesystem::path where = std::filesystem::path(shellenv.home) / ".shushcfg" / "alias.shush";
	std::ofstream file(where.string());
	if (file.is_open()) {
		for (std::pair<std::string, std::string> getalias : shellenv.aliastable)
			file << getalias.first << "=" << getalias.second << "\n";
		file.close(); return true;
	} else {
		std::cerr << "Alias table file not found. All alias were not saved, please return to your terminal and exit again if you don't want it saved.\n";
		return false;
	}
}

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
		return "";
	else
		return "Catastrophic error.\n";
}

bool isbuiltin(std::string file)
{
	for (long unsigned int i=0; i<size(shellutils); i++)
		if (file==shellutils[i])
			return true;
	return false;
}

std::string foundexe(std::string file)
{
	std::string path = "PATH";
	const char* fetchpath = std::getenv(path.c_str());
	if (fetchpath==nullptr) return "I found something";

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
		if (tokens.size()>2) std::cerr << "Why 2 or more parameters?\n";
		if (tokens.size()<2) {
			std::cerr << "Not enough parameters, why?";
			return 1;
		}
		
		if (!shellenv.aliastable.erase(tokens[1]))
			std::cerr << "Reminder that your specified alias isn't found.\n";
			
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
			std::cerr << "Unimplemented\n";
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
		if (tokens.size()<2) {
			std::cerr << "Not enough arguments.";
			return 9;
		}
		
		size_t delimiter = tokens[1].find("=");
		if (delimiter==std::string::npos) {
			std::cerr << "Where's your equal sign?";
			return 10;
		}

		std::string name = tokens[1].substr(0, delimiter);
		std::string val = tokens[1].substr(delimiter+1);

		shellenv.aliastable[name] = val;

		return 0;
	}

	if (tokens[0]=="echo") {
		if (tokens.size()<2)
			std::cout << "Example help text\n";
		else
			for (size_t i=1; i<tokens.size(); i++)
				std::cout << tokens[i] << " ";
		std::cout << '\n';
		return 0; // Will reimplement GNU echo
	}

	return -1;
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
	std::string qtbuff(""); // quote buffer
	uint16_t quotestate = 0; // quote state
	
	for (size_t i=0; i<readline.size(); i++)
	{
		char fetch = readline[i];

		if (fetch=='#' && quotestate==0)
			break;

		if (quotestate==0) {
			if (fetch=='\"') quotestate=1;
			else if (fetch=='\'') quotestate=2;
			else if (fetch!=' ') qtbuff += fetch;
			else {
				if (!qtbuff.empty()) {
					tokens.push_back(qtbuff);
					qtbuff.clear();
				}
			}
		} else

		if (quotestate==1) {
			if (fetch=='\"') {
				tokens.push_back(qtbuff);
				qtbuff.clear();
				quotestate = 0;
			}
			else if (fetch=='$') {
				size_t where = i+1;
				bool curlybraces = false;
				if (readline[where]=='{') {
					curlybraces = true;
					where++;
				}
				
				std::string getvariablename("");
				while (readline[where]=='_' ||
					(readline[where]>='0' && readline[where]<='9') ||
					(readline[where]>='a' && readline[where]<='z') ||
					(readline[where]>='A' && readline[where]<='Z') ) {
					getvariablename += readline[where];
					where++;
				}

				if (curlybraces) {
					if (readline[where]!='}') {
						std::cerr << "Imcomplete syntax. A curly brace is what you all needed to break a command like this.\n";
						return {};
					}
					where++; // if it's correct syntax
				}

				if (getvariablename=="")
					qtbuff += readline[i];
				else {
					std::pair<std::string, bool> getresult = rtvarexp(getvariablename);
					if (getresult.second) {
						std::cerr << "Invalid variable name: " << getvariablename << '\n';
						return {};
					}
					qtbuff += getresult.first;
					i = where-1;
				}
			}
			else
				qtbuff += fetch;
		} else

		if (quotestate==2) {
			if (fetch=='\'') {
				tokens.push_back(qtbuff);
				qtbuff.clear();
				quotestate = 0;
			}
			else qtbuff += fetch;
		} else

		{
			std::cerr << "Catastrophic error.\n";
			return {};
		}
	}
	if (!qtbuff.empty())
		tokens.push_back(qtbuff);
	
	if (quotestate!=0) {
		std::cerr << "Imcomplete command due to unmatched quote.\n";
		return {};
	}
	
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

		std::cerr << "ouu shi 👀\n";
		_exit(127);
	}
	else {
		int status = 0;
		pid_t waitcode = waitpid(pid, &status, 0);

		if (waitcode==-1) {
			std::cerr << "fym ouu shi";
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
		file.close(); // cleanup
	}

	// or check file extension
	std::string extension = std::filesystem::path(filepath).extension().string();
	if (extension==".sh" || extension==".py")
		return true;

	return false;
}

void prepcfg()
{
	std::filesystem::path cfgpath = std::filesystem::current_path() / ".shushcfg";
	// as a portable shell, this leaves a very inherent reminder in that

	if (!(std::filesystem::exists(cfgpath) && std::filesystem::is_directory(cfgpath)))
		cmdexec({"mkdir", cfgpath.string()});

	std::filesystem::path txtcfg = cfgpath / "config.shush";
	if (!(std::filesystem::exists(txtcfg) && !std::filesystem::is_directory(txtcfg)))
		cmdexec({"touch", txtcfg.string()});

	std::filesystem::path aliascfg = cfgpath / "alias.shush";
		if (!(std::filesystem::exists(aliascfg) && !std::filesystem::is_directory(aliascfg)))
			cmdexec({"touch", aliascfg.string()});
}

bool notify_no_prompt_once = false;
std::string readprompt(int lastexit) // this varies
{
	// predefine some necessity
	std::filesystem::path txtcfg = std::filesystem::current_path() / ".shushcfg" / "config.shush";

	std::string cwd; // current working directory
	try { cwd = std::filesystem::current_path().string(); }
	catch (...) { cwd = "//"; } // that's a quite nice catastrophe

	bool foundpromptcfg = false;
	std::ifstream is(txtcfg.string());
	if (is.is_open()) {
		std::string fetchcfg;
		while (getline(is, fetchcfg)) // i'm not going to play a guessing game here, this architecture will be adbandoned the moment i added fixed-size config at the end of the binary
			if (fetchcfg.rfind("prompt=", 0)) {
				fetchcfg = fetchcfg.substr(7);
				foundpromptcfg = true; break;
			}
	}
	
	std::string exampleprompt = shellenv.user + "@" + shellenv.hostname + " $ " + cwd;
	if (lastexit!=0) exampleprompt += " [" + std::to_string(lastexit) + "]";
	exampleprompt += " % ";

	// return accordingly
	if (!(std::filesystem::exists(txtcfg) && !std::filesystem::is_directory(txtcfg))
		|| !foundpromptcfg) {
		if (!notify_no_prompt_once) {
			std::cout << "A small warning that config.shush failed to create or found, or the specific configuration wasn't found.\n";
			notify_no_prompt_once = true;
		}
		return exampleprompt;
	} else {
		std::cout << "Note that any prompt over 2048 characters in raw will be cut off in snapshot 6 or later.\n";
		std::string userprompt("miku > ");
		/* we need to define some things before we do things
			{user} == username
			{host} == hostname
			{cwd} == current working directory
			ASCII color sequence == color will be later!
		*/

		size_t posfind = 0;
		while ((posfind=userprompt.find("{user}"))!=std::string::npos)		
			userprompt.replace(posfind, 6, shellenv.user);
		while ((posfind=userprompt.find("{host}"))!=std::string::npos)
			userprompt.replace(posfind, 6, shellenv.hostname);
		while ((posfind=userprompt.find("{cwd}"))!=std::string::npos)
			userprompt.replace(posfind, 5, cwd);

		// later	

		return userprompt;
	}
}

int main()
{
	prepcfg(); fetchenv(); loadalias();
	int lastexit = 0;
	linenoiseHistorySetMaxLen(20);

	while (true)
	{
		// request defined prompt
		std::string reqprompt = readprompt(lastexit);

		// request reference to input
		char* inputbuffer = linenoise(reqprompt.c_str());

		// you used ^D
		if (inputbuffer==nullptr) {
			lastexit = 0;
			break;
		}

		// read input line
		std::string readline = (std::string)inputbuffer;
		// release memory
		free(inputbuffer);

		// handle input
		if (readline.empty()) continue; // noise pollution
		linenoiseHistoryAdd(readline.c_str()); // add it

		// resolve input
		if (readline=="exit") {
			lastexit = 0;
			break;
		} else
		if (isScript(readline)) {
			lastexit = system(expandDir(readline).c_str());
		} else
		{
			std::vector<std::string> tokens = parse(readline);
			if (tokens.empty()) continue; // if you didn't type anything

			// test against shell commands first then find executable later
			int useshellcmd = shellcmd(tokens);
			lastexit = (useshellcmd==-1) ? cmdexec(tokens) : useshellcmd;		
		}
	}

	// print a newline before exiting
	std::cout << "Process exited with error code " << lastexit << ".\n";
	std::cerr << errlookup(lastexit);

	if (!writealias()) { prepcfg(); writealias(); }
	
	return lastexit;
}
