;;; GNU Emacs Lisp code to support running the Mgr window system.
;;; Author:  Vincent Broman, broman@nosc.mil, 24 August 1993, v1.2
;;;
;;; This file can be installed as lisp/term/mgr.el .
;;; Since no C code is involved, window-system is not set early enough
;;; to get this loaded from lisp/term/mgr-win.el .
;;; If your Emacs contains code for X or other window systems,
;;; invoke it with the -nw option, and make sure $TERM starts with "mgr".
;;; Ideally, run set_emacs to get your TERMCAP set,
;;; although the screen width/height is adjusted whenever needed.
;;;
;;; The customizations at the end makes the right mouse button move point,
;;; and possibly pushes a mark, while the midde button calls up the menus.
;;; 
(setq window-system 'mgr)
(defvar mgr-env-count 0)
(defvar mgr-default-esc 27)
(defvar mgr-esc mgr-default-esc)
(defvar mgr-delim 5
  "Delimiter character between menu selection/action items.
Must not collide with any character appearing in a menu selection
or in an action key-sequence.")

(defvar mgr-pixel-limit 1152)		; updated below.  why just one limit?
(defun mgr-constrain (x)
  (max 0 (min mgr-pixel-limit x)))

(defvar mgr-vi-mode-on nil		; set below
  "Flag needed to avoid unexpected garbage on mouse clicks.")

(defvar mgr-icondir "/usr/mgr/icon")	; readable by all icons


(defun mgr-putchar (c)
  (send-string-to-terminal (char-to-string c)))

(defun mgr-send (str)
  "Write to the Mgr terminal a string or list of character numbers."
  (if (stringp str)
      (send-string-to-terminal str)
    (while (consp str)
      (mgr-putchar (car str))
      (setq str (cdr str)))))

(defun mgr-flush ()
  "Function send-string-to-terminal does not seem to need flushing."
  nil)

(defun mgr-printstr (str)
  (mgr-send str))

(defun mgr-getchar ()
  "Return a single char read from the terminal without echoing."
  (let ((echo-keystrokes 10)
	(cursor-in-echo-area nil))
    (read-char)))

(defun mgr-gets ()
  "Return a string of one line read from the terminal, without the newline.
Error detection, when implemented, would return nil.
This function is intended for reading responses from Mgr."
  (let ((echo-keystrokes 10)
	(cursor-in-echo-area nil))
    (let ((charlist nil)
	  (c (read-char)))
      (while (not (= ?\n c))
	(setq charlist (cons c charlist))
	(setq c (read-char)))
      (concat (nreverse charlist)))))

(defun mgr-setesc (x)
  (setq mgr-esc x))

(defun mgr-resetesc ()
  (setq mgr-esc mgr-default-esc))

;;; I obtained the following by editing include/term.h and include/window.h
;;; from the Mgr source, so here's the credit...
;
;;                        Copyright (c) 1987 Bellcore
;;                            All Rights Reserved
;;       Permission is granted to copy or use this program, EXCEPT that it
;;       may not be sold for profit, the copyright notice must be reproduced
;;       on copies, and credit should be given to Bellcore where it is due.
;;       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
;;

; translate MACRO names from C to Elisp as follows:
; 
; m_addlines(n)	-> (mgr-addlines n)
; B_SRC		-> mgr-bitop-src
; C_ACTIVE	-> mgr-charflag-active
; CS_BOX	-> mgr-cursorstyle-box
; E_ADDLINE	-> mgr-cmd-addline
; G_WINSIZE	-> mgr-getinfoflag-winsize
; M_STANDOUT	-> mgr-modeflag-standout
; MF_SNIP	-> mgr-menuflag-snip
; P_ALL		-> mgr-pushenvflag-all
; T_YANK	-> mgr-textflag-yank
; W_OVERSTRIKE	-> mgr-getmodeflag-overstrike
; 
;; You've got the whole Mgr macro/function set here, whether used or not!

;; text flags - for commands with text string arguments

(defvar mgr-textflag-invalid	0)	;; invalid command
(defvar mgr-textflag-font	1)	;; down load a new font
(defvar mgr-textflag-menu	2)	;; down load a menu
(defvar mgr-textflag-event	3)	;; down load an event string
(defvar mgr-textflag-yank	4)	;; fill the yank bufffer
(defvar mgr-textflag-bitmap	5)	;; down load a bit map
(defvar mgr-textflag-command	6)	;; start a new window & command in it
(defvar mgr-textflag-gimme	7)	;; send me stuff
(defvar mgr-textflag-smap	8)	;; save a bitmap on a file
(defvar mgr-textflag-gmap	9)	;; read a bitmap from a file
(defvar mgr-textflag-send	10)	;; send a message to another application
(defvar mgr-textflag-grunch	11)	;; fast-draw, short vector mode
(defvar mgr-textflag-string	12)	;; write a text sting into an offscreen bitmap

;; option codes for getmode / setmode

(defvar mgr-modeflag-standout	0)	;; window is in standout mode
(defvar mgr-modeflag-wob	1)	;; window is white on black
(defvar mgr-modeflag-autoexpose	2)	;; expose window upon shell output
(defvar mgr-modeflag-background	3)	;; permit obscured window to update
(defvar mgr-modeflag-noinput	4)	;; don't accept keyboard input
(defvar mgr-modeflag-nowrap	5)	;; don't auto wrap
(defvar mgr-modeflag-overstrike	6)	;; overstrike mode
(defvar mgr-modeflag-abs	7)	;; use absolute coordinates
(defvar mgr-modeflag-activate	8)	;; activate / hide window; not a mode
(defvar mgr-modeflag-stack	12)	;; permit event stacking
(defvar mgr-modeflag-dupkey	13)	;; set keyboard escape key
(defvar mgr-modeflag-nobuckey	14)	;; prevent mgr processing buckey keys, pass them through to the application

;; cut/paste options

(defvar mgr-modeflag-snarflines	9)	;; only snarf entire lines
(defvar mgr-modeflag-snarftabs	10)	;; change spaces to tabs in snarf
(defvar mgr-modeflag-snarfhard	11)	;; snarf even if errors

;; option codes for getinfo

(defvar mgr-getinfoflag-mouse	 	0)	;; mouse coordinates
(defvar mgr-getinfoflag-termcap 	1)	;; send back termcap entry
(defvar mgr-getinfoflag-winsize 	2)	;; cols, lines
(defvar mgr-getinfoflag-font	 	3)	;; font wide, high, #
(defvar mgr-getinfoflag-coords 		4)	;; window coords
(defvar mgr-getinfoflag-status 		5)	;; window status
(defvar mgr-getinfoflag-all	 	6)	;; complete window status
(defvar mgr-getinfoflag-system 		7)	;; system status
(defvar mgr-getinfoflag-allfont 	8)	;; font information
(defvar mgr-getinfoflag-text		9)	;; text region size
(defvar mgr-getinfoflag-allmine	10)	;; window status for client windows
(defvar mgr-getinfoflag-cursor	11)	;; character/ graphics cursor position
(defvar mgr-getinfoflag-mouse2	12)	;; cooked mouse coordinates
(defvar mgr-getinfoflag-notify	13)	;; gimme info re notify windows
(defvar mgr-getinfoflag-id	14)	;; my client window number
(defvar mgr-getinfoflag-flags	15)	;; current window flags
(defvar mgr-getinfoflag-max	15)	;; maximum getinfo value

;; bit positions set by mgr-getinfo-get-flags
;; some names from setmode flags, others from defs.h

(defvar mgr-getmodeflag-exposed 1)	; window is exposed
(defvar mgr-getmodeflag-snarfable 4)	; cut function possible on window
(defvar mgr-getmodeflag-wob 8)		; window is white on black
(defvar mgr-getmodeflag-standout 16)	; window is in standout mode
(defvar mgr-getmodeflag-died 32)	; window has died
(defvar mgr-getmodeflag-autoexpose 64)	; expose window upon shell output
(defvar mgr-getmodeflag-background 128) ; permit obscured window to update
(defvar mgr-getmodeflag-nokill 256)	; don't kill window upon exit
(defvar mgr-getmodeflag-vi 512)		; vi mouse support hack enabled
(defvar mgr-getmodeflag-noinput 2048)	; don't accept keyboard input
(defvar mgr-getmodeflag-nowrap 4096)	; no auto wrap
(defvar mgr-getmodeflag-overstrike (* 8 1024)) ; overstrike mode
(defvar mgr-getmodeflag-abs (* 16 1024)) ; use absolute coordinates
(defvar mgr-getmodeflag-snarflines (* 64 1024)) ; only snarf entire lines
(defvar mgr-getmodeflag-snarftabs (* 128 1024)) ; snarf changes spaces to tabs
(defvar mgr-getmodeflag-snarfhard (* 256 1024)) ; snarf even if errors
(defvar mgr-getmodeflag-inherit (* 512 1024)) ; inherit menus and bitmaps
(defvar mgr-getmodeflag-dupkey (* 1024 1024)) ; duplicate key mode
(defvar mgr-getmodeflag-nobuckey (* 2048 1024)) ; no buckey keys

;; option codes for stacking window environment

(defvar mgr-pushenvflag-menu	1)	;; push menus
(defvar mgr-pushenvflag-event	2)	;; push events
(defvar mgr-pushenvflag-font	4)	;; push current font
(defvar mgr-pushenvflag-cursor	8)	;; push current cursor position
(defvar mgr-pushenvflag-bitmap	16)	;; push saved bitmaps
(defvar mgr-pushenvflag-position	32)	;; push window location
(defvar mgr-pushenvflag-window	64)	;; push window contents
(defvar mgr-pushenvflag-flags	128)	;; push window flags
(defvar mgr-pushenvflag-mouse	256)	;; push mouse position
(defvar mgr-pushenvflag-text	512)	;; push text region


(defvar mgr-pushenvflag-all	1023)	;; push everything
(defvar mgr-pushenvflag-max	1024)	;; end of codes marker
(defvar mgr-pushenvflag-default
	(logior mgr-pushenvflag-menu
		mgr-pushenvflag-event
		mgr-pushenvflag-font
		mgr-pushenvflag-flags
		mgr-pushenvflag-text))

(defvar mgr-pushenvflag-clear	1024)	;; clear new environment

;; menu flags

(defvar mgr-menuflag-snip	8)	;; don't send action for parent of s/o menu
(defvar mgr-menuflag-page	4)	;; auto-page for menus
(defvar mgr-menuflag-auto	2)	;; auto-right exit for menus
(defvar mgr-menuflag-clear	1)	;; clear menu flags

;; escape codes

(defvar mgr-cmd-minus	?-)	;; set the munus flag
(defvar mgr-cmd-sep1	?,)	;; primary field seperator
(defvar mgr-cmd-sep2	?;)	;; secondary field seperator
(defvar mgr-cmd-mouse	??)	;; testing  -- move the mouse
(defvar mgr-cmd-addline	?a)	;; add a new line
(defvar mgr-cmd-addchar	?A)	;; add a character
(defvar mgr-cmd-bitblt	?b)	;; do a bit blit
(defvar mgr-cmd-bitcrt	?B)	;; create a bit blit
(defvar mgr-cmd-cleareol	?c)	;; clear
(defvar mgr-cmd-cleareos	?C)	;; clear
(defvar mgr-cmd-deleteline	?d)	;; delete a line
(defvar mgr-cmd-bitload	?D)	;; download a bitmap
(defvar mgr-cmd-event	?e)	;; download events
(defvar mgr-cmd-deletechar	?E)	;; delete a char
(defvar mgr-cmd-down	?f)	;; down 1 line
(defvar mgr-cmd-font	?F)	;; pick a new font
(defvar mgr-cmd-go	?g)	;; go; move graphics pointer
(defvar mgr-cmd-move	?G)	;; move to x,y pixels
(defvar mgr-cmd-setcursor	?h)	;; select cursor style
(defvar mgr-cmd-bleep	?H)	;; blink a section of the screen
(defvar mgr-cmd-getinfo	?I)	;; get info from mgr
(defvar mgr-cmd-standout	?i)	;; start standout mode
(defvar mgr-cmd-fcolor	?j)	;; set forground color
(defvar mgr-cmd-bcolor	?J)	;; set background color
(defvar mgr-cmd-line	?l)	;; plot a line
(defvar mgr-cmd-link	?L)	;; menu links
(defvar mgr-cmd-menu	?m)	;; download menus
(defvar mgr-cmd-cup	?M)	;; move to col, row (zero origin)
(defvar mgr-cmd-standend	?n)	;; end standout mode
(defvar mgr-cmd-circle	?o)	;; plot a circle or an ellipse or an arc
(defvar mgr-cmd-push	?P)	;; push window environment
(defvar mgr-cmd-pop	?p)	;; pop window environment
(defvar mgr-cmd-rubber	?R)	;; rubber band a line/rect (obsolete)
(defvar mgr-cmd-right	?r)	;; right 1 column
(defvar mgr-cmd-clearmode	?s)	;; clear window mode
(defvar mgr-cmd-setmode	?S)	;; set a window mode
(defvar mgr-cmd-textregion	?t)	;; set the text region
(defvar mgr-cmd-upline	?u)	;; up 1 line
(defvar mgr-cmd-bitget	?U)	;; upload a bitmap
(defvar mgr-cmd-shape	?W)	;; reshape window, make it active
(defvar mgr-cmd-size	?w)	;; reshape window: cols,rows
(defvar mgr-cmd-gimme	?x)	;; send me data
(defvar mgr-cmd-putsnarf	?y)	;; put the snarf buffer
(defvar mgr-cmd-snarf	?Y)	;; snarf text into the snarf buffer
(defvar mgr-cmd-vi	?V)	;; set vi mode
(defvar mgr-cmd-novi	?v)	;; turn off vi mode
(defvar mgr-cmd-halfwin	?z)	;; make a 1/2 window
(defvar mgr-cmd-makewin	?Z)	;; make/goto a new window
(defvar mgr-cmd-null	?$)	;; do nothing, force exit
(defvar mgr-cmd-smap	?>)	;; save a bitmap
(defvar mgr-cmd-gmap	?<)	;; get a bitmap
(defvar mgr-cmd-send	?|)	;; send a message to another application
(defvar mgr-cmd-cursor	?%)	;; set mouse cursor
(defvar mgr-cmd-grunch	?:)	;; graphics crunch mode (experimental)
(defvar mgr-cmd-string	?.)	;; write characters in offscreen bitmap

(defvar mgr-cmd-xmenu	?X)	;; extended menu operations



;; misc

(defvar mgr-charflag-nochar	??)	;; for character not in font
(defvar mgr-charflag-exposed	?e)	;; window is not obscured
(defvar mgr-charflag-active	?a)	;; window has input focus
(defvar mgr-charflag-notactive	?n)	;; window is obscured
(defvar mgr-charflag-obscured	?o)	;; window is obscured
(defvar mgr-charflag-name	"px|mgr|mgr teminal emulator")

(defvar mgr-charflag-null	?\0)	;; null
(defvar mgr-charflag-bs		?\b)	;; back space
(defvar mgr-charflag-ff		?\f)	;; form feed
(defvar mgr-charflag-bell	?\a)	;; bell
(defvar mgr-charflag-tab	?\t)	;; tab
(defvar mgr-charflag-return	?\r)	;; return
(defvar mgr-charflag-nl		?\n)	;; line feed

;; cursor styles
(defvar mgr-cursorstyle-block	0)	;; standard block cursor
(defvar mgr-cursorstyle-left	1)	;; left vertical bar
(defvar mgr-cursorstyle-right	2)	;; right vertical bar
(defvar mgr-cursorstyle-box	3)	;; outline
(defvar mgr-cursorstyle-under	4)	;; underline
(defvar mgr-cursorstyle-invis	9)	;; invisible

;; some raster op functions  (for bit_copy)

(defun mgr-bit-not (x) (lognot x))
(defvar mgr-bitop-src		12)
(defvar mgr-bitop-dst		10)
(defvar mgr-bitop-or		(logior mgr-bitop-src mgr-bitop-dst))
(defvar mgr-bitop-copy		mgr-bitop-src)
(defvar mgr-bitop-copyinverted	(logand (mgr-bit-not mgr-bitop-src) 15))
(defvar mgr-bitop-xor		(logxor mgr-bitop-src mgr-bitop-dst))
(defvar mgr-bitop-and		(logand mgr-bitop-src mgr-bitop-dst))

;; raster op functions  (for bit_write and bit_line)

(defvar mgr-bitop-set		15)
(defvar mgr-bitop-clear		0)
(defvar mgr-bitop-invert	(logand (mgr-bit-not mgr-bitop-dst) 15))


;; events

(defvar mgr-event-button-1	1)		;; end button depressed
(defvar mgr-event-button-2	2)		;; middle button depressed
(defvar mgr-event-button-1u	3)		;; end button released
(defvar mgr-event-button-2u	4)		;; middle button released
(defvar mgr-event-reshape	5)		;; window was reshaped
(defvar mgr-event-reshaped	5)		;; window was reshaped
(defvar mgr-event-redraw	6)		;; screen was redrawn
(defvar mgr-event-redrawn	6)		;; screen was redrawn
(defvar mgr-event-activate	7)		;; window was activated
(defvar mgr-event-activated	7)		;; window was activated
(defvar mgr-event-deactivate	8)		;; window was deactivated
(defvar mgr-event-deactivated	8)		;; window was deactivated
(defvar mgr-event-covered	9)		;; window was covered
(defvar mgr-event-uncovered	10)		;; window was uncovered
(defvar mgr-event-move		11)		;; window was moved
(defvar mgr-event-moved		11)		;; window was moved
(defvar mgr-event-destroy	12)		;; window was destroyed
(defvar mgr-event-accept	13)		;; accept messages
(defvar mgr-event-notify	14)		;; set notification
(defvar mgr-event-snarfed	16)		;; text was just snarfed
(defvar mgr-event-paste		17)		;; text was just pasted


(defun mgr-addline ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-addline)))

