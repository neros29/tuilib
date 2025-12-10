#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include "../tui.h"


void set_raw() {
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(0, TCSANOW, &term);
}

void restore() {
    struct termios term;
    tcgetattr(0, &term);
    term.c_lflag = (ECHO | ICANON);
    tcsetattr(0, TCSANOW, &term);
}
bool is_char(){
    fd_set readfds;
    struct timeval timeout {0, 3};
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    int retval = select(0 + 1, &readfds, NULL, NULL, &timeout);
    if (retval){
        return true;
    }
    else {
        return false;
    }


}
char get_char(){
    char ch {};
    set_raw();
    if (is_char()){
        read(0, &ch, 1);
    }
    restore();
    return ch;
}
