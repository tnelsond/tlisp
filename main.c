#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*#include <dlfcn.h>*/

#define INT 1
#define FLOAT 2
#define STRING 4
#define OP 8
#define VAR 16
#define FUNC 32
#define NONE 64
#define DONE 128

#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4
#define PRINT 5 
#define EQ 6 
#define LOADLIB 7

#define BUFFER_SIZE 256
char buffer[BUFFER_SIZE];

typedef struct t_val{
	char type;
	union{
		int i;
		double f;
		char *s;
	}u;
}t_val;

typedef struct t_node{
	t_val val;
	struct t_node *next;	
	struct t_node *child;
} t_node;

typedef void (*t_func)(t_val *, t_val);

void t_add(t_val *a, t_val b) {
	if(a->type == NONE){
		a->type = b.type;
		a->u = b.u;
	}
	else if(a->type & b.type & INT){
		a->u.i += b.u.i;
	}
	else if(a->type & b.type & FLOAT){
		a->u.f += b.u.f;
	}
	else if(a->type & INT && b.type & FLOAT){
		a->type = FLOAT;
		a->u.f = a->u.i + b.u.f;
	}
	else if(a->type & FLOAT && b.type & INT){
		a->u.f += b.u.i;
	}
}

void t_sub(t_val *a, t_val b) {
	if(a->type == NONE){
		a->type = b.type;
		a->u = b.u;
	}
	else if(a->type & b.type & INT){
		a->u.i -= b.u.i;
	}
	else if(a->type & b.type & FLOAT){
		a->u.f -= b.u.f;
	}
	else if(a->type & INT && b.type & FLOAT){
		a->type = FLOAT;
		a->u.f = a->u.i - b.u.f;
	}
	else if(a->type & FLOAT && b.type & INT){
		a->u.f -= b.u.i;
	}
}

void t_eq(t_val *a, t_val b) {
	if(a->type == NONE){
		a->type = b.type;
		a->u = b.u;
	}
	else if(a->type & b.type & INT){
		a->u.i = b.u.i == a->u.i;
	}
	else if(a->type & b.type & FLOAT){
		a->type = INT;
		a->u.i = a->u.f == b.u.f;
	}
	else if(a->type & INT && b.type & FLOAT){
		a->u.i = (double)a->u.i == b.u.f;
	}
	else if(a->type & FLOAT && b.type & INT){
		a->type = INT;
		a->u.i = a->u.f == (double)b.u.i;
	}
}

void t_mul(t_val *a, t_val b) {
	if(a->type == NONE){
		a->type = b.type;
		a->u = b.u;
	}
	else if(a->type & b.type & INT){
		a->u.i *= b.u.i;
	}
	else if(a->type & b.type & FLOAT){
		a->u.f *= b.u.f;
	}
	else if(a->type & INT && b.type & FLOAT){
		a->type = FLOAT;
		a->u.f = a->u.i * b.u.f;
	}
	else if(a->type & FLOAT && b.type & INT){
		a->u.f *= b.u.i;
	}
}

/*
void t_runfromlib(t_val *a, t_val b){
	if(b.type & STRING){
		void *fn = dlsym(lib, b.u.s);
	}
	else{
		fprintf(stderr, "ERROR: runfromlib expects a string as an argument!\n");
	}
}
*/

void t_loadlib(t_val *a, t_val b) {
	if(b.type & STRING){
		printf("Loading Library %s...\n", b.u.s);
		/*
		lib = dlopen(b.u.s, RTLD_LAZY);
		if(!lib){
			fprintf(stderr, "ERROR: Can't load library. %s\n", dlerror());
		}
		*/
	}
	else{
		fprintf(stderr, "ERROR: loadlib expects a string as an argument!\n");
	}
}

void t_div(t_val *a, t_val b) {
	if(a->type == NONE){
		a->type = b.type;
		a->u = b.u;
	}
	else if(a->type & b.type & INT){
		a->u.i /= b.u.i;
	}
	else if(a->type & b.type & FLOAT){
		a->u.f /= b.u.f;
	}
	else if(a->type & INT && b.type & FLOAT){
		a->type = FLOAT;
		a->u.f = a->u.i / b.u.f;
	}
	else if(a->type & FLOAT && b.type & INT){
		a->u.f /= b.u.i;
	}
}

