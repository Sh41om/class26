#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define input_MAX 10 // 최대 입력 가능 .bin 파일 수는 10개로 한정

uint32_t value[input_MAX]={0}; // bin 파일 안의 4byte 값을 넣을 변수 선언 및 초기화



int openfile(int count, const char* filename){

	FILE* fp;
	fp = fopen(filename, "rb"); //bin 파일 열기

	if (fp == NULL) //파일이 없을 경우 예외 처리
	{
		printf("\nFAILED!\n");
		return 1;

	}

	fread(&value[count-1], 4, 1, fp); // bin 파일에서 4byte 읽어서 value[count-1]에 저장

	printf("%x\n", value[count-1]);

	return 0;

}

/*uint32_t 타입으로 인자를 받아서 함수를 수행해도 되지만, 전역변수로 uint32_t형 value를 사용하는 방향.*/
void ntohl(int count){ //NBO to HBO
	uint32_t n1 = (value[count] & 0xff000000) >> 24;
	uint32_t n2 = (value[count] & 0x00ff0000) >> 8;
	uint32_t n3 = (value[count] & 0x0000ff00) << 8;
	uint32_t n4 = (value[count] & 0x000000ff) << 24;

	value[count] = n1 | n2 | n3 | n4;

	return;
	
}

int main(int argc, const char* argv[]) {

	uint32_t result = 0; //최종 결과값 넣을 32비트 unsigned int

	// printf("\nargc :%d\n", argc); , argc 체크

	for(int i=1; i < argc ; i++){ //argv[1]~argv[n](filename)에 대해  파일 오픈

		openfile(i, argv[i]);
	}

	printf("\n------------\n\n");

	for(int i=0;i<(argc-1);i++){ // NBO to HBO !

		ntohl(i);
		printf("%#02x\n", value[i]);
	}

	for(int i=0;i<(argc-1);i++){ // value[n]의 총합을 result에 저장, 최종 결과물!
		
		result += value[i];

	}

	for(int i=0;i<(argc-1);i++){ // 연산 수행 및 결과 출력

		if(i!=(argc-2))
			printf("%d(%#x) + ", value[i], value[i]);
		else
			printf("%d(%#x) = ", value[i], value[i]);
	}

	printf("%d(%#x) 입니다!!\n\n", result, result);


}