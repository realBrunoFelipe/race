#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>

//#include "term.h" 

void setEnvironment();
void checkTerminal();
void saveAttributes();
void setMenuMode();
void setRaceMode(); 
void setNoncanonicalMode(int vmin, int vtime);
void registerSigIntCatcher();
void sigIntCatcher(int signum);
void disableRepeatAndCursor();
void resetEnvironment();
void resetInputMode();
void enableRepeatAndCursor();

//////////////////////////////

struct termios saved_attributes;

//////// AT START ////////////

void setEnvironment() {
	checkTerminal();
	saveAttributes();
	setMenuMode();
	registerSigIntCatcher();
	disableRepeatAndCursor();
}

void checkTerminal() {
	if (!~isatty(STDIN_FILENO)) {
		printf("Not a terminal: %d.\n", STDIN_FILENO);
		exit(EXIT_FAILURE);
	}
}

void saveAttributes() {
	tcgetattr(STDIN_FILENO, &saved_attributes);
}

// blocking mode (getc waits for input) 
void setMenuMode() {
	setNoncanonicalMode(1, 0);
}

// nonblocking mode (getc does not wait for input, it returns every 0.1 s)
void setRaceMode() {
	setNoncanonicalMode(0, 1);
}

void setNoncanonicalMode(int vmin, int vtime) {
	struct termios tattr;
	char *name;
	// set noncanonical mode, disable echo
	atexit(resetEnvironment);
	tcgetattr(STDIN_FILENO, &tattr);
	tattr.c_lflag &= ~(ICANON|ECHO);
	tattr.c_cc[VMIN] = vmin;
	tattr.c_cc[VTIME] = vtime;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}

void registerSigIntCatcher() {
	struct sigaction action;
	action.sa_handler = sigIntCatcher;
	sigaction(SIGINT, &action, NULL);
}

// method that gets executed when ctrl-c is pressed.
// necesary so that at_exit method gets executed,
// that sets terminal back to the original state.
void sigIntCatcher(int signum) {
	exit(0);
}

void disableRepeatAndCursor() {
	// disable repeat
	system("xset -r"); 
	// set cursor off. could also probably use system("setterm -cursor off);
	printf("\e[?25l");
	fflush(stdout);
}

///////// AT END ////////////

void resetEnvironment() {
	resetInputMode();
	enableRepeatAndCursor();
}

void resetInputMode() {
	// bring back canonical mode
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

void enableRepeatAndCursor() {
	// enable repeat	
	system("xset r");
	system("clear");
	// bring back cursor
	printf("\e[?25h");
	fflush(stdout) ;
}
