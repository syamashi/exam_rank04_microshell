#include <libc.h>
#define CDARG "error: cd: bad arguments"
#define CDERR "error: cd: cannot change directory to "
#define FATAL "error: fatal"
#define EXERR "error: cannot execute "

typedef struct s_sh{
	char *cmd[1024];
	int std_in;
	int std_out;
	struct s_sh *next;
} t_sh;

int ft_strlen(char *str){
	int ret = 0;
	while (*str){
		++ret;
		++str;
	}
	return ret;
}

int put_error(char *str, char *str2){
	write(2, str, ft_strlen(str));
	if (str2){
		write(2, str2, ft_strlen(str2));
	}
	write(2, "\n", 1);
	
	return 1;
}

t_sh *init_sh(){
	t_sh *ret;

	ret = (t_sh*)malloc(sizeof(t_sh));
	if (!ret){
		exit(put_error(FATAL, NULL));
	}
	ret->std_in = 0;
	ret->std_out = 1;
	ret->next = NULL;

	return ret;
}

void init_pipe(t_sh *sh){
	int pip[2];
	
	sh->next = init_sh();
	pipe(pip);
	sh->next->std_in = pip[0];
	sh->std_out = pip[1];
}


void vdebug(t_sh *sh){
	for(int i=0; i<10; ++i){
		if (sh->cmd[i] == NULL) break;
		printf("cmd[%d]:%s\n", i, sh->cmd[i]);
	}
}

void exec_sh(t_sh *sh, int start, int i){
	// vdebug(sh);
	
	// cd
	if (!strcmp(sh->cmd[0], "cd")){
		if (sh->cmd[2] || sh->cmd[1] == NULL){
			put_error(CDARG, NULL);
		}
		else if (chdir(sh->cmd[1])){
			put_error(CDERR, sh->cmd[1]);
		}
		return ;
	}
	
	int pid = fork();
	if (pid < 0){
		exit(put_error(FATAL, NULL));
	}
	// child
	if (pid == 0){
		if (sh->std_out != 1){
			close(sh->next->std_in);
			dup2(sh->std_out, 1);
		}
		if (sh->std_in != 0){
			dup2(sh->std_in, 0);
		}
		execve(sh->cmd[0], sh->cmd, NULL);
		exit(put_error(EXERR, sh->cmd[0]));
	}
	// parent
	if (sh->std_out != 1){
		close(sh->std_out);
	}
	if (sh->std_in != 0){
		close(sh->std_in);
	}
	if (sh->std_out == 1){
		while (start < i){
			waitpid(0, NULL, 0);
			++start;
		}
	}
}

int main(int argc, char **argv){
	t_sh *sh = NULL;
	int start = 1;
	int i = 1;

	while(i < argc){
		while (!strcmp(argv[i], ";")) ++i;
		if (i == argc) break;
		if (sh == NULL){
			sh = init_sh();
		}
		// addcmd
		int j = 0;
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|")){
			sh->cmd[j] = argv[i];
			++j;
			++i;
		}
		sh->cmd[j] = NULL;
		// pipe
		if (argv[i] && !strcmp(argv[i], "|")){
			init_pipe(sh);
		}
		
		// exec
		exec_sh(sh, start, i);
		
		// free
		if (sh->std_out != 1){
			t_sh *tmp = sh;
			sh = sh->next;
			free(tmp);
			tmp = NULL;
		}
		else{
			free(sh);
			sh = NULL;
			start = i + 1;
		}
		++i;
	}
	return 0;
}


