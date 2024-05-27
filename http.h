#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <WS2tcpip.h>
#pragma comment (lib,"ws2_32.lib")
#include "json.hpp"

using json = nlohmann::json;
using string = std::string;


struct Request {
	string start_line;
	std::map<string, string> header_fields;
	string message_body;
		
	void Clear();
	char* GetRequestMessage();
};

struct Response {
	string version;
	short status_code;
	string status_script;
	std::map<string, string> header_fields;
	string message_body;

	void Clear();
	void SetResponseMessage(char* msg);
};

class Result {
public:
	Result() {
		response_.Clear();
		error_flag_ = false;
		error_msg_ = "";
	}
	Response* operator->() { return &response_; }
	bool Error() { return error_flag_; };
	string GetErrorMsg() { return error_msg_; };
	void SetErrorMsg(string msg) {
		error_msg_ = msg;
		error_flag_ = true;
	};
private:
	Response response_;
	bool error_flag_;
	string error_msg_;
};
	

class Client {
private:
	SOCKADDR_IN server_addr_;
	Result TransmitMessage(char* req_msg);
public:
	Client(string server_ip, short server_port);
	~Client();
	Result Get(string route);
	Result Post(string route, json data);
};



