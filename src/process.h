#ifndef PROCESS_H
#define PROCESS_H

void factor();
void term();
void expression();
void assignment();
void block(char ending, const char *label_continue, const char *label_break);
void do_program();

#endif
