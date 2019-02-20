static const char* onloads =
";;;\n"
";;; A few functions, including 'require', ship with the runtime, so that they're\n"
";;; always available. These are functions that could be written in C++, but due\n"
";;; to either data dependencies, or conciseness, they're easier to write in\n"
";;; EBL. A script, format-onloads.ebl, will convert the file to a C string\n"
";;; literal, to be included by the VM during compilation.\n"
";;;\n"
"\n"
"(namespace std\n"
"  (defn some (pred lat)\n"
"    (if (null? lat)\n"
"        false\n"
"        (if (pred (car lat))\n"
"            (car lat)\n"
"            (recur pred (cdr lat))))))\n"
"\n"
"(namespace sys\n"
"  ;; NOTE: When deciding between a mutable global variable and a mutable setter,\n"
"  ;; the setter is better for sandboxing. We want users to be able to change the\n"
"  ;; load path, but also want to make it possible for users to disable\n"
"  ;; mutability (by overwriting the lambda bound to the base path setter).\n"
"  (def-mut set-require-base-path! null)\n"
"\n"
"  (def require\n"
"       (let-mut ((base-path (string 'ebl/)))\n"
"         (set set-require-base-path! (lambda (str) (set base-path str)))\n"
"         ((lambda ()\n"
"            (def-mut required-set null)\n"
"            (lambda (file-name)\n"
"              (def full-path (string base-path file-name))\n"
"              (let ((found (std::some (lambda (n)\n"
"                                        (equal? n full-path)) required-set)))\n"
"                (if found\n"
"                    null\n"
"                    (begin\n"
"                      (load full-path)\n"
"                      (set required-set (cons full-path required-set)))))))))))\n"
"\n"
"(def require sys::require)\n"
"\n"
"(defn list (...) ...)\n"
;