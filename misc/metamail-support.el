;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(require 'transparent)
(require 'rmail)

(defvar rmail-never-execute-automatically t
          "*Prevent metamail from happening semi-automatically")

(define-key rmail-mode-map "M" 'rmail-execute-content-type)

(defun rmail-check-content-type ()
    "Check for certain Content Type headers in mail"
      (rmail-maybe-execute-content-type nil))

(defun rmail-execute-content-type ()
    "Check for certain Content Type headers in mail"
      (interactive)
        (rmail-maybe-execute-content-type t))

(defun rmail-handle-content-type (ctype override dotoggle)
  (let ((oldpt nil)
	(oldbuf (current-buffer))
	(fname (make-temp-name "/tmp/rmailct")))
    
    (cond
     ((and rmail-never-execute-automatically (not override))
      (progn
	(if dotoggle (rmail-toggle-header))
	(message (concat "Type 'M' to view '" ctype "' message."))))
     ((or override
	  (getenv "MM_NOASK")
          (y-or-n-p (concat "MIME-interpret this '" ctype "' message? ")))
      (progn
	(write-region (point-min) (point-max)
		      fname 'nil 'silent)
	(if dotoggle (rmail-toggle-header))
	(cond
	 ((eq window-system 'x)
	  (switch-to-buffer-other-window "METAMAIL")
	  (erase-buffer)
	  (pop-to-buffer oldbuf)
	  (start-process "metamail"  "METAMAIL" "metamail" "-m"
			 "rmail" "-p" "-x" "-d" "-z" "-q" "-R" fname)
	  (message "Starting metamail.  Sending output to METAMAIL buffer."))
	 ((eq window-system 'mgr)
	  (save-excursion
	    (set-buffer (get-buffer-create "METAMAIL"))
	    (erase-buffer))
	  (let ((f (mgr-getinfo-get-font))
		(coords (mgr-getinfo-get-coords))
		(sys (mgr-getinfo-get-system)))
	    (if (and (= (length f) 4)
		     (= (length coords) 4)
		     (>= (length sys) 4))
		(let ((x (+ (nth 0 coords) 32))
		      (y (- (nth 1 coords) 32))
		      (w (nth 2 coords))
		      (h (nth 3 coords))
		      (dw (nth 1 sys))
		      (dh (nth 2 sys))
		      (bor (nth 3 sys))
		      (fn (nth 2 f)))
		  (setq w (+ w bor bor))
		  (setq h (+ h bor bor))
		  (if (> (+ x w) dw)
		      (setq x 0))
		  (if (< y 0)
		      (setq y (- dh h)))
		  (let ((pty (mgr-make-separate-window-font x y w h fn)))
		    (if pty
			(start-process
			 "metamail" "METAMAIL"
			 "mnew" "-t" pty
			 (format "-x%d" x)
			 (format "-y%d" y)
			 (format "-w%d" w)
			 (format "-h%d" h)
			 (format "-f%d" fn)
			 (format "METAMAIL_PAGER=mless metamail -m rmail -p -d -z %s" fname))
		      (error "failed to open MGR window for METAMAIL"))))
	      (error "failed to inquire MGR window environment"))))
	 (t
	  (switch-to-buffer "METAMAIL")
	  (erase-buffer)
	  (sit-for 0)
	  (transparent-window
	   "METAMAIL"
	   "metamail"
	   (list "-m" "rmail" "-p" "-d" "-z" "-q" "-R" fname)
	   nil
	   (concat
	    "\n\r\n\r*****************************************"
	    "*******************************\n\rPress any key "
	    "to go back to EMACS\n\r\n\r***********************"
	    "*************************************************\n\r")
	   )))))
     (t (progn
	  (if dotoggle (rmail-toggle-header))
	  (message (concat "Type 'M' to view MIME-format mail.")))))))

(defun rmail-maybe-execute-content-type (dorun)
  "Check for certain Content Type headers in mail"
  (cond
   ((not (getenv "NOMETAMAIL"))
    (let* ((ct nil)
	   (needs-toggled nil))
      (save-excursion
	(save-restriction
	  (widen)
	  (goto-char (rmail-msgbeg rmail-current-message))
	  (forward-line 1)
	  (if (and dorun (= (following-char) ?1)) (setq needs-toggled t))
	  (if (= (following-char) ?0)
	      (narrow-to-region
	       (progn (forward-line 2)
		      (point))
	       (progn (search-forward "\n\n"
				      (rmail-msgend rmail-current-message)
				      'move)
		      (point)))
	    (narrow-to-region (point)
			      (progn (search-forward "\n*** EOOH ***\n")
				     (beginning-of-line) (point))))
	  (setq ct (mail-fetch-field "content-type" t))
	  (setq cte (mail-fetch-field "content-transfer-encoding" t))))
      (cond
       ((or ct cte)
	(let ((ctlc (downcase (or ct "text/plain")))
	      (ctelc (downcase (or cte "7bit"))))
	  (cond ((or
		  (and (not (string= ctelc "7bit"))
		       (not (string= ctelc "8bit")))
		  (and (not (string= ctlc "text"))
		       (not (string= ctlc "text/plain"))
		       (not (string= ctlc "text/plain; charset=iso-8859-1"))
		       (not (string= ctlc "text/plain; charset=\"iso-8859-1\""))
		       (not (string= ctlc "text/plain; charset=us-ascii"))
		       (not (string= ctlc "text/plain; charset=\"us-ascii\""))))
		 (progn
		   (if needs-toggled (rmail-toggle-header))
		   (rmail-handle-content-type
		    (concat ctlc " - " ctelc) dorun needs-toggled)))
		(needs-toggled
		 (rmail-toggle-header))))))))))

(setq rmail-show-message-hook
      '(lambda () (rmail-check-content-type)))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
