(define length-list
  (lambda (lst)
    (if (null? lst)
        0
        (+ 1 (length-list (cdr lst))))))
(length-list (quote ()))
(length-list (quote (4 5 6)))
