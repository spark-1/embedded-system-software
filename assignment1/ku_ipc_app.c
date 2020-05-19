#include "ku_ipc_lib.c"

int main(){

	struct msgbuf msg1 = {
		1, "hello"
	};
	struct msgbuf msg2 = {
		1, "bye1"
	};
	struct msgbuf msg3 = {
		1, "bye2"
	};
	struct msgbuf msg4 = {
		1, "bye3"
	};
	struct msgbuf msg5 = {
		1, "bye4"
	};
	struct msgbuf msg6 = {
		1, "bye5"
	};
	struct msgbuf temp = { 0 };

	ku_msgget(1, KU_IPC_CREAT);

	ku_msgsnd(1, &msg1, sizeof(long) + 6, 0);
	ku_msgsnd(1, &msg2, sizeof(long) + 5, 0);
	ku_msgsnd(1, &msg3, sizeof(long) + 5, 0);
	ku_msgsnd(1, &msg4, sizeof(long) + 5, 0);
	ku_msgsnd(1, &msg5, sizeof(long) + 5, 0);
	ku_msgsnd(1, &msg6, sizeof(long) + 5, KU_IPC_NOWAIT);

	int sz;
	sz = ku_msgrcv(1, &temp, sizeof(long) + 6, 1, KU_MSG_NOERROR);
	printf("읽은 메세지 [%s] 크기 : %d\n", temp.text, sz);
	sz = ku_msgrcv(1, &temp, sizeof(long) + 6, 1, KU_MSG_NOERROR);
	printf("읽은 메세지 [%s] 크기 : %d\n", temp.text, sz);
	sz =ku_msgrcv(1, &temp, sizeof(long) + 6, 1, KU_MSG_NOERROR);
	printf("읽은 메세지 [%s] 크기 : %d\n", temp.text, sz);
	sz = ku_msgrcv(1, &temp, sizeof(long) + 6, 1, KU_MSG_NOERROR);
	printf("읽은 메세지 [%s] 크기 : %d\n", temp.text, sz);
	sz = ku_msgrcv(1, &temp, sizeof(long) + 6, 1, KU_MSG_NOERROR);
	printf("읽은 메세지 [%s] 크기 : %d\n", temp.text, sz);

	ku_msgclose(1);
}