(defun mgr-addlines (n)
  (mgr-send (format "%c%d%c" mgr-esc n mgr-cmd-addline)))

(defun mgr-addchar ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-addchar)))

(defun mgr-addchars (n)
  (mgr-send (format "%c%d%c" mgr-esc n mgr-cmd-addchar)))

(defun mgr-deleteline ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-deleteline)))

(defun mgr-deletelines (n)
  (mgr-send (format "%c%d%c" mgr-esc n mgr-cmd-deleteline)))

(defun mgr-deletechar ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-deletechar)))

(defun mgr-deletechars (n)
  (mgr-send (format "%c%d%c" mgr-esc n mgr-cmd-deletechar)))

(defun mgr-standend ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-standend)))

(defun mgr-standout ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-standout)))

(defun mgr-bell ()
  (mgr-send (format "%c" mgr-charflag-bell)))

(defun mgr-setcursor (n)
  (mgr-send (format "%c%d%c" mgr-esc n mgr-cmd-setcursor)))

;; lines


(defun mgr-line (x0 y0 x1 y1)
  (mgr-send (format "%c%d;%d;%d;%d%c" mgr-esc x0 y0 x1 y1 mgr-cmd-line)))

(defun mgr-lineto (to x0 y0 x1 y1)
  (mgr-send (format "%c%d;%d;%d;%d;%d%c" mgr-esc x0 y0 x1 y1 to mgr-cmd-line)))

