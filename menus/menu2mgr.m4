dnl {{{}}}
dnl {{{  Notes
dnl  The following m4 macros are used to create menues:
dnl
dnl  entry([text],[action])   defines entries ...
dnl  menu(number,font)        ... which are put into a menu
dnl  link(parent,child,entry)
dnl }}}
dnl {{{  m4 macros
changequote([,])dnl
define(__string__,[])dnl
define(__action__,[])dnl
define(menu,[[]$1;$2;len(__string__()[]__action__()	)m[]__string__()[]__action__()	[]define([__string__],)define([__action__],)[]dnl])dnl
define(entry,[define([__string__],__string__()[[	$1]])define([__action__],__action__()[[	$2]])[]dnl])dnl
define(activate,[$1m[]dnl])dnl
define(link,[$1;$2;$3;0m[]dnl])[]dnl
dnl }}}
dnl {{{  1     -- main
entry([MGR Origami         ])
entry([Motion            ->])
entry([Folding           ->])
entry([Editing           ->])
entry([Search & Replace  ->])
entry([Files             ->])
entry([Modes             ->])
entry([Quit              ->])
menu(1,7)
dnl }}}
dnl {{{  2     -- motion
entry([beginning-of-fold  M-<],[<])
entry([end-of-fold        M->],[>])
entry([previous-page      M-v],[v])
entry([next-page          C-V],[v])
entry([goto-line          M-g],[g])
menu(2,7)
dnl }}}
dnl {{{  3     -- folding
entry([open-fold                       C-O],[])
entry([close-fold                      C-C],[])
entry([enter-fold                    M-C-O],[])
entry([exit-fold                     M-C-C],[])
entry([unfold-fold                     C-U],[])
entry([create-fold                   M-C-N],[])
entry([toggle-file-fold            C-X C-F],[])
entry([toggle-attach-file-to-fold  C-X C-A],[])
menu(3,7)
dnl }}}
dnl {{{  4     -- editing
entry([kill-line                M-C-K],[])
entry([copy-to-kill-buffer        M-k],[k])
entry([insert-folded-kill-buffer  C-Y],[])
entry([delete-line            C-X C-K],[])
entry([undo-delete-line       C-X C-Y],[])
menu(4,7)
dnl }}}
dnl {{{  5     -- search & replace
entry([Search & Replace],[])
entry([incremental-search-forward   C-S],[])
entry([incremental-search-backward  C-R],[])
entry([query-replace-string         M-r],[r])
menu(5,7)
dnl }}}
dnl {{{  6     -- files
entry([save-file C-X C-S],[])
entry([read-file C-X C-R],[])
entry([insert-file C-X C-I],[	])
menu(6,7)
dnl }}}
dnl {{{  7,8,9 -- modes
entry([add-mode     ->],[])
entry([delete-mode  ->],[])
menu(7,7)
entry([autosave   C-X m a],[ma])
entry([c          C-X m c],[mc])
entry([overwrite  C-X m o],[mo])
entry([position   C-X m p],[mp])
entry([wrap       C-X m w],[mw])
menu(8,7)
entry([autosave   C-X C-M a],[a])
entry([c          C-X C-M c],[c])
entry([overwrite  C-X C-M o],[o])
entry([position   C-X C-M p],[p])
entry([wrap       C-X C-M w],[w])
menu(9,7)
dnl {{{  link menues together
link(7,0,8)
link(7,1,9)
dnl }}}
dnl }}}
dnl {{{  10    -- exit
entry([exit-origami  C-X C-C],[])
menu(10,7)
dnl }}}
dnl {{{  link menues together
link(1,1,2)
link(1,2,3)
link(1,3,4)
link(1,4,5)
link(1,5,6)
link(1,6,7)
link(1,7,10)
dnl }}}
activate(1)
