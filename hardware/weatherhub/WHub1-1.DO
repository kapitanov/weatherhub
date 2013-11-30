# ACCEL PCB V19.02.9589 Auto-Generated DO File
# Thu Oct 24 23:31:31 2013
#
bestsave on $\best.w
status_file $\progress.sts
#
unit mil
#
grid wire 25.000000 
grid via 25.000000 
#
rule pcb (width 35.0)
#
bus diagonal
route 50
clean 4
route 50 16
clean 4
filter 5
route 100 16
clean 2
delete conflicts
#
write wire $\WHub1-1.w
spread 
miter
write wire $\WHub1-1.m
#
write session $\WHub1-1.ses
report status $\WHub1-1.sts
