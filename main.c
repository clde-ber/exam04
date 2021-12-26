#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>

static int fd = 0;
static int has_pipe = 0;
static int fd0[2] = {0, 1};

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

char **create_command_tab(char **args)
{
    int i = 0;
    char **tab = malloc(sizeof(char *) * (args_size(args) + 1));
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

int exec_cmd(char **av, char **env, int pid)
{
    int ret = 1;
    int status = 0;

    if (has_pipe && fd == 0)
    {
     //   close(1);
        if (pipe(fd0))
        {
            ft_putstr_fd(2, "error: fatal\n");
		    return (1);
        }
    }
    else if (has_pipe)
    {
     //   close(0);
        if (pipe(fd0))
        {
            ft_putstr_fd(2, "error: fatal\n");
            return (1);
        }
    }
    pid = fork();
    if (pid == -1)
    {
        ft_putstr_fd(2, "error: fatal\n");
        return (1);
    }
    if (pid == 0)
    {
        if (has_pipe)
        {
            if (fd == 0)
            {
             //   close(1);
                if ((fd = dup(0)) == -1)
                {
                    ft_putstr_fd(2, "error: fatal\n");
                    return (1);
                }
                fd0[0] = 1;
                fd0[1] = 0;
                return (0);
            }
         //   close(0);
            if ((fd = dup(1)) == -1)
            {
                ft_putstr_fd(2, "error: fatal\n");
                return (1);
            }
            fd0[0] = 0;
            fd0[1] = 1;
        }
        char **cmd_tab = create_command_tab(av);
        for (int x = 0; x < args_size(cmd_tab) + 1; x++)
            printf("args %s\n", cmd_tab[x]);
        if ((ret = execve(cmd_tab[0], cmd_tab, env)) == -1)
        {
            ft_putstr_fd(2, "error: cannot execute ");
            ft_putstr_fd(2, cmd_tab[0]);
            ft_putstr_fd(2, "\n");
            ft_free(cmd_tab);
            exit(status);
        }
        ft_free(cmd_tab);
        exit(status);
    }
    else
    {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            ret = WEXITSTATUS(status);
        if (has_pipe)
        {
            close(fd);
            has_pipe--;
        }
    }
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

int catch_cmd(char **av, char **env)
{
    int pid = 0;

    printf("CMD %s\n", av[0]);
    if (ft_strcmp(av[0], "|") == 0)
    {
        has_pipe++;
        return (0);
    }
    if (ft_strcmp(av[0], ";") == 0)
        return (0);
    if (ft_strcmp(av[0], "cd") == 0)
    {
        if (do_cd(av))
            return (1);
        return (0);
    }
    return (exec_cmd(av, env, pid));
}

int next_sep(char **args)
{
    int i = 0;

    i++;
    while (args[i] && ft_strcmp(args[i], ";") && ft_strcmp(args[i], "|"))
        i++;
    printf("next sep %d\n", i);
    return (i + 1);
}

int parse_cmd(int ac, char **av, char **env)
{
    int i = 1;

    while (i < ac)
    {
        printf("&av[i][0] %s\n", &av[i][0]);
        if (&av[i][0] && catch_cmd(&av[i], env) == 1)
            return (1);
        i++;
    }
    return (0);
}

int main(int ac, char **av, char **env)
{
    has_pipe = 0;
    if (ac < 2)
        return (0);
    return (parse_cmd(ac, av, env));
}