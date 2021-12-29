#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>

static int has_pipe = 0;
static int idx = 0;
static int fd = 0;
static int fd1[2];
static int fd2[2];
static int *pipetab;

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
    idx = i;
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

void pipe_fds(int in_or_out)
{
    if (in_or_out == 0)
    {
        if (pipe(fd1) < 0 || pipe(fd2) < 0)
            ft_putstr_fd(2, "error\n");
    }
    else
    {
        if (pipe(fd2) < 0 || pipe(fd1) < 0)
            ft_putstr_fd(2, "error\n");
    }
}

void dup_fds(int in_or_out)
{
    if (in_or_out == 0)
    {
    close(fd1[1]);
    close(fd2[0]);

    if (fd1[0] != STDIN_FILENO)
    {
        if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
            ft_putstr_fd(2, "error: fatal\n");
        close(fd1[0]);
    }
    if (fd2[1] != STDOUT_FILENO)
    {
        if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
            ft_putstr_fd(2, "error: fatal\n");
        close(fd2[1]);
    }
    }
    else
    {
    close(fd2[0]);
    close(fd1[1]);

    if (fd2[1] != STDOUT_FILENO)
    {
        if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
            ft_putstr_fd(2, "error: fatal\n");
        close(fd2[1]);
    }
    if (fd1[0] != STDIN_FILENO)
    {
        if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
            ft_putstr_fd(2, "error: fatal\n");
        close(fd1[0]);
    }
    }  
}

void close_fds(int in_or_out)
{
    if (in_or_out == 0)
    {
    close(fd1[0]);
    close(fd2[1]);
    }
    else
    {
        close(fd2[1]);
        close(fd1[0]);
    }
}

int exec_cmd(char **av, char **env, int pid)
{
    int ret = 1;
    int status = 0;

    char **cmd_tab = create_command_tab(av);
    if (has_pipe)
        pipe_fds(fd);
    pid = fork();
    if (pid == -1)
    {
        ft_putstr_fd(2, "error: fatal\n");
        return (1);
    }
    if (pid == 0)
    {
        if (has_pipe)
            dup_fds(fd);
        for (int x = 0; x < args_size(cmd_tab) + 1; x++)
            printf("args %s\n", cmd_tab[x]);
        if ((ret = execve(cmd_tab[0], cmd_tab, env)) == -1)
        {
            ft_putstr_fd(2, "error: cannot execute ");
            for (int x = 0; x < args_size(cmd_tab); x++)
            {
                ft_putstr_fd(2, cmd_tab[x]);
                ft_putstr_fd(2, " ");
            }
            ft_putstr_fd(2, "\n");
            exit(status);
        }
        exit(status);
    }
    else
    {
        waitpid(pid, &status, 0);
        if (has_pipe)
        {
            close_fds(fd);
            has_pipe--;
        }
        if (WIFEXITED(status))
            ret = WEXITSTATUS(status);
    }
    ft_free(cmd_tab);
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
        if (has_pipe > 1)
            fd = (fd == 0) ? 1 : 0;
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

char **create_subtab(char **av, char* delimiter)
{
    int i = 0;
    int j = 0;
    if (ft_strcmp(av[i], delimiter) == 0)
        i++;
    char **tab = malloc(sizeof(char *) * (args_size(av) + 1));
    while (av[i])
    {
        if (ft_strcmp(av[i], delimiter) == 0)
            break ;
        tab[j] = ft_strdup(av[i]);
        i++;
        j++;
    }
    tab[j] = NULL;
    int x = 0;
        while (tab[x])
        {
            printf("tab[i] = %s\n", tab[x]);
            x++;
        }
    return (tab);
}

char ***parse_on_delimiter(int ac, char **av, char **env, char* delimiter)
{
    (void)env;
    (void)ac;
    char ***tab = NULL;
    int i = 0;
    int size = 1;
    char **tmp = NULL;

    if (!(tab = malloc(sizeof(char **) * (ac))))
        return (0);
    if (!(pipetab = malloc(sizeof(int) * (ac))))
        return (0);
    for (int x = 0 ; x < ac - 1 ; x++)
        pipetab[x] = 0;
    while (size < ac - 1)
    {
        if ((tab[i] = create_subtab(&av[size], delimiter)) == NULL)
            return (NULL);
        tmp = tab[i];
        if ((tab[i] = create_subtab(tab[i], "|")) == NULL)
            return (NULL);
        printf("1 %d\n", args_size(tab[i]));
        printf("2 %d\n", args_size(&av[size]));
        if (args_size(tab[i]) != args_size(tmp))
            pipetab[size - 1] = 1;
        ft_free(tmp);
        int x = 0;
        while (tab[i][x])
        {
            printf("tab[i][x] = %s\n", tab[i][x]);
            x++;
        }
        size += args_size(tab[i]) + 1;
        printf("size %d\n", size);
        i++;
        printf("**********\n");
    }
    tab[i] = NULL;
    return (tab);
}

int main(int ac, char **av, char **env)
{
    int pid = 0;
    int ret = 0;
    char ***tab = NULL;
    int i = 0;

    if (ac < 2)
        return (0);
    tab = parse_on_delimiter(ac, av, env, ";");
    while (tab[i])
    {
        if (ft_strcmp(tab[i][0], "cd") == 0)
            ret = do_cd(tab[i]);
        else
        {
            if (pipetab[i] || has_pipe)
            {
                has_pipe++;
                if (i)
                    fd = (fd == 0) ? 1 : 0;
            }
            printf("pipetab[i] %d\n", pipetab[i]);
            ret = exec_cmd(tab[i], env, pid);
        }
        i++;
    }
    i = 0;
    while (tab[i])
    {
        ft_free(tab[i]);
        i++;
    }
    free(tab);
    return (ret);
}