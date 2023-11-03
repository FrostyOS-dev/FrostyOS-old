#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>

#include <sstream>
#include <string>
#include <algorithm>

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <ELF-file> <out-file>", argv[0]);
        return 1;
    }

    char* home = getenv("HOME");
    if (home == nullptr) {
        fprintf(stderr, "Error: cannot find nm\n");
        return 1;
    }

    std::stringstream ss;
    ss << home << "/opt/";
#ifdef __x86_64__
    ss << "x86_64";
#else
#error Unknown architecture
#endif
    ss << "-worldos-cross/bin/";
#ifdef __x86_64__
    ss << "x86_64";
#else
#error Unknown architecture
#endif
    ss << "-worldos-nm";
    
    int link[2];
    if (pipe(link) < 0) {
        perror("pipe");
        return 1;
    }

    char* buffer;
    uint64_t nbytes;
    std::string str;

    std::string path = ss.str();
    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("fork");
        return 1;
    }

    if (child_pid) {
        // parent
        close(link[1]);
        wait(NULL);
        buffer = (char*)calloc(4096, 1);
        memset(buffer, 0, 4096);
        nbytes = 0;
        uint64_t count = 0;
        while (0 != (count = read(link[0], &(buffer[nbytes]), 4096))) {
            nbytes += count;
            if ((nbytes & 0xFFF) == 0) {
                buffer = (char*)realloc(buffer, nbytes + 4096);
                if (buffer == nullptr) {
                    perror("realloc");
                    return 1;
                }
            }
        }
        if ((nbytes & 0xFFF) == 0) {
            buffer = (char*)realloc(buffer, nbytes + 4096);
            if (buffer == nullptr) {
                perror("realloc");
                return 1;
            }
        }
        buffer[nbytes] = 0;
        str = std::string(buffer);
    }
    else {
        // child
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        char* c_path = new char[path.length() + 1];
        memcpy(c_path, path.c_str(), path.length() + 1);
        char* const new_argv[] = { c_path, "-C", "--format=bsd", "-n", argv[1], nullptr };
        execve(path.c_str(), new_argv, environ);
        perror("execve");
        return 1;
    }

    uint64_t address = 0;
    std::string name;
    std::string str_address;
    std::string::iterator line_start, line_end;
    line_start = str.begin();

    FILE* out = fopen(argv[2], "wb");
    if (out == nullptr) {
        perror("fopen");
        return 1;
    }
    if (0 < fseek(out, 0, SEEK_SET)) {
        perror("fseek");
        return 1;
    }

    while (line_start != str.end()) {
        line_end = std::find(line_start, str.end(), '\n');
        name = std::string(line_start + 19, line_end);
        str_address = std::string(line_start, line_start + 16);
        address = strtoul(str_address.c_str(), nullptr, 16);
        uint8_t* raw_address = (uint8_t*)&address;
        for (uint8_t i = 0; i < 8; i++)
            fputc(raw_address[i], out);
        fputc(0, out);
        fprintf(out, "%s", name.c_str());
        fputc(0, out);

        if (line_end == str.end())
            break;
        line_start = line_end + 1;
    }

    return 0;
}