(defun mgr-draw (x y)
  (mgr-send (format "%c%d;%d%c"
		    mgr-esc (mgr-constrain x) (mgr-constrain y) mgr-cmd-line)))

; the fastdraw buff can be a string or list of numbers.
(defun mgr-fastdraw (x y count buff)
  (mgr-send (format "%c%d;%d;%d%c"
		    mgr-esc (mgr-constrain x) (mgr-constrain y)
		    count mgr-cmd-grunch))
  (mgr-send buff))

(defun mgr-rfastdraw (count buff)
  (mgr-send (format "%c%d%c"
		    mgr-esc count mgr-cmd-grunch))
  (mgr-send buff))


(defun mgr-aligntext ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-line)))

(defun mgr-gotext ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-go)))

(defun mgr-go (x y)
  (mgr-send (format "%c%d;%d%c"
		    mgr-esc (mgr-constrain x) (mgr-constrain y) mgr-cmd-go)))

;; bitblits

(defun mgr-clear ()
  (mgr-send (format "%c" mgr-charflag-ff)))

(defun mgr-func (func)
  (mgr-send (format "%c%d%c"
		    mgr-esc func mgr-cmd-bitblt)))

(defun mgr-bitwrite (x y w h)
  (mgr-send (format "%c%d;%d;%d;%d%c"
		    mgr-esc x y w h mgr-cmd-bitblt)))

(defun mgr-bitwriteto (x y w h to)
  (mgr-send (format "%c%d;%d;%d;%d;%d%c"
		    mgr-esc x y w h to mgr-cmd-bitblt)))

(defun mgr-bitcopy (xd yd w h xs ys)
  (mgr-send (format "%c%d;%d;%d;%d;%d;%d%c"
		    mgr-esc xd yd w h xs ys mgr-cmd-bitblt)))

(defun mgr-bitcopyto (xd yd w h xs ys to from)
  (mgr-send (format "%c%d;%d;%d;%d;%d;%d;%d;%d%c"
		    mgr-esc xd yd w h xs ys to from mgr-cmd-bitblt)))

(defun mgr-bitld (w h x y size)
  (mgr-send (format "%c%d;%d;%d;%d;%d%c"
		    mgr-esc w h x y size mgr-cmd-bitload)))

(defun mgr-bitldto (w h x y to size)
  (mgr-send (format "%c%d;%d;%d;%d;%d;%d%c"
		    mgr-esc w h x y to size mgr-cmd-bitload)))

(defun mgr-bitdestroy (n)
  (mgr-send (format "%c%d%c"
		    mgr-esc n mgr-cmd-bitcrt))
  (mgr-flush))

(defun mgr-bitcreate (n w h)
  (mgr-send (format "%c%d;%d;%d%c"
		    mgr-esc n w h mgr-cmd-bitcrt)))

(defun mgr-bitget (from size offset)
  (mgr-send (format "%c%d;%d;%d%c"
		    mgr-esc from size offset mgr-cmd-bitget))
  (mgr-flush))

