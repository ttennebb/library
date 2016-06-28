// Copyright 2012-16 Turtle Ranch LLC
// t4t0nk4
// Ruby extension class for semaphores / shared memory
//

//#include "/usr/lib/ruby/1.8/i486-linux/ruby.h"
#include "/usr/include/ruby-1.9.1/ruby.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define _POSIX_SOURCE 1         /* POSIX compliant source */

//static char ch;
static int shmid;
static char *shm;//,*s;
static sem_t *mutex;

static char sem_name[32];
static key_t key;
static int shm_size; // 27
 
//static VALUE cv_sem;
typedef struct
{
  
} PARMS, *PPARMS;

static VALUE
r_sem_open(VALUE self, VALUE init, VALUE server)
{
  //create & initialize semaphore
  if(NUM2INT(server) > 0)
  {
    printf("server : %d\n", NUM2INT(server));
    printf("init   : %d\n", NUM2INT(init));
    mutex = sem_open(sem_name, O_CREAT, 0644, NUM2INT(init));
  }
  else
    mutex = sem_open(sem_name, 0, 0644, NUM2INT(init));
  if(mutex == SEM_FAILED)
  {
    if(NUM2INT(server) > 0)
    {
      perror("unable to create semaphore");
      sem_unlink(sem_name);
    }
    else
    {
      perror("reader:unable to execute semaphore");
      sem_close(mutex);
    }
    return INT2NUM(-1);
  }
  return INT2NUM(0);
}

static VALUE
r_shmget(VALUE self, VALUE server)
{
  //create the shared memory segment with this key
  if(NUM2INT(server) > 0)
    shmid = shmget(key, shm_size, IPC_CREAT|0666);
  else
    shmid = shmget(key, shm_size, 0666);
  if(shmid < 0)
  {
    perror("failure in shmget");
    return INT2NUM(-1);
  }
  return INT2NUM(shmid);
}

static VALUE
r_shmat(VALUE self)
{
  //attach this segment to virtual memory
  shm = shmat(shmid, NULL, 0);
  return self;
}

static VALUE
r_sem_wait(VALUE self)
{
  sem_wait(mutex);
  return self;
}

static VALUE
r_sem_post(VALUE self)
{
  sem_post(mutex);
  return self;
}

static VALUE
r_sem_put_char(VALUE self, VALUE data)
{
  static char c;
  c = NUM2CHR(data);
  *shm = c;
  return CHR2FIX(c);
}

static VALUE
r_sem_get_char(VALUE self)
{
  static char c;
  c = *shm;
  return CHR2FIX(c);
}

static VALUE
r_sem_put_string(VALUE self, VALUE s)
{
  VALUE str = StringValue(s);
  
  strcpy(shm, RSTRING_PTR(str));
  return str;
}

static VALUE
r_sem_get_string(VALUE self)
{
  return rb_str_new2(shm);
}

static VALUE
r_sem_get_params(VALUE self)
{
  printf("shmid    : %d\n", shmid);
  printf("sem_name : %s\n", sem_name);
  printf("key      : %d\n", key);
  printf("size     : %d\n", shm_size);
  return self;
}

static VALUE
r_sem_close(VALUE self, VALUE server)
{
  sem_close(mutex);
  if(NUM2INT(server) > 0)
    sem_unlink(sem_name);
  shmctl(shmid, IPC_RMID, 0);
  return self;
}

static VALUE
r_init(VALUE self, VALUE name, VALUE size, VALUE keyid)
{
  
  VALUE s = StringValue(name);
  strncpy(sem_name, RSTRING_PTR(s), 32);
  printf("sem_name : %s\n", sem_name);
	shm_size = NUM2INT(size);
  //name the shared memory segment
  key = NUM2INT(keyid);
  return self;
}


void Init_rb_sem()
{
  //VALUE rbsem_module = rb_define_module("RBSem");
  VALUE cv_sem = rb_define_class("RBSemClass", rb_cObject);
  rb_define_method(cv_sem, "initialize", r_init, 3);
  rb_define_method(cv_sem, "rsem_open", r_sem_open, 2);
  rb_define_method(cv_sem, "rshmget", r_shmget, 1);
  rb_define_method(cv_sem, "rshmat", r_shmat, 0);
  rb_define_method(cv_sem, "rsem_wait", r_sem_wait, 0);
  rb_define_method(cv_sem, "rsem_post", r_sem_post, 0);
  rb_define_method(cv_sem, "rsem_close", r_sem_close, 1);
  rb_define_method(cv_sem, "rsem_putc", r_sem_put_char, 1);
  rb_define_method(cv_sem, "rsem_getc", r_sem_get_char, 0);
  rb_define_method(cv_sem, "rsem_puts", r_sem_put_string, 1);
  rb_define_method(cv_sem, "rsem_gets", r_sem_get_string, 0);
  rb_define_method(cv_sem, "rsem_params", r_sem_get_params, 0);
  //rb_define_method(cv_sem, "", );
}
