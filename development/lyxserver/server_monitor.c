/*
 * LyXServer monitor. To send command to a running instance of LyX
 * and receive information from LyX.
 * 
 * To build it type:
 * > gcc -g server_monitor.c -o monitor -lforms -L/usr/X11/lib -lX11 -lm
 * > ./monitor
 * Before you run lyx uncomment the line "\serverpipe" from your
 * ~/.lyx/lyxrc file, according with your home path.
 * 
 * Created: 970531
 * Copyright (C) 1997 Alejandro Aguilar Sierra (asierra@servidor.unam.mx)
 * Updated: 980104
 * Copyright (C) 1998 Asger Alstrup (alstrup@diku.dk)
 */


/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>    

int lyx_listen = 0;
int  pipein=-1, pipeout=-1;
char pipename[100];        
char const *clientname = "monitor";

/**** Forms and Objects ****/

typedef struct {
	FL_FORM *server;
	void *vdata;
	long ldata;
	FL_OBJECT *pipename;
	FL_OBJECT *command;
	FL_OBJECT *arg;
	FL_OBJECT *info;
	FL_OBJECT *client;
	FL_OBJECT *submit;
	FL_OBJECT *notify;
} FD_server;


enum { LS_DUMMY=1, LS_OPEN, LS_CLOSE, LS_SUBMIT };

static FD_server *fd_server;

void closepipe();
void server_cb(FL_OBJECT *ob, long data);


FD_server *create_form_server(void)
{
  FL_OBJECT *obj;
  FD_server *fdui = (FD_server *) fl_calloc(1, sizeof(*fdui));

  fdui->server = fl_bgn_form(FL_NO_BOX, 620, 260);
  obj = fl_add_box(FL_UP_BOX,0,0,620,260,"");
  fdui->pipename = obj = fl_add_input(FL_NORMAL_INPUT,110,10,190,40,"Pipe name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,server_cb,0);
    fl_set_object_shortcut(obj,"#P",1);
  fdui->client = obj = fl_add_input(FL_NORMAL_INPUT,410,10,190,40,"Client name");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,server_cb,0);
    fl_set_object_shortcut(obj,"#N",1);
  fdui->command = obj = fl_add_input(FL_NORMAL_INPUT,110,60,190,40,"Command");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,server_cb,0);
    fl_set_object_shortcut(obj,"#C",1);
  fdui->arg = obj = fl_add_input(FL_NORMAL_INPUT,410,60,190,40,"Argument");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,server_cb,0);
    fl_set_object_shortcut(obj,"#A",1);
  obj = fl_add_text(FL_NORMAL_TEXT,10,110,100,40,"Info");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  fdui->info = obj = fl_add_text(FL_NORMAL_TEXT,110,110,490,40,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT,10,160,100,40,"Notify");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  fdui->notify = obj = fl_add_text(FL_NORMAL_TEXT,110,160,490,40,"");
    fl_set_object_boxtype(obj,FL_DOWN_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_button(FL_NORMAL_BUTTON,10,210,140,40,"Open pipes");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,server_cb,LS_OPEN);
    fl_set_object_shortcut(obj,"#O",1);
  obj = fl_add_button(FL_NORMAL_BUTTON,160,210,140,40,"Close pipes");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,server_cb,LS_CLOSE);
    fl_set_object_shortcut(obj,"#l",1);
  fdui->submit = obj = fl_add_button(FL_NORMAL_BUTTON,310,210,170,40,"Submit Command");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,server_cb,LS_SUBMIT);
    fl_set_object_shortcut(obj,"#S",1);
  obj = fl_add_button(FL_NORMAL_BUTTON,490,210,120,40,"Done");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_shortcut(obj,"#D^[",1);
  fl_end_form();

  fdui->server->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/