(defun mgr-othersave (id sub name)
  (mgr-send (format "%c%d;%d;%d%c%s"
		    mgr-esc id sub (length name) mgr-cmd-smap name)))

(defun mgr-windowsave (name)
  (mgr-send (format "%c%d%c%s"
		    mgr-esc (length name) mgr-cmd-smap name)))

(defun mgr-bitsave (from name)
  (mgr-send (format "%c%d;%d%c%s"
		    mgr-esc from (length name) mgr-cmd-smap name)))

(defun mgr-bitfromfile (to name)
  (mgr-send (format "%c%d;%d%c%s"
		    mgr-esc to (length name) mgr-cmd-gmap name)))

(defun mgr-highlight (x y w h)
  (mgr-send (format "%c%d;%d;%d;%d%c"
		    mgr-esc x y w h mgr-cmd-bleep))
  (mgr-flush))

(defun mgr-stringto (to x y text)
  (mgr-send (format "%c%d;%d;%d;%d%c%s"
		    mgr-esc to x y (length text) mgr-cmd-string text)))

;; other graphic functions

(defun mgr-circle (x y r)
  (mgr-send (format "%c%d;%d;%d%c"
		    mgr-esc (mgr-constrain x) (mgr-constrain y)
		    (mgr-constrain r) mgr-cmd-circle)))

(defun mgr-ellipse (x y r1 r2)
  (mgr-send (format "%c%d;%d;%d;%d%c"
		    mgr-esc (mgr-constrain x) (mgr-constrain y)
		    (mgr-constrain r1) (mgr-constrain r2) mgr-cmd-circle)))

(defun mgr-arc (x y x1 y1 x2 y2)
  (mgr-send (format "%c%d;%d;%d;%d;%d;%d%c"
		    mgr-esc (mgr-constrain x) (mgr-constrain y)
		    (mgr-constrain x1) (mgr-constrain y1)
		    (mgr-constrain x2) (mgr-constrain y2)
		    mgr-cmd-circle)))

(defun mgr-ellipseto (to x y r1 r2)
  (mgr-send (format "%c%d;%d;%d;%d;%d%c"
		    mgr-esc (mgr-constrain x) (mgr-constrain y)
		    (mgr-constrain r1) (mgr-constrain r2)
		    to mgr-cmd-circle)))

(defun mgr-rcircle (r)
  (mgr-send (format "%c%d%c"
		    mgr-esc (mgr-constrain r) mgr-cmd-circle)))

(defun mgr-rellipse (r1 r2)
  (mgr-send (format "%c%d;%d%c"
		    mgr-esc (mgr-constrain r1) (mgr-constrain r2)
		    mgr-cmd-circle)))


(defun mgr-movemouse (x y)
  (mgr-send (format "%c%d;%d%c" mgr-esc x y mgr-cmd-mouse)))

(defun mgr-movecursor (x y)
  (mgr-send (format "%c%d;%d%c" mgr-esc x y mgr-cmd-move)))

(defun mgr-move (col row)
  (mgr-send (format "%c%d;%d%c" mgr-esc col row mgr-cmd-cup)))

(defun mgr-moveprint (x y str)
  (mgr-send (format "%c%d;%d%c%s" mgr-esc x y mgr-cmd-move str)))

(defun mgr-incr (x)
  (mgr-send (format "%c%d%c" mgr-esc x mgr-cmd-move)))

(defun mgr-cleareol ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-cleareol)))

(defun mgr-cleareos ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-cleareos)))

;; window manipulation

(defun mgr-movewindow (x y)
  (mgr-send (format "%c%d;%d%c" mgr-esc x y mgr-cmd-shape)))

(defun mgr-shapewindow (x y dx dy)
  (mgr-send (format "%c%d;%d;%d;%d%c" mgr-esc x y dx dy mgr-cmd-shape)))

(defun mgr-font (x)
  (mgr-send (format "%c%d%c" mgr-esc x mgr-cmd-font)))

(defun mgr-loadfont (n name)
  (mgr-send (format "%c%d;%d%c%s"
		    mgr-esc n (length name) mgr-cmd-font name)))

(defun mgr-size (cols rows)
  (mgr-send (format "%c%d;%d%c" mgr-esc cols rows mgr-cmd-size)))

(defun mgr-sizeall (x y cols rows)
  (mgr-send (format "%c%d;%d;%d;%d%c"
		    mgr-esc x y cols rows mgr-cmd-size)))

(defun mgr-scrollregion (first last)
  (mgr-send (format "%c%d;%d%c"
		    mgr-esc first last mgr-cmd-textregion)))

(defun mgr-textregion (x y wide high)
  (mgr-send (format "%c%d;%d;%d;%d%c"
		    mgr-esc x y wide high mgr-cmd-textregion)))

(defun mgr-textreset ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-textregion)))

;; window creation/ destruction

(defun mgr-newwin (x y w h)
  (mgr-send (format "%c%d;%d;%d;%d%c" mgr-esc x y w h mgr-cmd-makewin)))

(defun mgr-halfwin (x y w h)
  (mgr-send (format "%c%d;%d;%d;%d%c" mgr-esc x y w h mgr-cmd-halfwin)))

(defun mgr-halfwinft (x y w h ft)
  (mgr-send (format "%c%d;%d;%d;%d;%d%c" mgr-esc x y w h ft mgr-cmd-halfwin)))

(defun mgr-destroywin (n)
  (mgr-send (format "%c%d;0%c" mgr-esc n mgr-cmd-makewin)))

(defun mgr-selectwin (n)
  (mgr-send (format "%c%d%c" mgr-esc n mgr-cmd-makewin)))

;; events

(defun mgr-map-event (z)
  (if (or (eq z 3) (eq z 4))
      (- 2 z)
    z))

(defun mgr-setevent (event x)
  (mgr-send (format "%c%d;%d%c%s"
		    mgr-esc (mgr-map-event event) (length x) mgr-cmd-event x)))

(defun mgr-clearevent (event)
  (mgr-send (format "%c%d%c"
		    mgr-esc (mgr-map-event event) mgr-cmd-event)))

;; message passing

(defun mgr-sendme (str)
  (mgr-send (format "%c%d%c%s"
		    mgr-esc (length str) mgr-cmd-gimme str)))

(defun mgr-sendto (pid str)
  (mgr-send (format "%c%d;%d%c%s"
		    mgr-esc pid (length str) mgr-cmd-send str)))

(defun mgr-broadcast (str)
  (mgr-send (format "%c%d%c%s"
		    mgr-esc (length str) mgr-cmd-send str)))

(defun mgr-snarf (str)
  (mgr-send (format "%c%d%c%s"
		    mgr-esc (length str) mgr-cmd-snarf str)))

(defun mgr-put ()
  (mgr-send (format "%c%c"
		    mgr-esc mgr-cmd-putsnarf)))

;; environment stacking

(defun mgr-push (mode)
  (setq mgr-env-count (1+ mgr-env-count))
  (mgr-send (format "%c%d%c"
		    mgr-esc (logior mode mgr-pushenvflag-clear) mgr-cmd-push)))

(defun mgr-pushsave (mode)
  (setq mgr-env-count (1+ mgr-env-count))
  (mgr-send (format "%c%d%c" mgr-esc mode mgr-cmd-push)))

(defun mgr-pop ()
  (if (> mgr-env-count 0)
      (setq mgr-env-count (1- mgr-env-count)))
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-pop)))

