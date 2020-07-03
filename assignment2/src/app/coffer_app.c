#include "switch_lib.c"
#include "step_motor_lib.c"
#include "speaker_lib.c"

int my_password[5][2] = {{1, 0}, {2, 1}, {3, 0}, {3, 1}, {1, 0}};

int main() {

	int flag = 0; // 0닫힘, 1열림
	int incorrect = 0;
	int count = 0;
	int key = 0;
	int ret = 0;
	int distance = 0;

	printf("큐 넘버를 고르세요: ");
	scanf("%d", &key);
	
	switch_queue_close(key);
	switch_queue_get(key);

	while(1) {
		if (flag == 0) { // 8초마다 비번 확인
			printf("비밀번호를 입력하세요\n");
			sleep(8);
			count = 0;
			incorrect = 0;
			while(1) {
				ret = switch_recv(key);
				distance = ultrasonic_recv(key);
				if (ret < 0 || count >= 5) {
					break;
				}
				printf("스위치:%d 거리:%d\n", ret, distance);
				if (distance >= 15) {
					distance = 1;
				}
				else {
					distance = 0;
				}
				if (my_password[count][0] != ret || my_password[count][1] != distance) {
					incorrect = 1;
				}
				count++;
			}
			if (!incorrect && (count == 5)) { // 열리는 조건 
				flag = 1;
				speaker_correct();
				step_motor_open();			
			}
			else if (count != 0) {
				speaker_incorrect();
			}
		}
		else { // 열린 상태이면
			while(1) {
				ret = switch_recv(key);
				distance = ultrasonic_recv(key);
				if (ret > 0) {
					flag = 0;
					step_motor_close();
					break;
				}
			}
		}
	}

	switch_queue_close(key);
}
