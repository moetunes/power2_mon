/* A light battery warning system
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 2 of the License, or
*  (at your option) any later version.
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>

#define SYS_FILE "/sys/class/power_supply/BAT0/uevent"
#define MIN_PERCENT 25
/* 100 for full charge */
#define MAX_PERCENT 95
#define SEARCHTERM1 "POWER_SUPPLY_STATUS"
#define SEARCHTERM2 "POWER_SUPPLY_CHARGE_FULL="
#define SEARCHTERM3 "POWER_SUPPLY_CHARGE_NOW="

static unsigned int SLEEP_TIME = 30;
static char *text = "FILE";
static char text1[30] = "ERROR";
static Window win;
static void info_return();
static void *event_loop();
static void wait_to_read();
static void quit_window();
static unsigned int text_width, text_width1, texty, win_run;
static unsigned int screen_num, width, height;
static unsigned long background, border;
static XEvent ev;
static Display *dis;
static char *fontname = "-*-terminus-*-*-*-*-*-*-*-*-*-*-*-*";
static XFontStruct *font;
static GC pen;
static XGCValues values;
static pthread_t pth = NULL;

void *event_loop() {
	while(win_run < 1) {
		XNextEvent(dis, &ev);
		switch(ev.type){
		case Expose:
            XDrawRectangle(dis, win, pen, 7, 7, width-14, height-14);
            XDrawRectangle(dis, win, pen, 12, 12, width-24, height-24);
   			texty = (height + font->ascent)/2;
            text_width = XTextWidth(font, text, strlen(text));
            text_width1 = XTextWidth(font, text1, strlen(text1));
   			XDrawString(dis, win, pen, (width-text_width)/2, texty-font->ascent, text, strlen(text));
   			XDrawString(dis, win, pen, (width-text_width1)/2, texty+font->ascent, text1, strlen(text1));
			break;
		case ConfigureNotify:
			if (width != ev.xconfigure.width || height != ev.xconfigure.height) {
				width = ev.xconfigure.width;
				height = ev.xconfigure.height;
				XClearWindow(dis, ev.xany.window);
			}
			break;
        /* exit if a button is pressed inside the window */
		case ButtonPress:
            quit_window();
            break;
		}
	}
	return 0;
}

void window_loop(){
    if(win_run == 1) {
        dis = XOpenDisplay(NULL);
        if (!dis) {
            fputs("\033[0;31mPOWERMON :: Unable to connect to display\033[0m", stderr);
            return;
        }

        screen_num = DefaultScreen(dis);
        background = WhitePixel(dis, screen_num);
        border = BlackPixel(dis, screen_num);
        width = (XDisplayWidth(dis, screen_num)/5);
        height = (XDisplayHeight(dis, screen_num)/6);
        font = XLoadQueryFont(dis, fontname);
        if (!font) {
            fprintf(stderr, "unable to load preferred font: %s using fixed", fontname);
            font = XLoadQueryFont(dis, "fixed");
        }
        win = XCreateSimpleWindow(dis, DefaultRootWindow(dis),width*4-20,height*5-20,width,height,2,border,background);
        /* create the pen to draw lines with */
        values.foreground = BlackPixel(dis, screen_num);
        values.line_width = 2;
        values.line_style = LineSolid;
        values.font = font->fid;
        pen = XCreateGC(dis, win, GCForeground|GCLineWidth|GCLineStyle|GCFont,&values);
        XSelectInput(dis, win, ButtonPressMask|StructureNotifyMask|ExposureMask );
        XSetTransientForHint(dis, win, DefaultRootWindow(dis));
        XMapWindow(dis, win);
        win_run = 0;
    	pthread_create(&pth, NULL,event_loop,NULL);
    	return;
    } else {
    	text_width = XTextWidth(font, text, strlen(text));
    	text_width1 = XTextWidth(font, text1, strlen(text1));
    	XClearWindow(dis, win);
        XDrawRectangle(dis, win, pen, 7, 7, width-14, height-14);
        XDrawRectangle(dis, win, pen, 12, 12, width-24, height-24);
    	XDrawString(dis, win, pen, 15, 15+font->ascent, "Updated:", 8);
    	XDrawString(dis, win, pen, (width-text_width)/2, texty-font->ascent, text, strlen(text));
        XDrawString(dis, win, pen, (width-text_width1)/2, texty+font->ascent, text1, strlen(text1));
        XFlush(dis);
        return;
    }
}

