#!/usr/bin/ruby1.8

require 'rb_sem.so'

CLIENT = 0

sem_capture = RBSem.new('sem_capture', 32, 1001)
#sem_start   = RBSem.new('sem_start', 1,1002)

if(sem_capture.rsem_open(0, CLIENT) != 0)
  print "ah crap - capture\n"
  exit 0
end

if(sem_capture.rshmget(CLIENT) == -1)
  print "great googli moogli capture\n"
  exit 0
end
=begin
if(sem_start.rsem_open(0, CLIENT) != 0)
  print "ah crap - start\n"
  exit 0
end

if(sem_start.rshmget(CLIENT) != 0)
  print "great googli moogli start\n"
  exit 0
end
=end
sem_capture.rshmat
#sem_start.rshmat

print "client is attached\n"
print "releasing the server...\n"
sem_capture.rsem_putc('x')
#sem_start.rsem_post
#sleep 1
while true
  sem_capture.rsem_wait
  break if(sem_capture.rsem_getc == 0)
  printf("got sem data : %d\n", sem_capture.rsem_getc)
  #sem_capture.rsem_post
  #sleep 1
end

sem_capture.rsem_close(CLIENT)
#sem_start.rsem_close(CLIENT)


