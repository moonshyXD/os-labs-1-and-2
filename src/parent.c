#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "parent_input.h"
#include "pipe_io.h"
#include "status.h"

static int close_descriptor(int fd) {
    if (close(fd) == -1) {
        perror("close");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static void start_child(int pipe1[2], char *filename) {
    if (dup2(pipe1[0], STDIN_FILENO) == -1) {
        perror("dup2");
        _exit(EXIT_FAILURE);
    }
    if (pipe1[0] != STDIN_FILENO && close_descriptor(pipe1[0]) != EXIT_SUCCESS) {
        _exit(EXIT_FAILURE);
    }
    if (close_descriptor(pipe1[1]) != EXIT_SUCCESS) {
        _exit(EXIT_FAILURE);
    }
    execv("./child", (char *[]){"./child", filename, NULL});
    perror("execv");
    _exit(EXIT_FAILURE);
}

static int send_commands(int fd) {
    for (;;) {
        char *line = NULL;
        size_t len = 0;
        Status status = read_command_line(&line, &len);
        if (status == STATUS_END_OF_INPUT) {
            return EXIT_SUCCESS;
        }
        if (status != STATUS_OK) {
            fprintf(stderr, "parent: ошибка чтения команды\n");
            return EXIT_FAILURE;
        }
        int result = write_all(fd, line, len);
        if (result == -1) {
            perror("write pipe1");
        }
        free(line);
        if (result == -1) {
            return EXIT_FAILURE;
        }
    }
}

static int wait_for_child(pid_t pid) {
    int status;
    while (waitpid(pid, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
    }
    if (!WIFEXITED(status)) {
        fprintf(stderr, "child завершился ненормально\n");
        return EXIT_FAILURE;
    }
    int code = WEXITSTATUS(status);
    if (printf("child завершился с кодом %d\n", code) < 0 || fflush(stdout) == EOF) {
        perror("stdout");
        return EXIT_FAILURE;
    }
    return code == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int ignore_sigpipe(void) {
    struct sigaction action = {0};
    action.sa_handler = SIG_IGN;
    if (sigemptyset(&action.sa_mask) == -1 || sigaction(SIGPIPE, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int run_parent(int pipe1[2], pid_t pid) {
    int result = close_descriptor(pipe1[0]);
    if (result == EXIT_SUCCESS) {
        result = send_commands(pipe1[1]);
    }
    if (close_descriptor(pipe1[1]) != EXIT_SUCCESS) {
        result = EXIT_FAILURE;
    }
    if (wait_for_child(pid) != EXIT_SUCCESS) {
        result = EXIT_FAILURE;
    }
    return result;
}

static int run_pipeline(char *filename) {
    if (ignore_sigpipe() != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    int pipe1[2];
    if (pipe(pipe1) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    if (fflush(stdout) == EOF) {
        perror("fflush");
        close_descriptor(pipe1[0]);
        close_descriptor(pipe1[1]);
        return EXIT_FAILURE;
    }
    pid_t pid = fork();
    if (pid == 0) {
        start_child(pipe1, filename);
    }
    if (pid == -1) {
        perror("fork");
        close_descriptor(pipe1[0]);
        close_descriptor(pipe1[1]);
        return EXIT_FAILURE;
    }
    return run_parent(pipe1, pid);
}

int main(void) {
    char *filename = NULL;
    if (read_filename(&filename) != STATUS_OK) {
        fprintf(stderr, "parent: не удалось прочитать имя файла\n");
        return EXIT_FAILURE;
    }
    int result = run_pipeline(filename);
    free(filename);
    return result;
}
