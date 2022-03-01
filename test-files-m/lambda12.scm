(let ((a 1))
  (let ((f (lambda () a)))
    (let ((a 2))
      (f))))
