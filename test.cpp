#include <iostream>
#include <fstream>
#include <termios.h>
#include <unistd.h> // For read()
#include <string>
#include <sys/ioctl.h>



using namespace std;

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
char get_char(){
	char ch {};
	if (read(0, &ch, 8) == 1){ // Do i need to uses read or is there a better way 
		return ch;
	}
	return NULL;

}

int main() {
    set_raw();
    ofstream stdout("/dev/stdout");
    struct winsize ws = {(struct winsize){0,0,0,0}};
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    cout << "\x1B[2J";
    cout << "\x1b[H";
    cout << "Hello terminal" <<  ws.ws_row << " " << ws.ws_col << endl;
    int index {0};
    while (true){
		char ch = get_char();
		if (ch == NULL) {
			continue;
		}
		if (ch == 'q'){
			break;
		}
        cout << ch;
        cout.flush(); // is there a better way then flushing every loop;
        ++index;
	}
	cout << endl;
    restore();

   
    return 0;
}