void io_cb(int fd, void *data)
{
    int n;
    char s[255];

    n = read(fd, &s[0], 200);
    if (n>=0)
      s[n] = 0;
    fprintf(stderr, "monitor: Coming: %s\n", s);
    if (strncmp(s, "LYXSRV:", 7) == 0) {
	if (strstr(s, "bye")) {
	    lyx_listen = 0;
	    fprintf(stderr, "monitor: LyX has closed connection!\n");
	    closepipe();
	    return;
	}
	if (strstr(s, "hello")) {
	    lyx_listen = 1;
	    fprintf(stderr, "monitor: LyX is listening!\n");
	    fl_set_object_lcol(fd_server->submit,FL_BLACK);
	    fl_activate_object(fd_server->submit);
	}                            
    }
    if (s[0]=='I')
      fl_set_object_label(fd_server->info, s);
    else
      fl_set_object_label(fd_server->notify, s); 
}

void openpipe()
{
    char buf[100];
    clientname = fl_get_input(fd_server->client);
    if (pipein==-1) {
        char *pipename = strdup(fl_get_input(fd_server->pipename));
        char s[255];

        fprintf(stderr, "monitor: Opening pipes ");
        strcpy(s, pipename);
        strcat(s, ".in");
        fprintf(stderr, "[%s] and ", s);
        pipein = open(s, O_RDWR);
        strcpy(s, pipename);
        strcat(s, ".out");
        fprintf(stderr, "[%s]\n", s);
        pipeout = open(s, O_RDONLY|O_NONBLOCK);
        fprintf(stderr, "monitor: Opened %d %d\n", pipein, pipeout);

	if (pipein<0 || pipeout<0) {
	    perror("monitor: Couldn't open the pipes");
	    pipein = pipeout = -1;
	    return;
	}
	fl_add_io_callback(pipeout, FL_READ, io_cb, 0);
	
	// greet LyX
	sprintf(buf, "LYXSRV:%s:hello\n", clientname);
	write(pipein, buf, strlen(buf));
	free(pipename);
    } else 
        fprintf(stderr, "monitor: Pipes already opened, close them first\n");
}             


void closepipe()
{
    char buf[100];
    
    if (pipein==-1 && pipeout==-1) {
	fprintf(stderr, "monitor: Pipes are not opened\n");
	return;
    }
	
    if (pipein>=0) {
	if (lyx_listen) {
	    lyx_listen = 0;
	    /* Say goodbye */
	    sprintf(buf, "LYXSRV:%s:bye\n", clientname);
	    write(pipein, buf, strlen(buf));
	}
	close(pipein);
    }
	      
    if (pipeout>=0) {
	close(pipeout);
	fl_remove_io_callback(pipeout, FL_READ, io_cb);
    }     
    pipein = pipeout = -1;
    fl_set_object_lcol(fd_server->submit,FL_INACTIVE);
    fl_deactivate_object(fd_server->submit);    
}


void submit()
{
    char s[255];
 
    const char *clientname = fl_get_input(fd_server->client);
    const char *argument = fl_get_input(fd_server->arg);
    const char *command = fl_get_input(fd_server->command);
    
    sprintf(s, "LYXCMD:%s:%s:%s\n", clientname, command, argument);
    fprintf(stderr, "monitor: command: %s\n", s);
    if (pipein>=0) 
      write(pipein, s, strlen(s));
    else
      fprintf(stderr, "monitor: Pipe is not opened\n");
}


/* callbacks for form server */
void server_cb(FL_OBJECT *ob, long data)
{

    switch (data) {
     case 0: break;
     case LS_OPEN:
	openpipe();
	break;
     case LS_CLOSE:
	closepipe();
	break;
     case LS_SUBMIT:
	submit();
	break;
     case LS_DUMMY:
	break;
    }
}


int main(int argc, char *argv[])
{
    fl_initialize(&argc, argv, 0, 0, 0);
    fd_server = create_form_server();

    strcpy(pipename, getenv("HOME"));
    strcat(pipename, "/.lyxpipe"),
    
    /* fill-in form initialization code */
    fl_set_input(fd_server->pipename, pipename);
    fl_set_input(fd_server->client, clientname);
    fl_deactivate_object(fd_server->submit);
    fl_set_object_lcol(fd_server->submit,FL_INACTIVE);
    
    /* show the first form */
    fl_show_form(fd_server->server,FL_PLACE_MOUSE,FL_FULLBORDER,"LyX Server Monitor");
    fl_do_forms();
    if (pipein != -1)
      closepipe();
    return 0;
}

