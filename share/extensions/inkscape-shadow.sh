convert -mattecolor "#ffff" -frame $((${2} * 2))x$((${2} * 2)) -fx '1-a' -channel A -blur $((${2} * 3))x${2}  $1 $1 
