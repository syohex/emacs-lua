;;; test.el --- lub binding test -*- lexical-binding: t -*-

;; Copyright (C) 2022 by Shohei YOSHIDA

;; Author: Shohei YOSHIDA <syohex@gmail.com>

;; This program is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <http://www.gnu.org/licenses/>.

;;; Commentary:

;;; Code:

(require 'ert)
(require 'lua)

(ert-deftest boolean ()
  "boolean type"
  (let ((ls (lua-new)))
    (lua-do-string ls "
a = true
b = false
")
    (should (lua-get-global ls "a"))
    (should-not (lua-get-global ls "b"))))

(ert-deftest fixnum ()
  "fixnum type"
  (let ((ls (lua-new)))
    (lua-do-string ls "
a = 1
b = 1 + 2
")
    (should (= (lua-get-global ls "a") 1))
    (should (= (lua-get-global ls "b") 3))))

(ert-deftest float ()
  "float type"
  (let ((ls (lua-new)))
    (lua-do-string ls "
a = 1.5
b = 1 / 4.0
")
    (should (= (lua-get-global ls "a") 1.5))
    (should (= (lua-get-global ls "b") 0.25))))

(ert-deftest string ()
  "string type"
  (let ((ls (lua-new)))
    (lua-do-string ls "
a = \"hello world\"
")
    (should (string= (lua-get-global ls "a") "hello world"))))

(ert-deftest table ()
  "table type"
  (let ((ls (lua-new)))
    (lua-do-string ls "
a = {}
a[\"x\"] = 10
a[10] = \"hello\"
")
    (let ((hash (lua-get-global ls "a")))
      (should (eq (type-of hash) 'hash-table))
      (should (= (gethash "x" hash) 10))
      (should (string= (gethash 10 hash) "hello")))))

(ert-deftest set-global ()
  "Set global variable from Emacs"
  (let ((ls (lua-new)))
    (lua-set-global ls "a" 1234)
    (should (= (lua-get-global ls "a") 1234))
    (lua-set-global ls "b" "Hello")
    (should (string= (lua-get-global ls "b") "Hello"))))

;;; test.el ends here
