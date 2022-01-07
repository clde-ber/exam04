#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static int pipe_end = 0;

int ft_strlen(char *str)
{
    int i = 0;

    while (str[i])
        i++;
    return i;
}

char* ft_strdup(char *str)
{
    char *res = NULL;
    int i = 0;

    res = malloc(sizeof(char) * (ft_strlen(str) + 1));
    while (str[i])
    {
        res[i] = str[i];
        i++;
    }
    res[i] = '\0';
    return res;
}

int ft_strcmp(char *s1, char *s2)
{
    int i = 0;

    while (s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return (s1[i] - s2[i]);
}

void ft_free_char(char **tab)
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

void ft_free_int(int **tab, int len)
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

void ft_free_triple_tab_char(char ***triple_tab)
{
    int i = 0;

    while (triple_tab[i])
    {
        ft_free_char(triple_tab[i]);
        i++;
    }
    free(triple_tab);
    triple_tab = NULL;
}

int triple_table_len(char ***triple_table)
{
    int i = 0;

    while (triple_table[i])
        i++;
    return i;
}

int double_table_len(char **double_table)
{
    int i = 0;

    while (double_table[i])
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

char **create_subtab(char **args, int start, int end)
{
    char **subtab = NULL;
    int i = 0;

    subtab = malloc(sizeof(char *) * (end - start + 1));
    while (start < end)
        subtab[i++] = ft_strdup(args[start++]);
    subtab[i] = NULL;
    return subtab;
}

int exec(char *cmd, char **cmd_list, char **env, int has_pipe, int** pipefd, int i)
{
    pid_t pid = 0;
    int ret = 0;
    int status;

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
            if ((ret = execve(cmd, cmd_list, env)) == -1)
                exit(status);
            exit(status);
        }
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

int exec_pipes(char ***cmd_pipe, char **env)
{
    int **pipefd = NULL;
    int i = 0;
    int ret = 0;

    pipe_end = 0;
    pipefd = malloc(sizeof(int *) * (triple_table_len(cmd_pipe) + 1));
    for (int x = 0; x < triple_table_len(cmd_pipe) + 1; x++)
        pipefd[x] = malloc(sizeof(int) * 2);
    while (cmd_pipe[i])
    {
        if (i == 0)
        {
            pipe(pipefd[i]);
            pipe(pipefd[i + 1]);
        }
        else
        {
            if (i == triple_table_len(cmd_pipe) - 1)
                pipe_end = 1;
            pipe(pipefd[i + 1]);
        }
        ret = exec(cmd_pipe[i][0], cmd_pipe[i], env, 1, pipefd, i + 1);
        i++;
    }
    ft_free_int(pipefd, triple_table_len(cmd_pipe) + 1);
    return ret;
}

char ***parse_on_delimiter(int start, char **args, char *delimiter)
{
    int i = 0;
    int x = 0;
    char ***cmd = NULL;

    cmd = malloc(sizeof(char **) * (count_elements(args, delimiter) + 1));
    for (x = start; x < double_table_len(args); x++)
    {
        if (ft_strcmp(args[x], delimiter) == 0)
        {
            cmd[i] = create_subtab(args, start, x);
            start = x + 1;
            i++;
        }
    }
    cmd[i] = create_subtab(args, start, x);
    cmd[++i] = NULL;
    return cmd;
}

int do_cd(char **args)
{
    if ((chdir(args[1])) == -1)
        return 1;
    if (args[2] && ft_strcmp(args[2], ";"))
        return 1;
    return 0;
}

int main(int ac, char **av, char**env)
{
    char ***cmd = NULL;
    char ***cmd_pipe = NULL;
    int i = 0;
    int ret = 0;

    if (ac < 2)
        return 1;
    cmd = parse_on_delimiter(1, av, ";");
    while (cmd[i])
    {
        cmd_pipe = parse_on_delimiter(0, cmd[i], "|");
        if (double_table_len(cmd[i]) != double_table_len(cmd_pipe[i]) && ft_strcmp(cmd_pipe[i][0], "cd"))
            ret = exec_pipes(cmd_pipe, env);
        else if (ft_strcmp(cmd_pipe[i][0], "cd"))
            ret = exec(cmd_pipe[i][0], cmd_pipe[i], env, 0, NULL, 0);
        else
            ret = do_cd(cmd_pipe[i]);
        i++;
        ft_free_triple_tab_char(cmd_pipe);
    }
    ft_free_triple_tab_char(cmd);
    return ret;
}