#include "keyevent.h"
//single key buffer
static char key=NULL_CASE;
char empty = 1;//empty=1 --> TRUE

void keyevent_push(char c) {
	key = c;
	empty = 0;
}
char keyevent_pop() {
	empty = 1;
	return key;
}
char keyevent_is_empty() {
	return empty;
}
