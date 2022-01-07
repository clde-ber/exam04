#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

static int pipe_end = 0;

int ft_strlen(char *str)
{
    int i = 0;

    while (str[i])
        i++;
    return i;
}

int ft_strcmp(char *s1, char *s2)
{
    int i = 0;

    while (s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return (s1[i] - s2[i]);
}

char* ft_strdup(char* str)
{
    int i = 0;
    char *ret = malloc(sizeof(char) * (ft_strlen(str) + 1));

    while (str[i])
    {
        ret[i] = str[i];
        i++;
    }
    ret[i] = '\0';
    return ret;
}

void free_double_tab(char **tab)
{
    int i = 0;

    while (tab[i])
    {
        free(tab[i]);
        tab[i] = NULL;
        i++;
    }
    free(tab);
    tab = NULL;
}

void free_double_tab_len(int **tab, int len)
{
    int i = 0;

    while (i < len)
    {
        free(tab[i]);
        tab[i] = NULL;
        i++;
    }
    free(tab);
    tab = NULL;
}

void free_triple_tab(char ***tab)
{
    int i = 0;

    while (tab[i])
    {
        free_double_tab(tab[i]);
        i++;
    }
    free(tab);
    tab = NULL;
}

int double_tab_len(char**tab)
{
    int i = 0;

    while (tab[i])
        i++;
    return i;
}

int triple_tab_len(char ***tab)
{
    int i = 0;

    while (tab[i])
        i++;
    return i;
}

int count_elements(char **args, char *element)
{
    int i = 0;
    int count = 0;

    while (args[i])
    {
        if (ft_strcmp(args[i], element) == 0)
            count++;
        i++;
    }
    return ++count;
}

int exec(char *cmd, char **cmd_list, char **env, int has_pipe, int **pipefd, int i)
{
    pid_t pid = 0;
    int ret = 1;
    int status = 0;

    if ((pid = fork()) == -1)
        return 1;
    if (pid == 0)
    {
        if (has_pipe)
        {
            close(pipefd[i - 1][1]);
            dup2(pipefd[i - 1][0], STDIN_FILENO);
            if (!pipe_end)
            {
                close(pipefd[i][0]);
                dup2(pipefd[i][1], STDOUT_FILENO);
            }
        }
        if ((ret = execve(cmd, cmd_list, env)) == -1)
            exit(status);
        exit(status);
    }
    else
    {
        if (has_pipe)
        {
            close(pipefd[i - 1][1]);
            close(pipefd[i - 1][0]);
        }
        waitpid(pid, &status, 0);
        if (has_pipe && pipe_end)
        {
            close(pipefd[i][0]);
            close(pipefd[i][1]);
            close(pipefd[0][0]);
            close(pipefd[0][1]);
        }
        if (WIFEXITED(status))
            ret = WEXITSTATUS(status);
    }
    return ret;
}

int exec_pipes(char ***cmd, char **env, int len)
{
    int i = 0;
    int ret = 0;
    int **pipefd = malloc(sizeof(int *) * (len + 2));

    for (int x = 0; x < len + 2; x++)
        pipefd[x] = malloc(sizeof(int) * 2);
    pipe_end = 0;
    while (cmd[i])
    {
        if (i == 0)
        {
            pipe(pipefd[i]);
            pipe(pipefd[i + 1]);
        }
        else
        {
            if (i == len - 1)
                pipe_end = 1;
            pipe(pipefd[i + 1]);
        }
        ret = exec(cmd[i][0], cmd[i], env, 1, pipefd, i + 1);
        i++;
    }
    free_double_tab_len(pipefd, len + 2);
    return ret;
}
int do_cd(char **args)
{
    if ((chdir(args[2]) == -1))
        return 1;
    if (args[2] && args[3] && ft_strcmp(args[3], ";"))
        return 1;
    return 0;
}

char **create_subtab(char **args, int start, int end)
{
    int i = 0;
    char **subtab = malloc(sizeof(char *) * (end - start + 1));
    while (start < end)
        subtab[i++] = ft_strdup(args[start++]);
    subtab[i] = NULL;
    return subtab;
}

char ***parse_on_delimiter(int start, char **args, char *delimiter)
{
    int i = 0;
    int x = 0;
    char ***cmd = NULL;
    
    cmd = malloc(sizeof(char**) * (count_elements(args, delimiter) + 1));
    for (x = start; x < double_tab_len(args); x++)
    {
        if (ft_strcmp(args[x], delimiter) == 0)
        {
            cmd[i] = create_subtab(args, start, x);
            i++;
            start = x + 1;
        }
    }
    cmd[i] = create_subtab(args, start, x);
    cmd[++i] = NULL;
    return cmd;
}

int main(int ac, char **av, char **env)
{
    char ***cmd = NULL;
    char ***cmd_pipe = NULL;
    int i = 0;
    int ret = 0;
    
    if (ac < 2)
        return 1;
    cmd = parse_on_delimiter(1, av, ";");
    while (cmd && cmd[i] && cmd[i][0])
    { 
        cmd_pipe = parse_on_delimiter(0, cmd[i], "|");
        if (double_tab_len(cmd_pipe[0]) != double_tab_len(cmd[i]) && ft_strcmp(cmd_pipe[0][0], "cd"))
            ret = exec_pipes(cmd_pipe, env, triple_tab_len(cmd_pipe));
        else if (ft_strcmp(cmd[i][0], "cd"))
            ret = exec(cmd[i][0], cmd[i], env, 0, NULL, 0);
        else
            ret = do_cd(cmd[i]);
        free_triple_tab(cmd_pipe);
        i++;
    }
    free_triple_tab(cmd);
    return ret;
}