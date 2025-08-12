#include <unistd.h>

int main()
{
    char *Env[] = {"test", "test", NULL};
    const char * argv[] = {"/usr/bin/php-cgi", "CGI_SCRIPTS/Login.php", NULL};
    execve(argv[0], const_cast<char **>(argv), Env);
}