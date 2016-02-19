# lua-emacs

Lua script engine from Emacs Lisp. This package is inspired by [mruby-lua](https://github.com/dyama/mruby-lua).


## How to build

```
% make LUA_VERSION=5.2
# Default Lua version is 5.2. You need to specify if you use other than 5.2.
```


## Sample Code

``` lisp
(defun lua-factorial ()
  (interactive)
  (let* ((state (lua-new))
         (got (lua-do-string state
                             "
function factorial(n)
  if n <= 1 then
    return 1
  else
    return n * factorial(n - 1)
  end
end

ans = factorial(10)
")))
    (message "@@ %s" (lua-get-global state "ans"))))
;; => @@ 3628800
```

``` lisp
(defun lua-set-global-test ()
  (interactive)
  (let ((state (lua-new)))
    (lua-set-global state "a" "bar")
    (lua-set-global state "b" 100)
    (lua-set-global state "c" t)
    (lua-do-string state "
print(a)
print(b)
print(c)
") ;; Output to stdout
    (message "@@ a=%s, b=%s, c=%s"
             (lua-get-global state "a")
             (lua-get-global state "b")
             (lua-get-global state "c"))))

;; => @@ a=bar, b=100, c=t
```
