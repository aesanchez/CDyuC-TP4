#ifndef KEYEVENT_H_
#define KEYEVENT_H_

void keyevent_push(char);
char keyevent_pop(void);
char keyevent_is_empty(void);

#endif