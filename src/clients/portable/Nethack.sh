#!/bin/sh
#	SCCS Id: @(#)nethack.sh	3.0	89/08/11

HACKDIR=/usr/games/lib/nethackdir
HACK=$HACKDIR/nethack
MAXNROFPLAYERS=4

# see if we can find the full path name of PAGER, so help files work properly
# assume that if someone sets up a special variable (HACKPAGER) for NetHack,
# it will already be in a form acceptable to NetHack
# ideas from brian@radio.astro.utoronto.ca
if test \( "xxx$PAGER" != xxx \) -a \( "xxx$HACKPAGER" = xxx \)
then

	HACKPAGER=$PAGER

#	use only the first word of the pager variable
#	this prevents problems when looking for file names with trailing
#	options, but also makes the options unavailable for later use from
#	NetHack
	for i in $HACKPAGER
	do
		HACKPAGER=$i
		break
	done

	if test ! -f $HACKPAGER
	then
		IFS=:
		for i in $PATH
		do
			if test -f $i/$HACKPAGER
			then
				HACKPAGER=$i/$HACKPAGER
				export HACKPAGER
				break
			fi
		done
		IFS=' 	'
	fi
	if test ! -f $HACKPAGER
	then
		echo Cannot find $PAGER -- unsetting PAGER.
		unset HACKPAGER
		unset PAGER
	fi
fi

GRAPHCHARS=" \mx\mq\ml\mk\mm\mj\mn\mv\mw\mu\mt\mx\mq\m\m/\m(\m)\m)\m+\m~\m#\m<\m>\m\^\m\"\m}\m{\mA\mB\mC\mD\mE\mF\mG"

ESC=""

NAME=`tty`
cd $HACKDIR
case $1 in
	-s*)
		exec $HACK $@
		;;
	*)
		if [ "`expr $NAME : '/dev/w' `" != "0" ]
		then
			/etc/setf $HACKDIR/hack.ft 1
			NETHACKOPTIONS="$NETHACKOPTIONS,graphics:$GRAPHCHARS"
			$HACK $@ $MAXNROFPLAYERS
			/etc/setf /usr/lib/wfont/BLD.ft 1
		elif [ "$TERM" = "mgr" ]
		then
			#
			# turn off echo
			#
			stty -echo
			#
			# find out where we are
			#
			echo "${ESC}4I\c"
			read startx starty width height
			echo "${ESC}2I\c"
			read width height
			#
			# Find out info about current font, font 1 and font 14
			# since we will use 1 and 14 for standard and
			# alternate fonts
			#
			echo "${ESC}3I\c"
			read wide1 high1 num1 name1
			echo "${ESC}14F\c"
			echo "${ESC}3I\c"
			read wide2 high2 num2 name2
			echo "${ESC}1F\c"
			echo "${ESC}3I\c"
			read wide3 high3 num3 name3
			#
			# load our fonts
			#
			echo "${ESC}1,12F3b1f8x11.fnt\c"
			echo "${ESC}14,9Fhack2.fnt\c"
			echo "${ESC}1F\c"
			#
			# put window where we want it and size it
			#
			echo "${ESC}0,50,80,24w\c"
			#
			# set up termcap
			#
			eval `set_emacs | sed 's/:/:as=\\\E14F:ae=\\\E1F:/'`
			trap '
			echo "Press <Return> to continue: \c"
			read fubar
			#
			# restore fonts
			#
			len=`length ${name2}`
			echo "${ESC}14,${len}F${name2}\c"
			len=`length ${name3}`
			echo "${ESC}1,${len}F${name3}\c"
			echo "${ESC}${num1}F\c"
			#
			# put window back
			#
			echo "${ESC}${startx},${starty},${width},${height}w\c"
			#
			# reset termcap
			#
			eval `set_emacs`
			clear
			' 0 1 2 3 9 15
			#
			# run the game
			#
			stty echo
			NETHACKOPTIONS="$NETHACKOPTIONS,graphics:$GRAPHCHARS"
			$HACK $@ $MAXNROFPLAYERS
		else
			exec $HACK $@ $MAXNROFPLAYERS
		fi
		;;
esac
