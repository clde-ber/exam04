#include <sys/wait.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static int pipe_start = 0;
static int pipe_end = 0;

void ft_free(char **tab)
{
    int i = 0;
    
    while (tab[i])
    {
        free(tab[i]);
        i++;
    }
    free(tab);
}

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

    while (s1[i] && s2[i])
    {
        if (s1[i] != s2[i])
            return (s1[i] - s2[i]);
        i++;
    }
    return 0;
}

char *ft_strdup(char *str)
{
    char *ret = NULL;
    int i = 0;

    if (!(ret = malloc(sizeof(char) * (ft_strlen(str) + 1))))
        return (NULL);
    while (str[i])
    {
        ret[i] = str[i];
        i++;
    }
    ret[i] = '\0';
    return ret;
}

int args_size(char **tab)
{
    int i = 0;
    while (tab[i])
        i++;
    return i;
}

int triple_table_len(char ***triple_tab)
{
    int i = 0;
    while (triple_tab[i])
        i++;
    return i;
}

int count_elements(char **cmd, char *element)
{
    int i = 0;
    int count = 0;

    while (cmd[i])
    {
        if (ft_strcmp(cmd[i], element) == 0)
            count++;
        i++;
    }
    return (count + 1);
}

int do_cd(char **args)
{
    if (args[1] && (chdir(args[1]) != -1))
        return 0;
    if (args[1] && args[2] && ft_strcmp(args[2], ";"))
        return 1;
    return 1;
}

int exec(char *cmd, char **cmd_list, char **env, int has_pipe, int **pipefd, int i)
{
    pid_t pid = 0;
    int ret = 1;
    int status = 0;

    if ((pid = fork()) == -1)
        return 1;
    else if (pid == 0)
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

void ft_free_pipefd(int **pipefd, int len)
{
    for (int x = 0; x < len; x++)
    {
        free(pipefd[x]);
        pipefd[x] = NULL;
    }
    free(pipefd);
    pipefd = NULL;
}

int exec_pipes(char ***pipe_cmd, char **env)
{
    int **pipefd = NULL;
    int i = 0;
    int ret = 0;
    int len = triple_table_len(pipe_cmd) + 1;

    pipe_start = 0;
    pipe_end = 0;
    if (!(pipefd = malloc(sizeof(int *) * len)))
        return 1;
    for (int x = 0; x < len; x++)
    {
        if (!(pipefd[x] = malloc(sizeof(int) * 2)))
            return 1;
    }
    pipe_start = 1;
    pipe(pipefd[i]);
    pipe(pipefd[i + 1]);
    ret = exec(pipe_cmd[i][0], pipe_cmd[i], env, 1, pipefd, i + 1);
    pipe_start = 0;
    i++;
    while (pipe_cmd[i] && pipe_cmd[i + 1])
    {
        pipe(pipefd[i + 1]);
        ret = exec(pipe_cmd[i][0], pipe_cmd[i], env, 1, pipefd, i + 1);
        i++;
    }
    pipe_end = 1;
    if (pipe_cmd[i])
    {
        pipe(pipefd[i + 1]);
        ret = exec(pipe_cmd[i][0], pipe_cmd[i], env, 1, pipefd, i + 1);
    }
    pipe_end = 0;
    ft_free_pipefd(pipefd, len);
    return ret;
}

char **create_subtab(char **cmd, int start, int i)
{
    int x = 0;
    char **ret = NULL;

    if (!(ret = malloc(sizeof(char *) * (i - start + 1))))
        return NULL;
    while (start < i)
        ret[x++] = ft_strdup(cmd[start++]);
    ret[x] = NULL;
    return ret;
}

char ***parse_on_delimiter(int start, char **cmd, char *delimiter)
{

    int i = start;
    int j = 0;
    char ***res = NULL;

    if (!(res = malloc(sizeof(char **) * (count_elements(cmd, delimiter) + 1))))
        return (NULL);
    for (i = start; i < args_size(cmd); i++)
    {
        if (ft_strcmp(cmd[i], delimiter) == 0)
        {
            res[j] = create_subtab(cmd, start, i);
            j++;
            start = i + 1;
        }
    }
    res[j] = create_subtab(cmd, start, i);
    res[++j] = NULL;
    return res;
}

int main(int ac, char **av, char **env)
{
    int i = 0;
    char ***cmd = NULL;
    char ***pipe_cmd = NULL;
    int ret = 0;

    if (ac < 2)
        return 0;
    if (!(cmd = parse_on_delimiter(1, av, ";")))
        return 1;
    while (i < triple_table_len(cmd))
    {
        if (!(pipe_cmd = parse_on_delimiter(0, cmd[i], "|")))
            return 1;
        if (args_size(pipe_cmd[0]) != args_size(cmd[i]) && ft_strcmp(pipe_cmd[0][0], "cd"))
            ret = exec_pipes(pipe_cmd, env);
        else if (ft_strcmp(cmd[i][0], "cd"))
            ret = exec(cmd[i][0], cmd[i], env, 1, NULL, 0);
        else
            ret = do_cd(cmd[i]);
        i++;
    }
    i = 0;
    while (cmd[i])
    {
        ft_free(cmd[i]);
        cmd[i] = NULL;
        i++;
    }
    free(cmd);
    cmd = NULL;
    i = 0;
    while (pipe_cmd[i])
    {
        ft_free(pipe_cmd[i]);
        pipe_cmd[i] = NULL;
        i++;
    }
    free(pipe_cmd);
    pipe_cmd = NULL;
    return ret;
}