void quit_window() {
    win_run = 1;
    XUnmapWindow(dis, win);
    XFlush(dis);
    XCloseDisplay(dis);
    return;
}

void info_return() {
    FILE *Batt;
    char  buffer[80];
    char *battstatus, *chargenow, *lastfull;
    unsigned int battdo = 0, dummy;
    long nowcharge, fullcharge;

    Batt = fopen( SYS_FILE, "r" ) ;
    if ( Batt == NULL ) {
        fprintf(stderr, "\t\033[0;31mPOWER_MON:: Couldn't find %s\033[0m \n", SYS_FILE);
        window_loop();
        return;
    } else {
        while(fgets(buffer,sizeof buffer,Batt) != NULL) {
            /* Now look for info
            * first search term to match */
            if(strstr(buffer,SEARCHTERM1) != NULL) {
                battstatus = strstr(buffer, "=");
                if(strcmp(battstatus, "=Discharging\n") == 0)
                    battdo = 2;
                else if(strcmp(battstatus, "=Charging\n") == 0)
                    battdo = 1;
                else if(strcmp(battstatus, "=Charged\n") == 0)
                    battdo = 3;
                else if(strcmp(battstatus, "=Full\n") == 0)
                    battdo = 4;
                else
                    battdo = 5;
            }
            /* Second search term */
            if(strstr(buffer,SEARCHTERM2) != NULL) {
                lastfull = strstr(buffer, "=");
                fullcharge = atoi(lastfull+1);
                //printf("\t%s", lastfull+=1);
            }
            /* Third search term */
            if(strstr(buffer,SEARCHTERM3) != NULL) {
                chargenow = strstr(buffer, "=");
                nowcharge = atoi(chargenow+1);
                //printf("\t%s ", chargenow+=1);
            }
        }
        fclose(Batt);
        if(battdo < 1) {
            text = "FILE";
            snprintf(text1, 5, "ERROR");
            window_loop();
            return;
        }

        dummy = ((float)nowcharge/fullcharge)*100;
        if(win_run == 0 && ((battdo == 2 && dummy > MIN_PERCENT) ||
          battdo == 1)) {
            XEvent e;
            e.type = ButtonPress;
            e.xbutton.button = Button1;
            e.xbutton.same_screen = True;
            XSendEvent(dis, win, True, 0xfff, &e);
            XFlush(dis);
            return;
        }
        if(dummy >= MAX_PERCENT) battdo = 3;
        /* if the battery is above MIN_PERCENT don't show the window
           unless it's above MAX_PERCENT or charged */
        if((dummy <= MIN_PERCENT && battdo == 2) || battdo > 2 || win_run == 0) {
            if(battdo == 2) text = "Power Supply Discharging";
            else if(battdo == 3) text = "Power Supply Charged";
            else if(battdo == 4) text = "Power Supply Full";
            else if(battdo == 5) text = "Power Supply Unknown !!";
            snprintf(text1, 29, "Remaining Charge %d %%", dummy);
            window_loop();
        }
    }
}

void wait_to_read() {
    fd_set set;
    struct timeval timeout;
    int ret;

    /* Initialize the file descriptor set. */
    FD_ZERO (&set);
    FD_SET (1, &set);

    /* Initialize the timeout data structure. */
    timeout.tv_sec = SLEEP_TIME;
    timeout.tv_usec = 0;

    /* select returns 0 if timeout, 1 if input available, -1 if error. */
    //return TEMP_FAILURE_RETRY (select (FD_SETSIZE,&set, NULL, NULL,&timeout));
    ret = select(2,&set,NULL,NULL,&timeout);
    if(ret == 0)
        return;
    else {
        fprintf(stderr, "POWER2_MON:: Timer Fail\nPOWER2_MON:: EXITING\n");
        exit(1);
    }
}

int main() {
    win_run = 1;
    for(;;) {
        wait_to_read();
        info_return();
        if(win_run == 1 && pth) {
            pthread_cancel(pth);
        }
    }
    XFreeGC(dis, pen);
    XFreeFont(dis, font);
    XCloseDisplay(dis);
    return 0;
}
