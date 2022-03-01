(define x 23)

(define fun1
  (lambda ()
    (let ((x 251))
      (fun2))))

(define fun2
  (lambda ()
    x))

(fun1)
(fun2)
