#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> /*핵심 헤더파일*/
#include <sys/socket.h> /*핵심 헤더파일*/
#include <iostream>
#include <thread>

using namespace std;

void usage() {
	cout << "syntax: tc [-an] <ip> <port>\n";
	cout << "  -an: auto newline\n";
	cout << "sample: client 127.0.0.1 1234\n";
}

struct Param {
	bool autoNewline{false};
	struct in_addr ip{0};
	uint16_t port{0};

	/*parameter 처리*/
	bool parse(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-an") == 0) {
				autoNewline = true;
				continue;
			}
			int res = inet_pton(AF_INET, argv[i++], &ip);
			switch (res) {
				case 1: break;
				case 0: cerr << "not a valid network address\n"; return false;
				case -1: perror("inet_pton"); return false;
			}
			port = stoi(argv[i++]);
		}
		return (ip.s_addr != 0) && (port != 0);
	}
} param;

void recvThread(int sd) {
	cout << "connected\n";
	static const int BUFSIZE = 65536;
	char buf[BUFSIZE];
	while (true) {
		ssize_t res = recv(sd, buf, BUFSIZE - 1, 0);
		if (res == 0 || res == -1) {
			cerr << "recv return " << res << endl;
			perror("recv");
			break;
		}
		buf[res] = '\0';
		if (param.autoNewline)
			cout << buf << endl;
		else {
			cout << buf;
			cout.flush();
		}
	}
	cout << "disconnected\n";
    close(sd);
	exit(0);
}

int main(int argc, char* argv[]) {
	/*인자 없는 경우 예외처리*/
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}

	/*1. 소켓 함수 생성*/
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		return -1;
	}

	/*2. 소켓 함수 설정*/
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(param.port);
	addr.sin_addr = param.ip;
	memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

    /*3. connet 함수*/
	int res = connect(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (res == -1) {
		perror("connect");
		return -1;
	}

	thread t(recvThread, sd);
	t.detach();

	/*문자열 입력받는 부분*/
	while (true) {
		string s;
		getline(cin, s);
        /*send 함수*/
		ssize_t res = send(sd, s.c_str(), s.size(), 0);
		if (res == 0 || res == -1) {
			cerr << "send return " << res << endl;
			perror("send");
			break;
		}
	}
	close(sd);
}
