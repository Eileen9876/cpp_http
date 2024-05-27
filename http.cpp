#include "http.h"

char* str2char(string str);

/*******************************
 * Request & Response function *
 *******************************/

void Request::Clear() {
	start_line = "";
	header_fields.clear();
	message_body = "";
}

void Response::Clear() {
	version = "";
	status_code = -1;
	string status_script = "";
	header_fields.clear();
	message_body = "";
}

char* Request::GetRequestMessage() {
	// request line
	string req_msg = this->start_line + "\r\n";

	// request header fields
	for (auto i : this->header_fields) {
		req_msg += i.first + ": " + i.second + "\r\n";
	}

	// empty line
	req_msg += "\r\n";

	// message body
	req_msg += this->message_body;

	return str2char(req_msg);
}

void Response::SetResponseMessage(char* msg) {
	stringstream stream(msg);
	string line;
	enum Parts {
		status_line,
		header_fields,
		message_body
	} part = status_line;

	while (getline(stream, line)) {
		switch (part) {
		case status_line:
		{
			stringstream line_stream(line);
			line_stream >> this->version;
			line_stream >> this->status_code;
			line_stream >> this->status_script;
			part = header_fields;
			break;
		}
		case header_fields:
		{
			if (line.length() == 1) {
				part = message_body;
				break;
			}
			size_t pos = line.find(':');
			this->header_fields[line.substr(0, pos)] = line.substr(pos + 2);
			break;
		}
		case message_body:
		{
			this->message_body += line;
			break;
		}
		default:
			break;
		}
	}
}

/*********************
 *	Client function  *
 *********************/

Client::Client(string server_ip, short server_port) {
	//載入Winsock動態函式庫
	WSADATA wsdata;
	int ws = WSAStartup(MAKEWORD(2, 2), &wsdata);
	if (ws != 0)
	{
		std::cerr << "ERROR in initialize";
		return;
	}

	//建立server地址資訊
	server_addr_.sin_family = AF_INET;
	server_addr_.sin_port = htons(server_port);
	char* ip = str2char(server_ip);
	inet_pton(AF_INET, ip, &server_addr_.sin_addr.S_un.S_addr);
	delete[] ip;
}

Client::~Client() {
	WSACleanup();
}

Result Client::Get(string route) {
	//建立請求訊息
	Request request;
	request.start_line = "GET " + route + " HTTP/1.1";
	request.header_fields["Host"] = "example.com";
	request.message_body = "";
	char* req_msg = request.GetRequestMessage();

	//傳送並接收訊息
	Result result = TransmitMessage(req_msg);

	//釋放資源
	delete[] req_msg;

	return result;
}

Result Client::Post(string route, json msg_body_json) {
	//將json資料轉為字串
	string msg_body_str = msg_body_json.dump();

	//建立請求訊息
	Request request;
	request.start_line = "POST " + route + " HTTP/1.1";
	request.header_fields["Host"] = "example.com";
	request.header_fields["Accept"] = "application/json";
	request.header_fields["Content-Type"] = "application/json";
	request.header_fields["Content-Length"] = to_string(msg_body_str.length());
	request.message_body = msg_body_str;
	char* req_msg = request.GetRequestMessage();
	
	//傳送並接收訊息
	Result result = TransmitMessage(req_msg);

	//釋放資源
	delete[] req_msg;

	return result;
}

Result Client::TransmitMessage(char* req_msg) {
	//建立返回訊息
	Result result;

	//建立 client socket
	SOCKET client = socket(PF_INET, SOCK_STREAM, 0);
	if (client == INVALID_SOCKET) {
		result.SetErrorMsg("Socket Create");
		return result;
	}

	//伺服器連線
	int con = connect(client, (sockaddr*)&server_addr_, sizeof(server_addr_));
	if (con != 0) {
		closesocket(client);
		result.SetErrorMsg("Server Connect");
		return result;
	}

	//傳送訊息
	int sendbytes = send(client, req_msg, strlen(req_msg), 0);
	if (sendbytes == SOCKET_ERROR) {
		closesocket(client);
		result.SetErrorMsg("Send Message");
		return result;
	}

	//接收訊息
	string resp_msg = ""; //儲存回應訊息
	int bufsize = 1024;
	char* buffer = new char[1024];
	while (1) {
		memset(buffer, 0, bufsize);
		size_t recvbytes = recv(client, buffer, bufsize, 0);

		if (recvbytes == SOCKET_ERROR) {
			delete[] buffer;
			closesocket(client);
			result.SetErrorMsg("Receive Message");
			return result;
		}
		else if (recvbytes == 0) break;

		resp_msg.append(buffer);
	}
	delete[] buffer;

	//將接收到的訊息寫入result
	buffer = str2char(resp_msg);
	result->SetResponseMessage(buffer);
	delete[] buffer;

	return result;
}

/**********************
 *	private function  *
 **********************/

char* str2char(string str) {
	int size = str.length() + 1; // +1 預留字串結束字元'\0'
	char* ret = new char[size];
	strcpy_s(ret, size, str.c_str());
	return ret;
}

