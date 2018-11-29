# EB LISP

## Introduction
A lisp dialect, runs standalone, or can be easily embedded in C++ applications. Some notable features listed below:

#### Lexical closures
``` scheme
(def closure-test
     (let ((a 1))
       (lambda (b)
         (set a (+ a b))
         a)))

(println (closure-test 3))  ;; 4
(println (closure-test 38)) ;; 42
```

#### Builtin unicode
```scheme
(length "안녕 세상") ;; 5

;; You can get crazy with this, but better to write your code in english :)
(defn शामिल (मूल्य सूची)
  "सूची में मूल्य पाएं"
  (if (null? सूची)
      false
      (if (not (pair? (car सूची)))
          (if (equal? मूल्य (car सूची))
              true
              (शामिल मूल्य (cdr सूची)))
          (or (शामिल मूल्य (car सूची))
              (शामिल मूल्य (cdr सूची))))))

(शामिल 1 (list 2 3 4 1 5)) ;; true
(help शामिल) ;; सूची में मूल्य पाएं
```

#### Variadic functions
```scheme
;; Extra args are passed in a list, bound to the ... variable
(defn example* (factor ...)
  (if (not (null? ...))
      (begin
        (print (* factor (car ...)) " ")
        (apply example* (cdr ...)))))

(example* 10 1 2 3 4 5) ;; 10 20 30 40 50
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