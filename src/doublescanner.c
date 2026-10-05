#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#include "yrm100/yrm100.h"
#include "yrm100/yrm100_command.h"
#include "yrm100/yrm100_error.h"
#include "yrm100/yrm100_param.h"
#include "yrm100/yrm100_string.h"
#include "yrm100/yrm100_util.h"

#define MODULE_COUNT 2
#define DEFAULT_SERIAL_DEVICE_0 "/dev/ttyUSB0"
#define DEFAULT_SERIAL_DEVICE_1 "/dev/ttyUSB1"
#define MAX_TAG_COUNT 30
#define LISTEN_BACKLOG 1

static volatile sig_atomic_t should_stop = 0;

static void handle_signal(int signal_number)
{
    (void)signal_number;
    should_stop = 1;
}

static int install_signal_handlers(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_signal;
    if (sigemptyset(&action.sa_mask) != 0 ||
        sigaction(SIGINT, &action, NULL) != 0 ||
        sigaction(SIGTERM, &action, NULL) != 0)
    {
        return -1;
    }

    action.sa_handler = SIG_IGN;
    return sigaction(SIGPIPE, &action, NULL);
}

static int parse_interval(const char *value, unsigned long *interval_ms)
{
    char *end = NULL;
    unsigned long parsed;

    errno = 0;
    parsed = strtoul(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || parsed == 0)
    {
        return -1;
    }
    *interval_ms = parsed;
    return 0;
}

static int check_socket_path_available(const char *socket_path)
{
    struct stat path_info;

    if (lstat(socket_path, &path_info) != 0)
    {
        return errno == ENOENT ? 0 : -1;
    }

    errno = EEXIST;
    return -1;
}

static int create_server_socket(const char *socket_path)
{
    int server_fd;
    struct sockaddr_un address;
    size_t path_length = strlen(socket_path);

    if (path_length == 0 || path_length >= sizeof(address.sun_path))
    {
        errno = ENAMETOOLONG;
        return -1;
    }
    if (check_socket_path_available(socket_path) != 0)
    {
        return -1;
    }

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        return -1;
    }
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    memcpy(address.sun_path, socket_path, path_length + 1);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) != 0 ||
        listen(server_fd, LISTEN_BACKLOG) != 0)
    {
        int saved_errno = errno;

        close(server_fd);
        unlink(socket_path);
        errno = saved_errno;
        return -1;
    }
    return server_fd;
}

static int configure_reader(yrm100_context_t *device)
{
    if (yrm100_command_disable_idle_sleep(device) != YRM100_STATUS_OK ||
        yrm100_command_enable_frequency_hopping(device) != YRM100_STATUS_OK ||
        yrm100_command_set_operating_region(
            device, YRM100_PARAM_REGION_EUROPE) != YRM100_STATUS_OK)
    {
        return -1;
    }
    return 0;
}

static void shutdown_module(yrm100_context_t *device)
{
    int result = yrm100_command_disable_continous_wave(device);

    if (result != YRM100_STATUS_OK)
    {
        fprintf(stderr, "Failed to disable continuous wave: %s (%d)\n",
                yrm100_error_code_to_string(result), result);
    }
    yrm100_deinit(device);
}

static void shutdown_modules(yrm100_context_t *devices[MODULE_COUNT])
{
    for (int i = 0; i < MODULE_COUNT; i++)
    {
        if (devices[i] != NULL)
        {
            shutdown_module(devices[i]);
            devices[i] = NULL;
        }
    }
}

static int open_modules(
    const char *serial_paths[MODULE_COUNT], yrm100_context_t *devices[MODULE_COUNT])
{
    for (int i = 0; i < MODULE_COUNT; i++)
    {
        devices[i] = yrm100_init(serial_paths[i]);
        if (devices[i] == NULL)
        {
            fprintf(stderr, "yrm100_init() failed for %s\n", serial_paths[i]);
            return -1;
        }
        if (configure_reader(devices[i]) != 0)
        {
            fprintf(stderr, "Failed to configure RFID reader on %s\n",
                    serial_paths[i]);
            return -1;
        }
    }
    return 0;
}

static void sleep_interval(unsigned long interval_ms)
{
    struct timespec requested;
    struct timespec remaining;

    requested.tv_sec = (time_t)(interval_ms / 1000UL);
    requested.tv_nsec = (long)((interval_ms % 1000UL) * 1000000UL);
    while (!should_stop && nanosleep(&requested, &remaining) != 0)
    {
        if (errno != EINTR)
        {
            break;
        }
        requested = remaining;
    }
}