(defun mgr-popall ()
  (while (> mgr-env-count 0)
    (setq mgr-env-count (1- mgr-env-count))
    (mgr-send (format "%c%c" mgr-esc mgr-cmd-pop))))

;; other stuff

(defun mgr-setmode (mode)
  (mgr-send (format "%c%d%c" mgr-esc mode mgr-cmd-setmode)))

(defun mgr-dupkey (key)
  (mgr-send (format "%c%d;%d%c"
		    mgr-esc mgr-modeflag-dupkey key mgr-cmd-setmode)))

(defun mgr-clearmode (mode)
  (mgr-send (format "%c%d%c" mgr-esc mode mgr-cmd-clearmode)))

(defun mgr-getinfo (x)
  (mgr-send (format "%c%d%c" mgr-esc x mgr-cmd-getinfo))
  (mgr-flush))

(defun mgr-whatsat (x y)
  (mgr-send (format "%c%d;%d%c" mgr-esc x y mgr-cmd-getinfo))
  (mgr-flush))

(defun mgr-sleep ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-null)))

;; menu stuff

(defun mgr-selectmenu (n)
  (mgr-send (format "%c%d%c" mgr-esc n mgr-cmd-menu)))

(defun mgr-selectmenu2 (n)
  (mgr-send (format "%c-%d%c" mgr-esc n mgr-cmd-menu)))

(defun mgr-nomenu ()
  (mgr-send (format "%c%d%c" mgr-esc 999 mgr-cmd-menu)))

(defun mgr-nomenu2 ()
  (mgr-send (format "%c-%d%c" mgr-esc 999 mgr-cmd-menu)))

(defun mgr-loadmenu (n str)
  (mgr-send (format "%c%d;%d%c%s" mgr-esc n (length str) mgr-cmd-menu str)))

(defun mgr-clearmenu (n)
  (mgr-send (format "%c%d;0%c" mgr-esc n mgr-cmd-menu)))

(defun mgr-linkmenu (parent item child flags)
  (mgr-send (format "%c%d;%d;%d;%d%c"
		    mgr-esc parent item child flags mgr-cmd-menu)))

(defun mgr-unlinkmenu (parent item)
  (mgr-send (format "%c%d;%d;%d;%c"
		    mgr-esc parent item -1 mgr-cmd-menu)))

(defun mgr-pagemenu (parent child)
  (mgr-send (format "%c%d;%d;%d;%c"
		    mgr-esc parent -1 child mgr-cmd-menu)))

(defun mgr-unpagemenu (parent)
  (mgr-send (format "%c%d;%d;%d;%c"
		    mgr-esc parent -1 -1 mgr-cmd-menu)))

;; temporary menu stuff

(defun mgr-menuitem (menu item)
  (mgr-send (format "%c%d;%d%c" mgr-esc menu item mgr-cmd-xmenu)))

(defun mgr-menuerase (menu)
  (mgr-send (format "%c%d%c" mgr-esc menu mgr-cmd-xmenu)))

(defun mgr-menushow (x y menu)
  (mgr-send (format "%c%d;%d;%d%c" mgr-esc x y menu mgr-cmd-xmenu)))

(defun mgr-menubar (x y menu item)
  (mgr-send (format "%c%d;%d;%d;%d%c" mgr-esc x y menu item mgr-cmd-xmenu)))

;; temporary relative character motion

(defun mgr-right (tenths)
  (mgr-send (format "%c%d;%d%c" mgr-esc tenths 10 mgr-cmd-right)))

(defun mgr-left (tenths)
  (mgr-send (format "%c%d;%d%c" mgr-esc tenths -10 mgr-cmd-right)))

(defun mgr-up (tenths)
  (mgr-send (format "%c%d;%d%c" mgr-esc tenths 10 mgr-cmd-upline)))

(defun mgr-down (tenths)
  (mgr-send (format "%c%d;%d%c" mgr-esc tenths 10 mgr-cmd-down)))

;; color stuff

(defun mgr-fcolor (color)
  (mgr-send (format "%c%d%c" mgr-esc color mgr-cmd-fcolor)))
(defun mgr-bcolor (color)
  (mgr-send (format "%c%d%c" mgr-esc color mgr-cmd-bcolor)))
(defun mgr-linecolor (op color)
  (mgr-send (format "%c%d;%d%c" mgr-esc op color mgr-cmd-bitblt)))

;; not in C header, feature called "temporary"

(defun mgr-set-vi-mode ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-vi)))
(defun mgr-unset-vi-mode ()
  (mgr-send (format "%c%c" mgr-esc mgr-cmd-novi)))


;;; 
;;; Mgr support code for GNU Emacs
;;; Author: Vincent Broman, broman@nosc.mil, August 1993.
;;; 

;;; Helper functions

(defun mgr-parse-string (str)
  "Return a list of strings and numbers appearing in STR separated by blanks."
  (mgr-parse-string-rec (or str "") nil))

(defun mgr-parse-string-rec (str ls)	; helper for mgr-parse-string
  (let ((mat (string-match "\\([^ ]+\\) *\\'" str)))
    (if (null mat)
	ls
      (let ((elt (substring str (match-beginning 1) (match-end 1))))
	(mgr-parse-string-rec (substring str 0 (match-beginning 0))
			      (cons (if (string-match "\\`[0-9+-]+\\'" elt)
					(string-to-int elt)
				      elt)
				    ls))))))

(defun mgr-window-is-active ()
  "Predicate indicates whether the current Mgr window is active."
  (string-equal "a" (or (mgr-getinfo-get-status) "unk")))

(defun mgr-bitfile (to name)
  "Load into bitmap TO the bitmap in the file NAME.
Return '(pixels-wide pixels-high), or nil if NAME not found."
  (mgr-bitfromfile to name)
  (mgr-parse-string (mgr-gets)))

(defun mgr-makewindow (x y dwide dhigh)
  "Create an new alternate window at (X, Y) with dimensions dwide x dhigh.
Return the ID of the new window, or nil in case of failure.
The actual dimensions may be truncated to the screen bottom/right edges."
  (mgr-newwin x y dwide dhigh)
  (mgr-parse-string (mgr-gets)))

(defun mgr-make-separate-window (x y dwide dhigh)
  "Create a new window with half a pty at (X, Y) with dimensions dwide x dhigh.
Return the device name of the new window pty, or nil in case of failure.
The actual dimensions may be truncated to the screen bottom/right edges."
  (mgr-halfwin x y dwide dhigh)
  (let ((ret (mgr-parse-string (mgr-gets))))
    (if (consp ret) (car ret) nil)))

(defun mgr-make-separate-window-font (x y dwide dhigh ft)
  "Create a new window with half a pty at (X, Y) with dimensions dwide x dhigh.
The initial font number to be used in the window is ft.
Return the device name of the new window pty, or nil in case of failure.
The actual dimensions may be truncated to the screen bottom/right edges."
  (mgr-halfwinft x y dwide dhigh ft)
  (let ((ret (mgr-parse-string (mgr-gets))))
    (if (consp ret) (car ret) nil)))

(defun mgr-getinfo-readlines (prevls)
;; Read lines from Mgr till the first empty line, return parsed.
  (let ((ln (mgr-gets)))
    (if (zerop (length ln))
	(nreverse prevls)
      (mgr-getinfo-readlines (cons (mgr-parse-string ln) prevls)))))

(defun mgr-getinfo-get-all ()
  "Return a list of descriptions of each window on the display, front-to-back,
each description being a list of strings or numbers in display coordinates:
'(leftmost-x topmost-y width height pty-name-suffix id status set-id).
The status is \"e\" for exposed, \"o\" for obscured, or else
\"E\" or \"O\" respectively if the window involved is the current window."
  (mgr-getinfo mgr-getinfoflag-all)
  (mgr-getinfo-readlines nil))

