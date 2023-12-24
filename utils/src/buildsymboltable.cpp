#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/wait.h>

#include <sstream>
#include <string>
#include <algorithm>

// we read the output of nm from stdin and write the symbol table to <out-file>
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <out-file>", argv[0]);
        return 1;
    }

    std::string str = "";
    char c = getchar();
    while (c != EOF) {
        str += c;
        c = getchar();
    }

    uint64_t address = 0;
    std::string name;
    std::string str_address;
    std::string::iterator line_start, line_end;
    line_start = str.begin();

    FILE* out = fopen(argv[1], "wb");
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
