;-*- Mode:     Lisp -*-
;;;; Author:   Paul Dietz
;;;; Created:  Sat Oct 12 13:27:22 2002
;;;; Contains: Tests of TAGBODY

(in-package :cl-test)

(deftest tagbody.1
  (tagbody)
  nil)

(deftest tagbody.2
  (tagbody 'a)
  nil)

(deftest tagbody.3
  (tagbody (values))
  nil)

(deftest tagbody.4
  (tagbody (values 1 2 3 4 5))
  nil)

(deftest tagbody.5
  (let ((x 0))
    (values
     (tagbody
      (setq x 1)
      (go a)
      (setq x 2)
      a)
     x))
  nil 1)

(deftest tagbody.6
  (let ((x 0))
    (tagbody
     (setq x 1)
     (go a)
     b
     (setq x 2)
     (go c)
     a
     (setq x 3)
     (go b)
     c)
    x)
  2)

;;; Macroexpansion occurs after tag determination
(deftest tagbody.7
  (let ((x 0))
    (macrolet ((%m () 'a))
      (tagbody
       (tagbody
	(go a)
	(%m)
	(setq x 1))
       a ))
    x)
  0)

(deftest tagbody.8
  (let ((x 0))
    (tagbody
     (flet ((%f (y) (setq x y) (go a)))
       (%f 10))
     (setq x 1)
     a)
    x)
  10)

;;; Tag names are in their own name space
(deftest tagbody.9
  (let (result)
    (tagbody
     (flet ((a (x) x))
       (setq result (a 10))
       (go a))
     a)
    result)
  10)

(deftest tagbody.10
  (let (result)
    (tagbody
     (block a
       (setq result 10)
       (go a))
     (setq result 20)
     a)
    result)
  10)

(deftest tagbody.11
  (let (result)
    (tagbody
     (catch 'a
       (setq result 10)
       (go a))
     (setq result 20)
     a)
    result)
  10)

(deftest tagbody.12
  (let (result)
    (tagbody
     (block a
       (setq result 10)
       (return-from a nil))
     (setq result 20)
     a)
    result)
  20)