(defun mgr-getinfo-get-allmine ()
  "Return a list of descriptions of each of my windows, front-to-back,
each description being a list of strings or numbers in display coordinates:
'(leftmost-x topmost-y width height pty-name-suffix id status set-id).
The status is \"e\" for exposed, \"o\" for obscured, or else
\"E\" or \"O\" respectively if the window involved is the current window."
  (mgr-getinfo mgr-getinfoflag-allmine)
  (mgr-getinfo-readlines nil))

(defun mgr-getinfo-get-coords ()
  "Return the position and size of the current window in display coordinates,
'(leftside topside width height)"
  (mgr-getinfo mgr-getinfoflag-coords)
  (mgr-parse-string (mgr-gets)))

(defun mgr-getinfo-get-cursor ()
  "Return the current location of the character cursor in character
coordinates and the graphics cursor in window coordinates as
'(col row x y)"
  (mgr-getinfo mgr-getinfoflag-cursor)
  (mgr-parse-string (mgr-gets)))

(defun mgr-getinfo-get-font ()
  "Return information on the current font as
'(width-pixels height-pixels font-number font-name)"
  (mgr-getinfo mgr-getinfoflag-font)
  (mgr-parse-string (mgr-gets)))

(defun mgr-getinfo-get-id ()
  "Return the alternate window id number of the current window (zero if main)
and the number of alternate/main windows controlled by this program:
'(win-id win-count)"
  (mgr-getinfo mgr-getinfoflag-id)
  (mgr-parse-string (mgr-gets)))

(defun mgr-getinfo-get-mouse-display ()
  "Return a list containing the position of the mouse in display coordinates
and the last button transition, +1, -1, +2, or -2,
where + means pressed and - means released.
'(mouse-x mouse-y last-button)"
  (mgr-getinfo mgr-getinfoflag-mouse)
  (mgr-parse-string (mgr-gets)))

(defun mgr-getinfo-get-mouse-window ()
  "Return a list containing the position of the mouse in window coordinates
and the last button transition, +1, -1, +2, or -2,
where + means pressed and - means released.
'(mouse-x mouse-y last-button)"
  (mgr-getinfo mgr-getinfoflag-mouse2)
  (mgr-parse-string (mgr-gets)))

(defun mgr-getinfo-get-status ()
  "Return a one-char string indicating the status of the current window.
\"a\" means active, \"e\" exposed but inactive, \"o\" obscured."
  (mgr-getinfo mgr-getinfoflag-status)
  (mgr-gets))

(defun mgr-getinfo-get-system ()
  "Return a list of global/system information, viz.
'(mgr-hostname display-width-pixels display-height-pixels window-border-pixels)
This function ought to reveal something about color capabilities, too."
  (mgr-getinfo mgr-getinfoflag-system)
  (mgr-parse-string (mgr-gets)))

(defun mgr-getinfo-get-termcap ()
  "Return a string suitable for a TERMCAP description of this window.
This string is more suitable for vi than emacs."
  (mgr-getinfo mgr-getinfoflag-termcap)
  (or (mgr-gets) (mgr-charflag-name ":tc=unknown:")))

(defun mgr-getinfo-get-text ()
  "Return a list containing the position and size of the text region
in window coordinates, or nil if the region is the entire window.
'(left-edge top-edge width height)"
  (mgr-getinfo mgr-getinfoflag-text)
  (mgr-parse-string (mgr-gets)))

(defun mgr-getinfo-get-winsize ()
  "Return the number of columns and rows in the current
text region, or in the entire window if no text region is defined.
'(ncolumns nrows)"
  (mgr-getinfo mgr-getinfoflag-winsize)
  (mgr-parse-string (mgr-gets)))

(defun hex-digit-value (ch)
;; Return the hexadecimal numerical value of the hex digit char CH.
  (if (and (<= ?0 ch) (<= ch ?9))
      (- ch ?0)
    (+ 10 (- ch ?a))))

(defun hexstring-to-int (str)
  "Return a nonnegative number for the hexadecimal string STR,
or nil in case of error."
  (let ((ma (string-match "\\` *\\(0x\\)?\\([0-9a-f]+\\) *\\'"
			  (downcase str))))
    (if (null ma)
	nil
      (let ((len (- (match-end 2) (match-beginning 2)))
	    (num (substring str (match-beginning 2) (match-end 2)))
	    (val 0)
	    (ind 0))
	(while (< ind len)
	  (setq val (+ (* 16 val) (hex-digit-value (elt num ind))))
	  (setq ind (1+ ind)))
	val))))

(defun mgr-getinfo-get-flags ()
  "Return a list of one, or on some systems four, numerical mode flag(s)
representing the state of the current window.
The first number is the bitwise-OR of flag values given
mnemonic names of mgr-getmodeflag-* in this source code.
'(mode-flag-bits character-style background-color raster-op-function)"
  (mgr-getinfo mgr-getinfoflag-flags)
  (let ((ls (mgr-parse-string (concat "0x"
				      (or (mgr-gets) "0")))))
    (cons (hexstring-to-int (car ls))
	  (cdr ls))))

(defun mgr-getinfo-get-notify ()
  "Return a list of lists, one for each window which has a notify string set,
the decription being in this form:
'(window-shell-process-id window-id-number notify-string)"
  (mgr-getinfo mgr-getinfoflag-notify)
  (let ((reply (mgr-gets))
	(ls nil))
    (while (string-match "\\`\\([0-9]+\\)\\.\\([0-9]+\\) +\\([0-9]+\\) " reply)
      (let ((pid (string-to-int
		  (substring reply (match-beginning 1) (match-end 1))))
	    (nbr (string-to-int
		  (substring reply (match-beginning 2) (match-end 2))))
	    (len (string-to-int
		  (substring reply (match-beginning 3) (match-end 3))))
	    (prolog-end (match-end 0)))
	(let ((notify (substring reply prolog-end)))
	  (while (< (length notify) len)
	    ;; there could be a NL in the notify string
	    (setq notify (concat notify "\n" (mgr-gets))))
	  (setq ls (cons (list pid nbr notify) ls)))))
    (nreverse ls)))


(defun mgr-emacs-report-screen-size ()
  "Inform Emacs of the current screen (Mgr window) size, if necessary."
  (interactive)
  (let ((colrow (mgr-getinfo-get-winsize)))
    (if colrow
	(let ((ncol (car colrow))
	      (nrow (car (cdr colrow))))
	  (if (/= nrow (screen-height))
	      (set-screen-height nrow))
	  (if (/= ncol (screen-width))
	      (set-screen-width ncol)))
      (error "Could not read window size from Mgr"))))


;;; inquire the size of the display
(let ((s (mgr-getinfo-get-system)))
  (if (>= (length s) 4)
      (setq mgr-pixel-limit (max (nth 1 s) (nth 2 s)))))

;;; check whether vi mode is on
(setq mgr-vi-mode-on (/= 0 (logand mgr-getmodeflag-vi
				   (car (mgr-getinfo-get-flags)))))

;;; make sure emacs knows the right screen (window) size
(mgr-textreset)
(mgr-emacs-report-screen-size)


;; 
;; the preceding was generic, the rest is customization.
;; 


;; minibuffer-prompt-length and part of mgr-mouse-move-to
;; written by lnz@lucid.com for the Apollo.

