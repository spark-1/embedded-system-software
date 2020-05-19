#include "ku_ipc_lib.c"

int main() {

        struct msgbuf temp = {
                0
        };

	ku_msgget(1, KU_IPC_CREAT);
	ku_msgrcv(1, &temp, sizeof(long) + 6, 1, 0);

	printf("받은 메세지 [%s]\n", temp.text);
}
