#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>

static int fd = 0;
static int has_fork = 0;

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

int ft_strlen(char *str)
{
    int i = 0;

    while (str[i])
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

char **create_command_tab(char **args)
{
    int i = 0;
    char **tab = malloc(sizeof(char *) * args_size(args));
    while (args[i])
    {
        if (ft_strcmp(args[i], "|") == 0 || ft_strcmp(args[i], ";") == 0)
            break ;
        tab[i] = ft_strdup(args[i]);
        i++;
    }
    tab[i] = NULL;
    return (tab);
}

int exec_cmd(char **av, char *cmd, char **env, int pid)
{
    int ret = 1;
    int status = 0;

    printf("hey");
    if (pid == -1)
        return (1);
    if (pid == 0)
    {
        char **cmd_tab = create_command_tab(av);
        if ((ret = execve(cmd, cmd_tab, env)) == -1)
        {
            free(cmd_tab);
            exit(1);
        }
    /*    for (int x = 0; x < 2; x++)
            printf("cmd_tab %s\n", cmd_tab[x]);
        printf("cmd %s\n", cmd);
        free(cmd_tab);
    */    exit(ret);
    }
    else
    {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            ret = WEXITSTATUS(status);
        close(fd); 
    }
    exit(ret);
}

int do_pipe()
{
    int new_fd = (fd == 0) ? 1 : 0;
    int pid = fork();
    if (fd == 0)
    {
        if (dup2(fd, new_fd) == -1)
            exit(1);
        fd = 1;
    }
    dup2(fd, new_fd);
    fd = 0;
    return (pid);
}

int do_cd(char **args)
{
    int ret = 0;
    if (args[1] && (ret = chdir(args[1])) == -1)
        exit(1);
   // args = &args[1];
    exit(ret);
}

int catch_cmd(char **av, char *cmd, char **env)
{
    int pid = 0;

    printf("cmd %s\n", cmd);
    if (ft_strcmp(cmd, "|") == 0)
    {
        has_fork = 1;
        pid = do_pipe();
        return (exec_cmd(av, cmd, env, pid));
    }
    if (ft_strcmp(cmd, ";") == 0)
        return (0);
    if (ft_strcmp(cmd, "cd") == 0)
    {
        printf("la");
        av = &av[1];
        return (do_cd(av));
    }
    if (has_fork == 0)
    {
        has_fork = 1;
        pid = fork();
        return (exec_cmd(av, cmd, env, pid));
    }
    return (0);
}

int parse_cmd(int ac, char **av, char **env)
{
    int i = 1;

    while (i < ac)
    {
        if (catch_cmd(&av[i], av[i], env))
            return (1);
        i++;
    }
    return (0);
}

int main(int ac, char **av, char **env)
{
    if (ac < 2)
        return (0);
    if (!parse_cmd(ac, av, env))
        return (0);
    return (1);
}