(defun minibuffer-prompt-length ()
  "Returns the length of the current minibuffer prompt."
  (let ((window (selected-window)))
    (select-window (minibuffer-window))
    (let ((point (point)))
      (goto-char (point-min))
      (insert-char ?a 200)
      (goto-char (point-min))
      (vertical-motion 1)
      (let ((length (- (screen-width) (point))))
	(goto-char (point-min))
	(delete-char 200)
	(goto-char point)
	(select-window window)
	length))))

(defun mgr-mouse-move-to (x y)
  "Determine the window containing the screen col,row of X,Y.
Select that window and its buffer for editing.
Move point to be at that screen location, or as close as possible."
  (let ((edges (window-edges))
	(window nil))
    (while (and (not (eq window (selected-window)))
		(or (<  y (nth 1 edges))
		    (>= y (nth 3 edges))
		    (<  x (nth 0 edges))
		    (>= x (nth 2 edges))))
      (setq window (next-window window))
      (setq edges (window-edges window)))
    (if (and window
	     (not (eq window (selected-window)))
	     (eq window (minibuffer-window)))
	(error "Cannot use mouse to enter minibuffer!"))
    (if window (select-window window))
    (move-to-window-line (- y (nth 1 edges)))
    (let* ((width-1 (1- (window-width window)))
	   (wraps (/ (current-column) width-1))
	   (prompt-length (if (eq (selected-window) (minibuffer-window))
			      (minibuffer-prompt-length)
			    0)))
      (move-to-column (+ (- x (nth 0 edges) prompt-length)
			 (* wraps width-1))))))

(defun mgr-mouse-move-point ()
  "Handle the event of a mouse button press (down-transition).
The mouse location is read, the window containing this location determined,
and the window and buffer there are selected for editing.
Point is moved to be at that screen location, or as close as possible."
  (interactive)
  (let ((sx-sy (mgr-parse-string (or (mgr-gets) "0 0"))))
    (if mgr-vi-mode-on
	(while (/= ?| (mgr-getchar))
	  nil))				; discard the vi cmd
    (mgr-mouse-move-to (elt sx-sy 0) (elt sx-sy 1))))

