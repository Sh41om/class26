#include <stdio.h>
#include "pcap.h" // pcap library
#include <arpa/inet.h> // to use inet_ntoa
#include <netinet/in.h> // to use in_addr

#define ETHER_ADDR_LEN 6

struct ethernet_header {
    u_char ether_dhost[ETHER_ADDR_LEN]; // destination MAC
    u_char ether_shost[ETHER_ADDR_LEN]; // source MAC
    u_short ether_type;
};

#define IP_HL(ip) (((ip)->ip_vhl) & 0x0f)
#define IP_V(ip) (((ip)->ip_vhl) >> 4)

struct ip_header {
    u_char ip_vhl;
    u_char ip_tos;
    u_short ip_len;
    u_short ip_id;
    u_short ip_off;
    #define IP_RF 0x8000
    #define IP_DF 0x4000
    #define IP_MF 0x2000
    #define IP_OFFMASK 0x1fff
    u_char ip_ttl;
    u_char ip_p; // IP protocol type
    u_short ip_sum;
    struct in_addr ip_src; // source IP address
    struct in_addr ip_dst; // destination IP address

};

typedef u_int tcp_seq;

struct tcp_header {
    u_short th_sport; // source TCP address
    u_short th_dport; // destination TCP address
    tcp_seq th_seq;
    tcp_seq th_ack;
    u_char th_offx2;
    #define TH_OFF(th) (((th)->th_offx2 & 0xf0) >> 4)
    u_char th_flags;
    u_short th_win;
    u_short th_sum;
    u_short th_urp;
    
};

#define SIZE_ETHERNET 14

/*libpcap 쓰고 싶었지만 오류가 많이 나서 결국 직접 eth, ip, tcp 구조체 구현해서 사용,,,*/
struct ethernet_header *ethernet;
struct ip_header *ip;
struct tcp_header *tcp;

/*execution 함수에서도 사용하기 위해 전역변수로 선언*/
struct pcap_pkthdr* header; // 패킷 헤더
const u_char* packet; // 실제 패킷
char *payload; 

u_int size_ip;
u_int size_tcp;

void usage() { // 이 프로그램의 쓰임새 출력
    printf("syntax: pcap-test <interface>\n");
    printf("sample: pcap-test wlan0\n");
}

void execution() {
    printf("\n\n");
    int i, payload_len;
    /*이더넷 패킷 정보 출력*/
    ethernet = (struct ethernet_header*) (packet);
    printf("\nMAC Source Address : ");
    for( i=0 ; i<ETHER_ADDR_LEN ; i++){
        printf("%02x ", ethernet->ether_shost[i]);
    }
    printf("\nMAC Destination Address : ");
    for( i=0 ; i<ETHER_ADDR_LEN ; i++){
        printf("%02x ", ethernet->ether_dhost[i]);
    }

    /*IP 패킷 정보 출력*/
    ip = (struct ip_header*) (packet + SIZE_ETHERNET);
    size_ip = IP_HL(ip)*4;
    printf("\nIP Source Addres : %s\n", inet_ntoa(ip->ip_src));
    printf("IP Destination Addres : %s\n", inet_ntoa(ip->ip_dst));

    /*TCP 패킷 정보 출력*/
    tcp = (struct tcp_header*) (packet + SIZE_ETHERNET + size_ip);
    size_tcp = TH_OFF(tcp)*4;
    printf("TCP Source Port : %d\n", ntohs(tcp->th_sport));
    printf("TCP Destination Port : %d\n", ntohs(tcp->th_dport));

    /*Payload 정보 출력(16바이트까지만)*/
    payload = (char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);
    payload_len = ntohs(ip->ip_len) - (size_ip + size_tcp);
    if(payload_len == 0)
        printf("< No PayLoad >");
    else{
        printf(" < PayLoad > \n");
        for( i=0 ; i<16 ; i++ ){
        printf("%02x ", payload[i]);
        }
    }
    

}

int main(int argc, char* argv[]) {


    /* dev 인자 1개가 들어오지 않으면 오류 반환*/
    if (argc != 2) {
        usage();
        return -1;
    }

    char* dev = argv[1]; 
    char errbuf[PCAP_ERRBUF_SIZE]; //에러 메시지가 저장되는 문자열

    pcap_t* handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf); //패킷 잡는 함수

    if (handle == nullptr) { // pcap_open_live가 정상적으로 작동되지 않았을 경우 오류 코드 
        fprintf(stderr, "pcap_open_live(%s) return nullptr - %s\n", dev, errbuf);
        return -1;
    }

    while (1) {

        int res = pcap_next_ex(handle, &header, &packet);

        if (res == 0) continue; // 시간초과 발생

        if (res == -1 || res == -2) { // 오류 발생
            printf("pcap_next_ex return %d(%s)\n", res, pcap_geterr(handle));
            break;
        }

        execution();
    }

}