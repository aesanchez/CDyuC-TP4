#ifndef KEYEVENT_H_
#define KEYEVENT_H_
// TODO ver para que era el caso_nulo
#define NULL_CASE '~' 

void keyevent_push(char);
char keyevent_pop(void);
char keyevent_is_empty(void);

#endif