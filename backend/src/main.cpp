#include <unistd.h>
#include <string>

int main () {

    std::string buff = "Hello World!";
    write(2, buff.c_str(), buff.size());
    return 0;

}