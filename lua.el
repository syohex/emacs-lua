;;; lua.el --- Lua in Emacs -*- lexical-binding: t -*-

;; Copyright (C) 2022 by Shohei YOSHIDA

;; Author: Shohei YOSHIDA <syohex@gmail.com>
;; URL: https://github.com/syohex/emacs-lua
;; Version: 0.01

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

(require 'lua-core)

;;;###autoload
(defun lua-new ()
  (lua-core-init))

(defun lua-close (state)
  (lua-core-close state))

(defun lua-do-string (state str)
  (lua-core-do-string state str))

(defun lua-get-global (state key)
  (lua-core-get-global state key))

(defun lua-set-global (state key val)
  (lua-core-set-global state key val))

(provide 'lua)

;;; lua.el ends here
