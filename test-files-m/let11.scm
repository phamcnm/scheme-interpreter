(let ((x #t))
  (let ((y (if x 1 2)))
    (let ((x #f))
      y)))