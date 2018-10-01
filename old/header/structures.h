typedef struct _f_node {
	char filepath[512];
	long long offset;
	long long len;
	int flag;
	struct _f_node* next;
} f_node;

typedef struct _f_list {
	long md;
	void *bp_offset;
	int is_prefetched;
	f_node* head;
	f_node* tail;
	struct _f_list* next;
} f_list;

typedef struct _fl_list {
	f_list* head;
	f_list* tail;
} fl_list;

f_node* newFNode(char* filepath, long long offset, long long len, int flag) {
	f_node* newnode = (f_node*)malloc(sizeof(f_node));
	strncpy(newnode->filepath, filepath, sizeof(newnode->filepath));
	newnode->offset = offset;
	newnode->len = len;
	newnode->flag = flag;
	newnode->next = NULL;
	return newnode;
}

f_list* newFList(long md, void *bp_offset) {
	f_list* newlist = (f_list*)malloc(sizeof(f_list));
	newlist->md = md;
	newlist->bp_offset = bp_offset;
	newlist->is_prefetched = 0;
	newlist->head = NULL;
	newlist->tail = NULL;
	newlist->next = NULL;
	return newlist;
}
fl_list* newFLlist() {
	fl_list* newlist = (fl_list*)malloc(sizeof(fl_list));
	newlist->head = NULL;
	newlist->tail = NULL;
	return newlist;
}


void appendFNode(f_list* list, f_node* node) {
	if (list->head == NULL) {
		list->head = node;
		list->tail = node;
	}
	else {
		list->tail->next = node;
		list->tail = node;
	}
}

void appendFList(fl_list* list, f_list* item) {
	if (list->head == NULL) {
		list->head = item;
		list->tail = item;
	}
	else {
		list->tail->next  = item;
		list->tail = item;
	}
}
f_list* getFList(fl_list* list, long md, void *bp_offset) {
	f_list* temp = list->head;
	while ( temp != NULL) {
		if ( temp->md == md) {
			if ( temp->bp_offset == bp_offset) {
				return temp;
			}
		}
		temp = temp->next;
	}
	return NULL;
}
typedef struct _restore_node {
	void *offset;
	long long data;
	f_list* flist;
	struct _restore_node* next;
} r_node;

typedef struct _restore_list {
	r_node* head;
	r_node* tail;
} r_list;

r_node* newRNode(void *address, long long data, f_list* flist) {
	r_node* newnode = (r_node*)malloc(sizeof(r_node));
	newnode->offset = address;
	newnode->data = data;
	newnode->flist = flist;
	newnode->next = NULL;
	return newnode;
}

r_list* newRList() {
	r_list* newlist = (r_list*)malloc(sizeof(r_list));
	newlist->head = NULL;
	newlist->tail = NULL;
	return newlist;
}

void appendRNode(r_list* list, r_node* node) {
	if (list->head == NULL) {
		list->head = node;
		list->tail = node;
	}
	else {
		list->tail->next = node;
		list->tail = node;
	}
}

r_node* getRNode(r_list* list, void *key) {
	r_node* temp = list->head;
	while (temp != NULL) {
		if ((temp->offset) == key) {
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

typedef struct _offset_node {
	long md;
	void *bp_offset;
	struct _offset_node* next;
} offset_node;

typedef struct _offset_list {
	offset_node* head;
	offset_node* tail;
} offset_list;

offset_node* newOffsetNode(long md, void *bp_offset) {
	offset_node* newnode = (offset_node*)malloc(sizeof(offset_node));
	newnode->bp_offset = bp_offset;
	newnode->md = md;
	newnode->next = NULL;
	return newnode;
}

offset_list* newOffsetList() {
	offset_list* newlist = (offset_list*)malloc(sizeof(offset_list));
	newlist->head = NULL;
	newlist->tail = NULL;
	return newlist;
}

void appendONode(offset_list* o_list, offset_node* o_node) {
	if (o_list->head == NULL) {
		o_list->head = o_node;
		o_list->tail = o_node;
	}
	else {
		o_list->tail->next = o_node;
		o_list->tail = o_node;
	}
}

offset_node* getONode(offset_list* o_list, long md) {
	offset_node* o_node = o_list->head;
	// printf("\nKEY : %d\n", md);
	while (o_node != NULL) {
		// printf("[SEARCH]= %d, %llx, %p\n", o_node->md, o_node->bp_offset, o_node->next);
		if ((o_node->md) == md) {
			return o_node;
		}
		o_node = o_node->next;
	}
	return NULL;
}

typedef struct _pid_stack_node {
	pid_t pid;
	struct _pid_stack_node* next;
} pid_stack_node;

typedef struct _pid_stack {
	struct _pid_stack_node* top;
} pid_stack;

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
	if (stack->top == NULL) {
		return -1;
	}
	pid_stack_node* temp_node = stack->top;
	stack->top = stack->top->next;
	return temp_node->pid;
}
