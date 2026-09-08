#define main scanner_main
#include "../src/scanner.c"
#undef main
#include <sys/wait.h>

static int test_blocked_output(int use_socket)
{
    int output[2];
    int ready[2];
    char buffer[4096] = {0};
    int status;
    if ((use_socket ? socketpair(AF_UNIX, SOCK_STREAM, 0, output) : pipe(output)) != 0)
    {
        return 1;
    }
    int flags = fcntl(output[1], F_GETFL);
    if (flags < 0 || fcntl(output[1], F_SETFL, flags | O_NONBLOCK) < 0)
    {
        return 1;
    }
    while (write(output[1], buffer, sizeof(buffer)) > 0)
    {
    }
    if ((errno != EAGAIN && errno != EWOULDBLOCK) ||
        fcntl(output[1], F_SETFL, flags) < 0)
    {
        return 1;
    }
    if (pipe(ready) != 0)
    {
        return 1;
    }
    pid_t child = fork();
    if (child == 0)
    {
        if (install_signal_handlers() != 0)
        {
            _exit(1);
        }
        close(ready[0]);
        if (write(ready[1], "R", 1) != 1)
        {
            _exit(1);
        }
        close(ready[1]);
        int result = write_all(output[1], "tag\n", 4);
        _exit(result == -1 && errno == EINTR && should_stop &&
              fcntl(output[1], F_GETFL) == flags ? 0 : 1);
    }
    if (child < 0)
    {
        return 1;
    }
    close(ready[1]);
    if (read(ready[0], buffer, 1) != 1)
    {
        (void)waitpid(child, &status, 0);
        close(ready[0]);
        close(output[0]);
        close(output[1]);
        return 1;
    }
    close(ready[0]);
    struct timespec delay = {0, 10000000};
    for (int i = 0; i < 10; i++)
    {
        (void)nanosleep(&delay, NULL);
    }
    (void)kill(child, SIGTERM);
    for (int i = 0; i < 100; i++)
    {
        if (waitpid(child, &status, WNOHANG) == child)
        {
            close(output[0]);
            close(output[1]);
            return !(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        }
        (void)nanosleep(&delay, NULL);
    }
    (void)kill(child, SIGKILL);
    (void)waitpid(child, &status, 0);
    close(output[0]);
    close(output[1]);
    return 1;
}

int main(void)
{
    int failures = test_blocked_output(0) + test_blocked_output(1);
    int output[2];
    char received[4];
    if (pipe(output) != 0)
    {
        return 1;
    }
    if (write_all(output[1], "tag\n", 4) != 0 ||
        read(output[0], received, sizeof(received)) != 4 ||
        memcmp(received, "tag\n", 4) != 0)
    {
        failures++;
    }
    close(output[0]);
    close(output[1]);
    printf("%s: scanner output\n", failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