static int write_all(int output_fd, const char *data, size_t length)
{
    size_t written = 0;
    int flags = fcntl(output_fd, F_GETFL);
    int status = -1;
    int saved_errno;

    if (flags < 0 || fcntl(output_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        return -1;
    }

    while (written < length)
    {
        ssize_t result;

        if (should_stop)
        {
            errno = EINTR;
            break;
        }
        result = write(output_fd, data + written, length - written);

        if (result > 0)
        {
            written += (size_t)result;
        }
        else if (result < 0 && errno == EINTR)
        {
            continue;
        }
        else if (result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            struct pollfd output = {output_fd, POLLOUT, 0};

            /* Bound the wait even if a stop signal arrives just before poll. */
            if (poll(&output, 1, 100) < 0 && errno != EINTR)
            {
                break;
            }
        }
        else
        {
            if (result == 0)
            {
                errno = EIO;
            }
            break;
        }
    }
    if (written == length)
    {
        status = 0;
    }
    saved_errno = errno;
    if (fcntl(output_fd, F_SETFL, flags) < 0)
    {
        return -1;
    }
    errno = saved_errno;
    return status;
}

static int write_debug_tag(int module_index, const char *epc)
{
    char line[YRM100_TAG_EPC_STRING_LENGTH + 32];

    snprintf(line, sizeof(line), "module %d: %s\n", module_index, epc);
    return write_all(STDOUT_FILENO, line, strlen(line));
}

static int scan_module(
    yrm100_context_t *device, int module_index, int output_fd, int debug,
    yrm100_rfid_tag_t *tags)
{
    int result;

    yrm100_reset_tag_buf(tags, MAX_TAG_COUNT);
    result = yrm100_command_single_poll(device, tags, MAX_TAG_COUNT);
    if (result < 0)
    {
        fprintf(stderr, "module %d: single poll failed: %s (%d)\n",
                module_index, yrm100_error_code_to_string(result), result);
        return 0;
    }

    for (unsigned short i = 0; i < MAX_TAG_COUNT; i++)
    {
        char epc[YRM100_TAG_EPC_STRING_LENGTH];

        if (yrm100_is_empty_tag(&tags[i]))
        {
            continue;
        }
        yrm100_get_tag_epc_string(&tags[i], epc);
        if (write_all(output_fd, epc, strlen(epc)) != 0 ||
            write_all(output_fd, "\n", 1) != 0)
        {
            return -1;
        }
        if (debug && output_fd != STDOUT_FILENO &&
            write_debug_tag(module_index, epc) != 0)
        {
            return -1;
        }
    }
    return 0;
}

static int scan_all_modules(
    yrm100_context_t *devices[MODULE_COUNT], int output_fd, int debug,
    yrm100_rfid_tag_t *tags)
{
    /* Modules are polled one after another so their RF fields never overlap,
     * avoiding the reader-to-reader interference of two independent scanners. */
    for (int i = 0; i < MODULE_COUNT; i++)
    {
        if (scan_module(devices[i], i, output_fd, debug, tags) != 0)
        {
            return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *socket_path;
    unsigned long interval_ms;
    const char *serial_paths[MODULE_COUNT] = {
        DEFAULT_SERIAL_DEVICE_0, DEFAULT_SERIAL_DEVICE_1};
    yrm100_context_t *devices[MODULE_COUNT] = {NULL, NULL};
    yrm100_rfid_tag_t tags[MAX_TAG_COUNT] = {{0}};
    int server_fd;
    int use_stdout;
    int debug = 0;
    const char *positional_arguments[4];
    int positional_count = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug") == 0)
        {
            if (debug)
            {
                positional_count = -1;
                break;
            }
            debug = 1;
        }
        else if (positional_count < 4)
        {
            positional_arguments[positional_count++] = argv[i];
        }
        else
        {
            positional_count = -1;
            break;
        }
    }
    if (positional_count < 2 || positional_count > 4)
    {
        fprintf(stderr,
                "Usage: %s [--debug] <unix-socket-path|-> <interval-ms> "
                "[serial-device-0] [serial-device-1]\n"
                "Defaults: serial-device-0=%s serial-device-1=%s\n",
                argv[0], DEFAULT_SERIAL_DEVICE_0, DEFAULT_SERIAL_DEVICE_1);
        return EXIT_FAILURE;
    }
    socket_path = positional_arguments[0];
    use_stdout = strcmp(socket_path, "-") == 0;
    if (parse_interval(positional_arguments[1], &interval_ms) != 0)
    {
        fprintf(stderr, "Interval must be a positive integer in milliseconds\n");
        return EXIT_FAILURE;
    }
    if (positional_count >= 3)
    {
        serial_paths[0] = positional_arguments[2];
    }
    if (positional_count >= 4)
    {
        serial_paths[1] = positional_arguments[3];
    }

    if (install_signal_handlers() != 0)
    {
        fprintf(stderr, "Failed to install signal handlers: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    if (open_modules(serial_paths, devices) != 0)
    {
        shutdown_modules(devices);
        return EXIT_FAILURE;
    }

    printf("Serial devices: %s, %s\nUnix socket: %s\nPoll interval %lums\n", serial_paths[0], serial_paths[1], use_stdout ? "stdout" : socket_path, interval_ms);
    fflush(stdout);

    if (use_stdout)
    {
        while (!should_stop)
        {
            if (scan_all_modules(devices, STDOUT_FILENO, debug, tags) != 0)
            {
                fprintf(stderr, "Failed to write EPC code to stdout: %s\n",
                        strerror(errno));
                break;
            }
            sleep_interval(interval_ms);
        }

        yrm100_free_tag_data(tags, MAX_TAG_COUNT);
        shutdown_modules(devices);
        return should_stop ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    server_fd = create_server_socket(socket_path);
    if (server_fd < 0)
    {
        fprintf(stderr, "Failed to create socket %s: %s\n",
                socket_path, strerror(errno));
        shutdown_modules(devices);
        return EXIT_FAILURE;
    }

    while (!should_stop)
    {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            fprintf(stderr, "accept failed: %s\n", strerror(errno));
            break;
        }
        if (debug)
        {
            printf("Socket client connected\n");
            fflush(stdout);
        }
        while (!should_stop)
        {
            if (scan_all_modules(devices, client_fd, debug, tags) != 0)
            {
                break;
            }
            sleep_interval(interval_ms);
        }
        close(client_fd);
    }

    yrm100_free_tag_data(tags, MAX_TAG_COUNT);
    close(server_fd);
    unlink(socket_path);
    shutdown_modules(devices);
    return should_stop ? EXIT_SUCCESS : EXIT_FAILURE;
}
