#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> /*핵심 헤더파일*/
#include <sys/socket.h> /*핵심 헤더파일*/
#include <iostream>
#include <thread>

using namespace std;

int sockets[5] = {0,};
int client_num = 0;

/*basic usage*/
void usage() {
	cout << "syntax: ts [-e[-b]] <port>\n";
	cout << "  -b : broadcasting\n";
	cout << "  -e : echo\n";
	cout << "sample: server 1234 -e -b\n";
}

/*parameter 처리*/
struct Param {
	bool broadcast{false};
	bool echo{false};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-b") == 0) {
				broadcast = true;
				continue;
			}
			if (strcmp(argv[i], "-e") == 0) {
				echo = true;
				continue;
			}
			port = stoi(argv[i++]);
		}
		return port != 0;
	}
} param;

void recvThread(int sd) {
	cout << "connected\n";
	static const int BUFSIZE = 65536;
	char buf[BUFSIZE];
	while (true) {
		/*recv 함수*/
		ssize_t res = recv(sd, buf, BUFSIZE - 1, 0);
		if (res == 0 || res == -1) {
			cerr << "recv return " << res << endl;
			perror("recv");
			break;
		}
		buf[res] = '\0';
        /*수정해야할 내용*/
		cout << buf;
        cout.flush();
		if (param.broadcast){ 
            /*Broadcast*/
            for(int i=0 ; i<client_num ; i++){
                res = send(sockets[i], buf, res, 0);
                if (res == 0 || res == -1) {
				cerr << "send return " << res << endl;
				perror("send");
				break;
			    }
            }
        }
		if (param.echo) {
			/*send 함수*/
			res = send(sd, buf, res, 0);
			if (res == 0 || res == -1) {
				cerr << "send return " << res << endl;
				perror("send");
				break;
			}
		}
	}
	cout << "disconnected\n";
    close(sd);
    client_num--;
}

int main(int argc, char* argv[]) {
	/*인자가 없을 경우 오류처리*/
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

	/*2. 소켓 설정*/
	int optval = 1;
	int res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		perror("setsockopt");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(param.port);

	/*3. bind 함수*/
	ssize_t res2 = ::bind(sd, (struct sockaddr *)&addr, sizeof(addr));
	if (res2 == -1) {
		perror("bind");
		return -1;
	}

	/*4. listen 함수*/
	res = listen(sd, 5); // 제한 : 5
	if (res == -1) {
		perror("listen");
		return -1;
	}

	while (true) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		
		/*accept 함수*/
		int cli_sd = accept(sd, (struct sockaddr *)&cli_addr, &len);
		if (cli_sd == -1) {
			perror("accept");
			break;
		}

        /*현재 연결된 Client 수 관리*/
        sockets[client_num] = cli_sd;
        client_num++;

		thread* t = new thread(recvThread, cli_sd);
		t->detach();
	}
	close(sd);
}
