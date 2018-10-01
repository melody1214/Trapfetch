#include "structure.h"

pid_stack* newPidStack() {
	pid_stack* newStack = (pid_stack*)malloc(sizeof(pid_stack));
	newStack->top = NULL;
	return newStack;
}

void push(pid_stack* stack, pid_t pid) {
	pid_stack_node* temp_node = (pid_stack_node*)malloc(sizeof(pid_stack_node));
	temp_node->pid = pid;
	if ( stack->top != NULL) {
		temp_node->next = stack->top;
	}
	else {
		temp_node->next = NULL;
	}
	stack->top = temp_node;
}

pid_t pop(pid_stack* stack) {
    if(stack->top == NULL){
        return -1;
    }
    pid_stack_node* temp_node = stack->top;
    stack->top = stack->top->next;
    return temp_node->pid;
}

pid_t getStackpid(pid_stack* stack, pid_t pid) {
	pid_stack_node* temp_node = NULL;
	if (stack->top != NULL) {
		temp_node = stack->top;
		if (temp_node->next != NULL) {
			do {
				if (pid == temp_node->pid) {
					return pid;
				}
				temp_node = temp_node->next;
			} while(temp_node != NULL);
		}
		else {
			if (pid == temp_node->pid)
				return pid;
		}
	}
	return -1;
}
