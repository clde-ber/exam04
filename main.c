#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>

static int pipefd[2];
static int altfd = 0;

int ft_strlen(char *str)
{
    int i = 0;

    while (str[i])
        i++;
    return i;
}

void ft_putstr_fd(int fd, char *str)
{
    write(fd, str, ft_strlen(str));
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
    return (0);
}

int args_size(char **args)
{
    int i = 0;
    while (args[i])
        i++;
    return i;
}

char *ft_strdup(char *src)
{

    int i = 0;
    char *dst = malloc(sizeof(char) * ft_strlen(src) + 1);
    while (src[i])
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return (dst);
}

void ft_free(void **tab)
{
    int i = 0;
    while (tab[i])
    {
        free(tab[i]);
        i++;
    }
    free(tab);
}

int exec(char *cmd, char **av, char **env, int has_pipe)
{
    int ret = 1;
    int status = 0;
    int pid = 0;
    
    pid = fork();
    if (pid == -1)
    {
        ft_putstr_fd(2, "error: fatal\n");
        return (1);
    }
    if (pid == 0)
    {
        if (has_pipe && altfd == 0)
        {
            close(pipefd[0]);
            dup2(pipefd[1], 1);
        }
        else if (has_pipe)
        {
            close(pipefd[1]);
            dup2(pipefd[0], 0);
        }
        if ((ret = execve(cmd, av, env)) == -1)
        {
            ft_putstr_fd(2, "error: cannot execute ");
            for (int x = 0; x < args_size(av); x++)
            {
                ft_putstr_fd(2, av[x]);
                ft_putstr_fd(2, " ");
            }
            ft_putstr_fd(2, "\n");
            exit(status);
        }
        exit(status);
    }
    else
    {
        if (has_pipe && altfd == 0)
        {
            close(pipefd[0]);
            close(pipefd[1]);
        }
        else if (has_pipe)
        {
            close(pipefd[1]);
            close(pipefd[0]);
        }
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            ret = WEXITSTATUS(status);
    }
    printf("altfd %d\n", altfd);
    altfd = (altfd == 0) ? 1 : 0;
    return (ret);
}

int do_cd(char **args)
{
    int ret = 0;
    if (args[1] && (ret = chdir(args[1])) == -1)
    {
        ft_putstr_fd(2, "error: cd: cannot change directory to ");
        ft_putstr_fd(2, args[1]);
        ft_putstr_fd(2, "\n");
        return (1);
    }
    if (args[1] && args[2] && ft_strcmp(args[2], ";"))
    {
        ft_putstr_fd(2, "error: cd: bad arguments\n");
        return (1);
    }
    return (ret);
}

char *ft_strjoin(char *s1, char *s2)
{
    int i = 0;
    int j = 0;
    char *ret = NULL;

    if (!(ret = malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + 1))))
        return (0);
    while (s1[i])
    {
        ret[i] = s1[i];
        i++;
    }
    while (s2[j])
    {
        ret[i + j] = s2[j];
        j++;
    }
    ret[i + j] = '\0';
    return (ret);
}

char **create_subtab(char **av, int start, int i)
{
    char **tab = NULL;
    int x = 0;
    
    if (!(tab = malloc(sizeof(char *) * (i - start + 1))))
        return (NULL);
    while (start < i)
    {
        tab[x++] = ft_strdup(av[start]);
        start++;
    }
    tab[x] = NULL;
    return (tab);
}

int count_elements(char **tab, char *element)
{
    int i = 0;
    int count = 0;

    while (tab[i])
    {
        if (ft_strcmp(tab[i], element) == 0)
            count++;
        i++;
    }
    return (++count);
}

char*** parse_on_delimiter(int start, char** argv, char* delimiter)
{
    char*** formatted_args = NULL;
    int j = 0;
    int i = 0;

    if (!(formatted_args = malloc(sizeof(char **) * (count_elements(argv, delimiter) + 1))))
        return (0);
    for (i = 0; i < args_size(argv); i++)
    {
        if (ft_strcmp(argv[i], delimiter) == 0)
        {
            formatted_args[j] = create_subtab(argv, start, i);
            j++;
            start = i + 1;
        }
    }
    formatted_args[j] = create_subtab(argv, start, i);
    formatted_args[++j] = NULL;
    return (formatted_args);
}

int exec_pipes(char ***cmds_list, char **env, int has_pipe)
{
    int ret = 0;
    int i = 0;

    while (cmds_list[i])
    {
        pipe(pipefd);
        for (int x = 0; x < args_size(cmds_list[i]); x++)
            printf("cmds_list[x] %s\n", cmds_list[i][x]);
        ret = exec(cmds_list[i][0], cmds_list[i], env, has_pipe);
        i++;
    }
    return (ret);
}

int main(int ac, char **av, char **env)
{
    char ***cmds = NULL;
    char ***pipe_cmds = NULL;
    int ret = 0;
    int i = 0;
    int x = 0;
    
    if (ac < 2)
        return (0);
    cmds = parse_on_delimiter(1, av, ";");
    while (cmds[i])
    {
        pipe_cmds = parse_on_delimiter(0, cmds[i], "|");
        if (args_size(pipe_cmds[x]) != args_size(cmds[i]))
            ret = exec_pipes(pipe_cmds, env, 1);
        else
            ret = exec_pipes(pipe_cmds, env, 0);
        while (pipe_cmds[x])
        {
            ft_free(((void**)pipe_cmds)[x]);
            pipe_cmds[x] = NULL;
            x++;
        }
        free(pipe_cmds);
        pipe_cmds = NULL;
        x = 0;
        i++;
    }
    while (cmds[x])
    {
        ft_free(((void**)cmds)[x]);
        cmds[x] = NULL;
        x++;
    }
    cmds = NULL;
    free(cmds);
    return (ret);
}