gcc main.c
unbuffer ./a.out 1001 1002 1003 1004 1005 > 1.out &
unbuffer ./a.out 1002 1001 1003 1004 1005 > 2.out &
unbuffer ./a.out 1003 1002 1001 1004 1005 > 3.out &
unbuffer ./a.out 1004 1002 1003 1001 1005 > 4.out &
unbuffer ./a.out 1005 1002 1003 1004 1001 > 5.out &
