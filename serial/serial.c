// Copyright 2012-16 Turtle Ranch LLC
// t4t0nk4
// Ruby extension class for communication via RS232 interface
//

//#include "/usr/lib/ruby/1.8/i486-linux/ruby.h"
#include "/usr/include/ruby-1.9.1/ruby.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>

#define _POSIX_SOURCE 1         /* POSIX compliant source */

static VALUE cv_rs232;          /* variable for class being defined */
static int fd;
static char buf[80];
static struct termios oldtio, newtio;

static VALUE
rs232_close(VALUE self)
{
  tcsetattr(fd,TCSANOW,&oldtio);
  close(fd);
  return Qnil;
}

static VALUE
rs232_rcvByte(VALUE self)
{
  read(fd,buf,1);
  return CHR2FIX(buf[0]);
}

static VALUE
rs232_sndByte(VALUE self, VALUE val)
{
  buf[0] = NUM2CHR(val);
  int res = write(fd,buf,1);
  return INT2FIX(res);
}

static VALUE
rs232_rcvData(VALUE self, VALUE nbytes)
{
  int n = FIX2INT(nbytes);
  read(fd,buf,n);
  VALUE ary = rb_ary_new2(n);
  int i; for (i = 0; i < n; i++)
           rb_ary_push(ary, CHR2FIX(buf[i]));
  return ary;
}

static VALUE
rs232_sndData(VALUE self, VALUE ary)
{
  int i, len = RARRAY_LEN(ary);
  buf[0] = len;
  for (i = 0; i < len; i++)
    buf[i+1] = NUM2CHR(rb_ary_entry(ary, i));
  buf[len+1] = 0;
  int res = write(fd,buf,len+1);
  return INT2FIX(res);
}

static VALUE
rs232_rcvString(VALUE self, VALUE nbytes)
{
  int n = FIX2INT(nbytes);
  int res = read(fd,buf,n);
  VALUE str;
  if(res > 0)
    str = rb_str_new(buf,res);
  else 
  {
    strcpy(buf, "\n");
    str = rb_str_new(buf,strlen(buf));
  }
  return str;
}

static VALUE
rs232_rcvChar(VALUE self)
{
  char inpc;  
  read(fd,&inpc,1);
  return CHR2FIX(inpc);
}

static VALUE
rs232_sndString(VALUE self, VALUE str)
{
  int res = write(fd,RSTRING_PTR(str), RSTRING_LEN(str));
  return INT2FIX(res);
}

static VALUE
rs232_flush_buffers(VALUE self)
{
  tcflush(fd,TCIOFLUSH);
  return Qnil;
  
}

static VALUE
rs232_baud_rate(VALUE self, VALUE baud_rate)
{
  newtio.c_cflag = CS8 | CREAD | CLOCAL;
  switch (FIX2INT(baud_rate)) {
  case 115200:
    newtio.c_cflag |= B115200;
    break;
  case 57600:
    newtio.c_cflag |= B57600;
    break;
  case 38400:
    newtio.c_cflag |= B38400;
    break;
  case 19200:
    newtio.c_cflag |= B19200;
    break;
  case 9600:
    newtio.c_cflag |= B9600;
    break;
  default:
    newtio.c_cflag |= B9600;
    break;
  }
  tcflush(fd,TCIOFLUSH);
  tcsetattr(fd,TCSANOW,&newtio);
  return Qnil;
}

static VALUE
rs232_initialize(VALUE self, VALUE port, VALUE mode)
{ 
  //char dev[24];
  //char *cp;
  //VALUE str = StringValue(port);
  //VALUE blk = StringValue(mode);
  //char *cp = RSTRING_PTR(port);

  /*sprintf(dev, "/dev/ttyUSB%d",FIX2INT(port));*/
  if(RSTRING_PTR(port) == NULL)
  {
    perror("Null pointer");
    exit(-1);
  }
  //sprintf(dev, "%s", RSTRING(port)->ptr);
  //fd = open(dev, O_RDWR | O_NOCTTY);
  fd = open(RSTRING_PTR(port), O_RDWR | O_NOCTTY);
  if (fd < 0) { perror("Opening serial port"); exit(-1); }

  tcgetattr(fd,&oldtio);        /* save current port settings */

  bzero(&newtio,sizeof(newtio));
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;
  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag     &= ~(ICANON | ECHO | ECHOE | ISIG);
  if(strcmp("block", RSTRING_PTR(mode)) == 0)
  {
    newtio.c_cc[VTIME] = 0;      /* block */
    newtio.c_cc[VMIN]  = 1;       /* block for one char */
  }
  else
  {
    newtio.c_cc[VTIME] = 10;      /* wait for a sec */
    newtio.c_cc[VMIN]  = 0;       /* block only for a sec as set by vtime */
  }

  rs232_baud_rate(self,INT2FIX(9600));
  return Qnil;
}

void
Init_serial()
{
  cv_rs232 = rb_define_class("ComPort", rb_cObject);
  rb_define_method(cv_rs232, "initialize", rs232_initialize, 2);
  rb_define_method(cv_rs232, "closeComPort", rs232_close, 0);
  rb_define_method(cv_rs232, "rcvByte", rs232_rcvByte, 0);
  rb_define_method(cv_rs232, "sndByte", rs232_sndByte, 1);
  rb_define_method(cv_rs232, "rcvData", rs232_rcvData, 1);
  rb_define_method(cv_rs232, "sndData", rs232_sndData, 1);
  rb_define_method(cv_rs232, "rcvString", rs232_rcvString, 1);
  rb_define_method(cv_rs232, "sndString", rs232_sndString, 1);
  rb_define_method(cv_rs232, "rcvChar", rs232_rcvChar, 0);
  rb_define_method(cv_rs232, "flushBuffers", rs232_flush_buffers, 0);
  rb_define_method(cv_rs232, "baud_rate", rs232_baud_rate, 1);
}
