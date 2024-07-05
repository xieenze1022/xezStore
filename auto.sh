#!/bin/bash

compile()
{
	cd kernel/
	make clean && make

	cd ../user/
	make clean && make
}

clean()
{
	cd kernel/
	make clean

	cd ../user/
	make clean
}

menu_start()
{
	case $1 in
		make)
			compile	
			;;

		clean)
			clean
			;;

		*)
			echo "usage:"
			echo "$0 make | clean"
			exit 1
			;;
	esac
}

menu_start $1