void t_print(t_val *a, t_val b) {
	if(b.type & INT){
		printf("%d ", b.u.i);
	}
	else if(b.type & FLOAT){
		printf("%f ", b.u.f);
	}
	else if(b.type & STRING){
		printf("%s ", b.u.s);
	}
}

t_val t_eval(struct t_node *root){
	t_func func;
	t_val ret = {NONE, 0};
	struct t_node *child = root->child;
	if(child == NULL){
		return root->val;
	}
	if(root->val.type & OP){
		if(root->val.u.i == ADD)
			func = t_add;
		else if(root->val.u.i == SUB)
			func = t_sub;
		else if(root->val.u.i == MUL)
			func = t_mul;
		else if(root->val.u.i == DIV)
			func = t_div;
		else if(root->val.u.i == EQ)
			func = t_eq;
		else if(root->val.u.i == LOADLIB)
			func = t_loadlib;
		else{
			func = t_print;
		}
	}
	while(child){
		if(child->val.type & OP){
			(*func)(&ret, t_eval(child));
		}
		else{
			(*func)(&ret, child->val);
		}
		child = child->next;
	}
	return ret;
}

int t_parse(t_node *root){
	int i = 0;
	t_node *node = root;
	t_node **next = &node->child;
	int state = OP;
	int c;
	while((c = getchar()) != '\n' && c != EOF){
		if(state & STRING && !(state & DONE)){
			if(c == '"'){
				state |= DONE;	
			}
			else{
				buffer[i++] = c;
			}
		}
		else{
			if(c == '('){
				*next = malloc(sizeof(struct t_node));
				node = *next;
				node->next = NULL;
				node->child = NULL;
				next = &node->next;
				if(t_parse(node)){
					return 1;
				}
			}
			else if(isspace(c) || c == ')'){
				if(i > 0){
					buffer[i] = '\0';
					if(state & OP){
						node->val.type = OP;
						if(strcmp(buffer, "+") == 0){
							node->val.u.i = ADD;
						}
						else if(strcmp(buffer, "-") == 0){
							node->val.u.i = SUB;
						}
						else if(strcmp(buffer, "/") == 0){
							node->val.u.i = DIV;
						}
						else if(strcmp(buffer, "*") == 0){
							node->val.u.i = MUL;
						}
						else if(strcmp(buffer, "=") == 0){
							node->val.u.i = EQ;
						}
						else if(strcmp(buffer, "print") == 0){
							node->val.u.i = PRINT;
						}
						else if(strcmp(buffer, "loadlib") == 0){
							node->val.u.i = LOADLIB;
						}
						else{
							node->val.type |= STRING;
							node->val.u.s = malloc(strlen(buffer));
							strcpy(node->val.u.s, buffer);
						}
						next = &node->child;
					}
					else{
						*next = malloc(sizeof(t_node));
						node = *next;
						node->next = NULL;
						node->child = NULL;
						next = &node->next;
						if(state & STRING){
							node->val.type = STRING;
							node->val.u.s = malloc(strlen(buffer));
							strcpy(node->val.u.s, buffer);
						}
						else if(state & VAR){
							node->val.type = VAR;	
						}
						else if(state & FLOAT){
							node->val.type = FLOAT;	
							node->val.u.f = atof(buffer);
						}
						
						else{
							node->val.type = INT;	
							node->val.u.i = atoi(buffer);
						}
					}
					state = 0;
					i = 0;
				}
				if(c == ')'){
					return 0;
				}
			}
			else{
				buffer[i++] = c;
				if(c == '.'){
					state |= FLOAT;
				}
				else if(c == '"'){
					state |= STRING;
					--i;
				}
				else if(!isdigit(c)){
					state |= VAR;
				}
			}
		}
	}
	if(c == EOF){
		return 1;
	}
	return 0;
}

void t_debug(t_node *root){
	t_node *node = root;
	do{
		if(node->child){
			printf("(");
		}
		printf("%d ", node->val.type);
		if(node->child){
			t_debug(node->child);
			printf("\b) ");
		}
	}while((node = node->next));
}

int main(int argc, char **argv){
	t_node root = {{OP, PRINT}, NULL, NULL};
	int end = 0;
	while(!end){
		printf("\n>> ");
		end = t_parse(&root);
		t_debug(&root);
		t_eval(&root);
	}

	return 0;
}
