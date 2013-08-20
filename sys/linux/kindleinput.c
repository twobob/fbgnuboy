
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "input.h"
#include "rc.h"

static int inputfds[] = { -1, -1, -1 };

rcvar_t joy_exports[] =
{
	RCV_END
};

void joy_init()
{
	int c;
	char dev[128];

	for(c=0; c < 3; c++) {
		sprintf(dev, "/dev/input/event%d", c);
		inputfds[c] = open(dev, O_RDONLY|O_NONBLOCK);
		if(inputfds[c] < 0) {
			fprintf(stderr, "couldn't open <%s>\n", dev);
		}
	}
}

void joy_close()
{
	int c;
	for(c = 0; c < 3; c++) {
		if(inputfds[c] < 0) continue;
		close(inputfds[c]);
	}
}

void joy_poll()
{
	event_t ev;
	struct input_event iev;
	int c;
	

	for(c = 0; c < 3; c++) {
		if(inputfds[c] < 0) continue;
		while (read(inputfds[c],&iev,sizeof(struct input_event)) == sizeof(struct input_event)) {
			if(iev.type == EV_KEY) {
				if(iev.value == 1) {
					ev.type = EV_PRESS;
				} else if(iev.value == 0) {
					ev.type = EV_RELEASE;
				} else {
					continue;
				}
				switch(iev.code) {
				case 103:
				case 122: // FW_UP (K3, KDX)
					ev.code = K_JOYUP;
					break;
				case 108:
				case 123: // FW_DOWN (K3, KDX)
					ev.code = K_JOYDOWN;
					break;
				case 105: // FW_LEFT (both)
					ev.code = K_JOYLEFT;
					break;
				case 106: // FW_RIGHT (both)
					ev.code = K_JOYRIGHT;
					break;
				case 102:
				case 98:  // HOME
					ev.code = K_JOY6;
					break;
				case 44:  // Z
					ev.code = K_JOY1;
					break;
				case 45:  // X
					ev.code = K_JOY0;
					break;
				case 30:  // A
					ev.code = K_JOY2;
					break;
				case 31:  // S
					ev.code = K_JOY3;
					break;
				case 16:  // Q
					ev.code = K_JOY4;
					break;
				case 17:  // W
					ev.code = K_JOY5;
					break;
				default:
					continue;
				}
				ev_postevent(&ev);
			}
		}
	}
}
