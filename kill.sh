ps ax | grep a.out | grep -v grep | sed "s:^ ::" | cut -f 1 -d ' ' | xargs kill -9
