# EB LISP

A lisp interpreter. Lexically scoped, with proper closures, and easily embeddable. See repl.cpp in tools/ for an example of embedding the interpreter in a C++ application.

``` scheme
(def closure-test
     (let ((a 1))
       (lambda (b)
         (set a (+ a b))
         a)))

(println (closure-test 3))  ;; 4
(println (closure-test 38)) ;; 42
```

## Example

Here's something a little more advanced, mandelbrot sets:

``` scheme

(def iters 500)

(def mandelbrot
     (lambda (z c iter-depth)
       (if (> iter-depth 0)
           (if (< (abs z) 2.0)
               (mandelbrot (+ (* z z) c) c (- iter-depth 1))
               false)
           true)))

(def make-matrix
     (lambda (xl yl n)
       (if (< n 1)
           (cons xl yl)
           (begin
             (def list-x (cons (- 1.0 (* n (/ 3.5 100.0))) xl))
             (def list-y (if (< n 50)
                             (cons (- 1.0 (* n (/ 2.0 50.0))) yl)
                             yl))
             (make-matrix list-x list-y (- n 1))))))

(def data-mat (make-matrix null null 100))

(dolist
 (lambda (i)
   (dolist
    (lambda (r)
      (if (mandelbrot (complex 0.0 0.0) (complex r i) iters)
          (print "*")
          (print " ")))
    (car data-mat))
   (println ""))
 (cdr data-mat))

```

Output:
```


                               *
                               *
                             *****
                             *****
                             *****
                              ****
                          * ********* *
                        **************    **
                  ***  *********************
                   ***********************
                    ************************
                   ****************************
                   ***************************
                  ****************************
                 ******************************
                  ******************************
                 ********************************     ***** *
                 ********************************   *********
                 ********************************  ************
                  ******************************* *************
                  ******************************* *************
                   ****************************** *****************
                     ****************************************************************
                   ****************************** *****************
                  ******************************* *************
                  ******************************* *************
                 ********************************  ************
                 ********************************   *********
                 ********************************     ***** *
                  ******************************
                 ******************************
                  ****************************
                   ***************************
                   ****************************
                    ************************
                   ***********************
                  ***  *********************
                        **************    **
                          * ********* *
                              ****
                             *****
                             *****
                             *****
                               *
                               *



```