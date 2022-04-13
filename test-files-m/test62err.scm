(let ((x 3))
  (letrec ((y x) (x 4))
    y))
