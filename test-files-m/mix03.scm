(define append-list
  (lambda (lst1 lst2)
    (if (null? lst1)
        lst2
        (cons (car lst1) (append-list (cdr lst1) lst2)))))

(append-list (quote (4 5)) (quote (6 7)))

(define reverse-list
  (lambda (lst)
    (if (null? lst)
        lst
        (append-list (reverse-list (cdr lst)) (cons (car lst) (quote ()))))))

(reverse-list (quote ()))
(reverse-list (quote (1 2 3 4)))
(reverse-list (quote (("computer" "science") "is" "awesome")))
