#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char** argv) {

	openlog(NULL,0,LOG_USER);
	if (argc != 3) {
		syslog(LOG_ERR, "Incorrect Number of Arguments: %d you should enter the filename and string", argc);
		closelog();
		return 1;
	}
	else {
		int fd= open(argv[1],O_WRONLY|O_CREAT,0666);
		if ( fd == -1 ){
			syslog(LOG_ERR, "Unable to open or create %s", argv[1]);
			closelog();
			return -1;
		}else{
			syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);

			closelog();

			write(fd, argv[2], strlen(argv[2]));

			close(fd);
		}
	}
	return 0;
}