(defun mgr-mouse-move-mark ()
  "Handle the event of a mouse button release (up-transition).
The mouse location is read, the window containing this location determined,
and the window and buffer there are selected for editing.
Point is moved to be at that screen location, or as close as possible.
If this mouse response has moved point but not switched to a different buffer,
then a mark is pushed at the old location of point.
Thus, a dragging motion sets the region."
  (interactive)
  (let ((oldpt (point))
	(oldbf (current-buffer))
	(sx-sy (mgr-parse-string (or (mgr-gets) "0 0"))))
    (mgr-mouse-move-to (elt sx-sy 0) (elt sx-sy 1))
    (if (and (/= (point) oldpt)
	     (equal (current-buffer) oldbf))
	(push-mark oldpt 'nomessage))))

(defun extract-file-name-around (x y)
;; Return the string filename found on screen at col X, row Y.
  (let ((filename-characters "-!#$%*+,./0-9=?@A-Z[\\]^_a-z{}~")
	(skip-at-end '(?/ ?* ?@ ?= ?. ?,)))
    (save-excursion
      (mgr-mouse-move-to x y)
      (skip-chars-backward filename-characters)
      (let ((start (point)))
	(skip-chars-forward filename-characters)
	(let* ((filename (buffer-substring start (point)))
	       (last-char (aref filename (- (length filename) 1))))
	  (if (memq last-char skip-at-end)
	      (substring filename 0 -1)
	    filename))))))

(defun mgr-get-mouse-pos ()
;; Return a cons cell containing the character coordinates (x y) of the
;; current mouse position relative to the text region of the current window.
;; The mouse might not be located inside the text region.
  (mgr-pushsave mgr-pushenvflag-flags)
  (mgr-setmode mgr-modeflag-abs)
  (let ((tr (or (mgr-getinfo-get-text) '(0 0)))
	(ms (mgr-getinfo-get-mouse-window))
	(fo (mgr-getinfo-get-font)))
    (let ((x (/ (- (elt ms 0) (elt tr 0)) (elt fo 0)))
	  (y (/ (- (elt ms 1) (elt tr 1)) (elt fo 1))))
      (mgr-pop)
      (cons x y))))

(defun find-file-named-here ()
  "Open, or find, the file named onscreen where the mouse is now pointing."
  (interactive)
  (let* ((ms (mgr-get-mouse-pos))
	 (name (extract-file-name-around (car ms) (cdr ms))))
    (find-file name)))

(defun mgr-entire-window-was-reshaped ()
  "Inform Emacs of screen (Mgr window) size, and refresh screen.
This function is called whenever Mgr reshapes the screen."
  (interactive)
  (mgr-emacs-report-screen-size)
  (save-excursion			; just to refresh without moving
    (move-to-window-line nil)
    (recenter nil)))

(defvar mgr-menu-key-assoclist nil
  "An assoclist for menu entries looking like:
    '(action-function . key-sequence)")
(defvar mgr-menu-key-start "\C-^M")
(defvar mgr-menu-key-nextchar ?\ )

(defun mgr-function-lookup-key (fn)
  "Look up the command FN and return the key sequence associated with it.
If none, then assign one, store it, bind it globally, and return it."
  (let ((fnassoc (assoc fn mgr-menu-key-assoclist)))
    (if fnassoc
	(cdr fnassoc)
      (let ((str (concat mgr-menu-key-start
			 (char-to-string mgr-menu-key-nextchar))))
	;; bump mgr-menu-key-nextchar, avoiding backslash which is magic.
	;; may have trouble with a huge number of distinct commands run
	;; from menus, if we overflow 7 or 8 bits.
	(setq mgr-menu-key-nextchar (1+ mgr-menu-key-nextchar))
	(if (= ?\\ mgr-menu-key-nextchar)
	    (setq mgr-menu-key-nextchar (1+ mgr-menu-key-nextchar)))
	(setq mgr-menu-key-assoclist (cons (cons fn str)
					   mgr-menu-key-assoclist))
	(global-set-key str fn)
	str))))

(defun mgr-compose-menu-actions (assoclist-elt)
  (let ((action (cdr assoclist-elt)))
    (if action
	(mgr-function-lookup-key action)
      "")))

(defun mgr-compose-menu-string (assoclist delim)
  "Return an Mgr menu string, given an assoclist of menu selections/commands
and the delimiter character to separate items."
  (let ((stringdelim (char-to-string delim)))
    (concat
     stringdelim
     (mapconcat 'car assoclist stringdelim)
     stringdelim
     (mapconcat 'mgr-compose-menu-actions assoclist stringdelim)
     stringdelim)))

(defvar mgr-last-menu-nbr -1
  "Mgr menu numbers are allocated starting at zero by mgr-allocate-menu-nbr.")
(defun mgr-allocate-menu-nbr ()
  (setq mgr-last-menu-nbr (1+ mgr-last-menu-nbr))
  mgr-last-menu-nbr)

(defvar mgr-emacs-top-menu (mgr-allocate-menu-nbr))
(defvar mgr-emacs-move-menu (mgr-allocate-menu-nbr))
(defvar mgr-emacs-edit-menu (mgr-allocate-menu-nbr))
(defvar mgr-emacs-files-menu (mgr-allocate-menu-nbr))
(defvar mgr-emacs-buffers-menu (mgr-allocate-menu-nbr))
(defvar mgr-emacs-windows-menu (mgr-allocate-menu-nbr))
(defvar mgr-emacs-modes-menu (mgr-allocate-menu-nbr))
(defvar mgr-emacs-programs-menu (mgr-allocate-menu-nbr))
(defvar mgr-emacs-help-menu (mgr-allocate-menu-nbr))

(setq kill-emacs-hook 'mgr-popall)
(mgr-pushsave mgr-pushenvflag-menu)


(mgr-loadmenu
 mgr-emacs-edit-menu
 (mgr-compose-menu-string
  '(
    ("Copy           " . copy-region-as-kill)
    ("Cut            " . kill-region)
    ("Paste          " . yank)
    ("---------------" . nil)
    ("Cut   Rectangle" . kill-rectangle)
    ("Paste Rectangle" . yank-rectangle)
    ("Open  Rectangle" . open-rectangle)
    ("Clear Rectangle" . clear-rectangle))
  mgr-delim))

(mgr-loadmenu
 mgr-emacs-move-menu
 (mgr-compose-menu-string
  '(
    ("Top of file      " . beginning-of-buffer)
    ("Prev Page        " . backward-page)
    ("Prev Screen      " . scroll-down)
    ("Prev Paragraph   " . backward-paragraph)
    ("Start of Sentence" . backward-sentence)
    ("Front of Line    " . back-to-indentation)
    ("-----move-to-----" . nil)
    ("End of Line      " . end-of-line)
    ("End of Sentence  " . forward-sentence)
    ("Next Paragraph   " . forward-paragraph)
    ("Next Screen      " . scroll-up)
    ("Next Page        " . forward-page)
    ("End of file      " . end-of-buffer))
  mgr-delim))

(mgr-loadmenu
 mgr-emacs-files-menu
 (mgr-compose-menu-string
  '(
    ("-----files-----" . nil)
    ("Open This One  " . find-file-named-here)
    ("Open...        " . find-file)
    ("Save           " . save-buffer)
    ("Save As...     " . write-file)
    ("Insert...      " . insert-file)
    ("Write Region..." . write-region)
    ("---------------" . nil)
    ("Dir List...    " . list-directory)
    ("Dir Edit...    " . dired))
  mgr-delim))

(mgr-loadmenu
 mgr-emacs-buffers-menu
 (mgr-compose-menu-string
  '(
    ("---buffers----" . nil)
    ("Switch to...  " . switch-to-buffer)
    ("Insert from..." . insert-buffer)
    ("Rename...     " . rename-buffer)
    ("Save          " . save-buffer)
    ("Save As...    " . write-file)
    ("Kill...       " . kill-buffer)
    ("--------------" . nil)
    ("Buffer List   " . list-buffers)
    ("Buffer Menu   " . buffer-menu))
  mgr-delim))

(mgr-loadmenu
 mgr-emacs-windows-menu
 (mgr-compose-menu-string
  '(
    ("-----windows--------" . nil)
    ("Goto Next Window    " . other-window)
    ("Split Vertically    " . split-window-vertically)
    ("Split Horizontally  " . split-window-horizontally)
    ("Enlarge Vertically  " . enlarge-window)
    ("Enlarge Horizontally" . enlarge-window-horizontally)
    ("Delete This Window  " . delete-window)
    ("Delete Others       " . delete-other-windows))
  mgr-delim))

(mgr-loadmenu
 mgr-emacs-modes-menu
 (mgr-compose-menu-string
  '(
    ("--minor--toggling----" . nil)
    ("Overwrite/Insert     " . overwrite-mode)
    ("Auto Fill            " . auto-fill-mode)
    ("Auto Save            " . auto-save-mode)
    ("------Major----------" . nil)
    ("Text                 " . text-mode)
    ("Indented Text        " . indented-text-mode)
    ("TeX                  " . tex-mode)
    ("LaTeX                " . latex-mode)
    ("Emacs Lisp           " . emacs-lisp-mode)
    ("Scheme               " . scheme-mode)
    ("Ada                  " . ada-mode)
    ("C                    " . c-mode)
    ("Pascal               " . pascal-mode)
    ("Fundamental (Vanilla)" . fundamental-mode))
  mgr-delim))

(mgr-loadmenu
 mgr-emacs-programs-menu
 (mgr-compose-menu-string
  '(
    ("--run program---" . nil)
    ("Shell Command..." . shell-command)
    ("Shell in Buffer " . cmushell)
    ("Grep...         " . grep)
    ("Compile...      " . compile)
    ("Goto Next Hit   " . next-error)
    ("Unix Man...     " . manual-entry)
    ("RMail           " . rmail)
    ("Send Mail       " . mail)
    ("UseNet News     " . gnus))
  mgr-delim))

(mgr-loadmenu
 mgr-emacs-help-menu
 (mgr-compose-menu-string
  '(
    ("--------help---------" . nil)
    ("Tutorial             " . help-with-tutorial)
    ("Info HyperManual     " . info)
    ("Apropos...           " . command-apropos)
    ("Key -> Function...   " . describe-key)
    ("Function -> Key...   " . where-is)
    ("Describe Function... " . describe-function)
    ("Describe Key...      " . describe-key)
    ("Describe Current Mode" . describe-mode))
  mgr-delim))

(mgr-loadmenu
 mgr-emacs-top-menu
 (mgr-compose-menu-string
  '(
    ("Search...      " . isearch-forward-regexp)
    ("Replace...     " . query-replace-regexp)
    ("Undo           " . advertised-undo)
    ("Move to      =>" . nil)
    ("Edit         =>" . nil)
    ("Files        =>" . nil)
    ("Buffers      =>" . nil)
    ("Windows      =>" . nil)
    ("Modes        =>" . nil)
    ("Run Programs =>" . nil)
    ("---------------" . nil)
    ("Help         =>" . nil)
    ("Exit Emacs     " . save-buffers-kill-emacs))
  mgr-delim))

(mgr-linkmenu mgr-emacs-top-menu  3 mgr-emacs-move-menu     mgr-menuflag-auto)
(mgr-linkmenu mgr-emacs-top-menu  4 mgr-emacs-edit-menu     mgr-menuflag-auto)
(mgr-linkmenu mgr-emacs-top-menu  5 mgr-emacs-files-menu    mgr-menuflag-auto)
(mgr-linkmenu mgr-emacs-top-menu  6 mgr-emacs-buffers-menu  mgr-menuflag-auto)
(mgr-linkmenu mgr-emacs-top-menu  7 mgr-emacs-windows-menu  mgr-menuflag-auto)
(mgr-linkmenu mgr-emacs-top-menu  8 mgr-emacs-modes-menu    mgr-menuflag-auto)
(mgr-linkmenu mgr-emacs-top-menu  9 mgr-emacs-programs-menu mgr-menuflag-auto)
(mgr-linkmenu mgr-emacs-top-menu 11 mgr-emacs-help-menu     mgr-menuflag-auto)
(mgr-selectmenu mgr-emacs-top-menu)	; middle mouse button

(mgr-setevent
 mgr-event-button-1
 (concat (mgr-function-lookup-key 'mgr-mouse-move-point) "%P\n"))
(mgr-setevent
 mgr-event-button-1u
 (concat (mgr-function-lookup-key 'mgr-mouse-move-mark) "%P\n"))
(mgr-setevent
 mgr-event-reshape
 (mgr-function-lookup-key 'mgr-entire-window-was-reshaped))
;(global-set-key "\C-L" 'mgr-entire-window-was-reshaped)	; instead of event...

(mgr-setevent mgr-event-notify "Gnu Emacs")

(mgr-flush)

(provide 'mgr)
