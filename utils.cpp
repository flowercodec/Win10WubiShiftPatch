#include "utils.h"

#include <algorithm>

static int run_as_daemon  = 0;

#ifdef HAVE_TERMIOS_H
	/* init terminal so that we can grab keys */
	static struct termios oldtty;
	static int restore_tty;
#endif

void term_exit(void)
{
	//printf("term_exit");
#ifdef HAVE_TERMIOS_H
	if(restore_tty)
		tcsetattr (0, TCSANOW, &oldtty);
#endif
}

static void sigterm_handler(int sig)
{
	term_exit();
}

void term_init(void)
{
	setvbuf(stderr,NULL,_IONBF,0);
#ifdef HAVE_TERMIOS_H
	if(!run_as_daemon){
		struct termios tty;
		int istty = 1;
#if HAVE_ISATTY
		istty = isatty(0) && isatty(2);
#endif
		if (istty && tcgetattr (0, &tty) == 0) {
			oldtty = tty;
			restore_tty = 1;

			tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
				|INLCR|IGNCR|ICRNL|IXON);
			tty.c_oflag |= OPOST;
			tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
			tty.c_cflag &= ~(CSIZE|PARENB);
			tty.c_cflag |= CS8;
			tty.c_cc[VMIN] = 1;
			tty.c_cc[VTIME] = 0;

			tcsetattr (0, TCSANOW, &tty);
		}
		signal(SIGQUIT, sigterm_handler); /* Quit (POSIX).  */
	}
	signal(SIGINT , sigterm_handler); /* Interrupt (ANSI).    */
	signal(SIGTERM, sigterm_handler); /* Termination (ANSI).  */
#endif
}

int read_key()
{
    unsigned char ch;
#if HAVE_TERMIOS_H
    int n = 1;
    struct timeval tv;
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    n = select(1, &rfds, NULL, NULL, &tv);
    if (n > 0) {
        n = read(0, &ch, 1);
        if (n == 1)
            return ch;

        return n;
    }
#elif HAVE_KBHIT
#    if HAVE_PEEKNAMEDPIPE
    static int is_pipe;
    static HANDLE input_handle;
    DWORD dw, nchars;
    if(!input_handle){
        input_handle = GetStdHandle(STD_INPUT_HANDLE);
        is_pipe = !GetConsoleMode(input_handle, &dw);
    }

    if (is_pipe) {
        /* When running under a GUI, you will end here. */
        if (!PeekNamedPipe(input_handle, NULL, 0, NULL, &nchars, NULL)) {
            // input pipe may have been closed by the program that ran ffmpeg
            return -1;
        }
        //Read it
        if(nchars != 0) {
            read(0, &ch, 1);
            return ch;
        }else{
            return -1;
        }
    }
#    endif
    if(kbhit())
        return(getch());
#endif
    return -1;
}

bool set_debug_privilege()
{
	TOKEN_PRIVILEGES tp;
	HANDLE hToken;
	LUID luid;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken ))
		return 0;

	if(!LookupPrivilegeValueW(L"", L"SeDebugPrivilege", &luid))
		return 0;

	tp.PrivilegeCount         = 1;
	tp.Privileges[0].Luid     = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	return AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL );
}


std::string get_system_winodws_path()
{
	std::string ret = "C:\\";
	char str[MAX_PATH];       
	if (GetWindowsDirectoryA(str, MAX_PATH) > 0) {
		ret = str;
		ret += "\\";
	}
	return ret;
}

std::wstring lower_case(const std::wstring& text)
{
	std::wstring ret = text;
	std::transform(ret.begin(), ret.end(), ret.begin(), tolower);
	return ret;
}

std::string lower_case(const std::string& text)
{
	std::string ret = text;
	std::transform(ret.begin(), ret.end(), ret.begin(), tolower);
	return ret;
}
