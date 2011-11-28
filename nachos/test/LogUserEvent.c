#include "syscall.h"



int main()
{
	NachosUserEvent("LogUserEvent", "UserEvent",  4);

	return 0;
}
