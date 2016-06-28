#!/usr/bin/ruby1.8

require 'rb_sem.so'

SERVER = 1

sem_capture = RBSem.new('sem_capture', 32, 1001)
#sem_start   = RBSem.new('sem_start', 1, 2002)

if(sem_capture.rsem_open(0, SERVER) != 0)
  print "drat - capture\n"
  exit 0 
end
shmid = sem_capture.rshmget(SERVER)
if(shmid == -1)
  print "double drat - capture\n"
  exit 0
end
printf("capture shmid : %d\n", shmid)
=begin
shmid = 0
if(sem_start.rsem_open(0, SERVER) != 0)
  print "drat - start\n"
  exit 0 
end
shmid = sem_start.rshmget(SERVER)
if(shmid == -1)
  print "double drat - start\n"
  exit 0
end
printf("capture shmid : %d\n", shmid)
=end

sem_capture.rshmat
#sem_start.rshmat
sem_capture.rsem_putc('c')

print "we're attached\n"
print "waiting for the client to release me...\n"
#sem_start.rsem_wait
while true
  break if 120 == sem_capture.rsem_getc
  sleep 1
end


1.upto 20 do |i|
  printf("i : %d\n", i)
  sem_capture.rsem_putc(i)
  sem_capture.rsem_post
  sleep 1
end
#sleep 1
sem_capture.rsem_putc(0)
sem_capture.rsem_post

sem_capture.rsem_close(SERVER)
#sem_start.rsem_close(SERVER)
