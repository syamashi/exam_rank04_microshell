#include <libc.h>
#define FATAL "error: fatal"
#define CDARG "error: cd: bad arguments"
#define CDERR "error: cd: cannot change directory to "
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
	if (!ret)
		exit(put_error(FATAL, NULL));
	ret->std_in = 0;
	ret->std_out = 1;
	ret->next = NULL;

	return ret;
}

void init_pipe(t_sh *sh){
	int pip[2];

	pipe(pip);
	sh->next = init_sh();
	sh->next->std_in = pip[0];
	sh->std_out = pip[1];
}

void exec_sh(t_sh *sh, int start, int i){
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
		execve(sh->cmd[0], sh->cmd, NULL); // env:NULL
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
			waitpid(0, NULL, 0); // muriyari
			++start;
		}
	}
}

int main(int argc, char **argv){

	t_sh *sh = NULL;
	int i = 1;
	int start = 1;
	
	while (i < argc){
		// skip ;
		while (!strcmp(argv[i], ";")) ++i;
		if (i == argc) break;
		if (sh == NULL) sh = init_sh();
		
		// init_cmd
		int j = 0;
		while (argv[i] && strcmp(argv[i], ";") && strcmp(argv[i], "|")){
			sh->cmd[j] = argv[i];
			++i;
			++j;
		}
		
		// init_pipe
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
}
