;-*- Mode:     Lisp -*-
;;;; Author:   Paul Dietz
;;;; Created:  Fri Dec 20 05:58:06 2002
;;;; Contains: BOA Constructor Tests

(in-package :cl-test)

;;; See the DEFSTRUCT page, and section 3.4.6 (Boa Lambda Lists)

(defstruct (sbt-01 (:constructor sbt-01-con (b a c)))
  a b c)

(deftest structure-boa-test-01/1
  (let ((s (sbt-01-con 1 2 3)))
    (values (sbt-01-a s)
	    (sbt-01-b s)
	    (sbt-01-c s)))
  2 1 3)

(defstruct (sbt-02 (:constructor sbt-02-con (a b c))
		   (:constructor sbt-02-con-2 (a b))
		   (:constructor sbt-02-con-3 ()))
  (a 'x) (b 'y) (c 'z))

(deftest structure-boa-test-02/1
  (let ((s (sbt-02-con 1 2 3)))
    (values (sbt-01-a s)
	    (sbt-01-b s)
	    (sbt-01-c s)))
  1 2 3)

(deftest structure-boa-test-02/2
  (let ((s (sbt-02-con-2 'p 'q)))
    (values (sbt-01-a s)
	    (sbt-01-b s)
	    (sbt-01-c s)))
  p q z)

(deftest structure-boa-test-02/3
  (let ((s (sbt-02-con-3)))
    (values (sbt-01-a s)
	    (sbt-01-b s)
	    (sbt-01-c s)))
  x y z)

;;; &optional in BOA LL

(defstruct (sbt-03 (:constructor sbt-03-con (a b &optional c)))
  c b a)

(deftest structure-boa-test-03/1
  (let ((s (sbt-03-con 1 2)))
    (values (sbt-03-a s) (sbt-03-b s)))
  1 2)

(deftest structure-boa-test-03/2
  (let ((s (sbt-03-con 1 2 3)))
    (values (sbt-03-a s) (sbt-03-b s) (sbt-03-c s)))
  1 2 3)


(defstruct (sbt-04 (:constructor sbt-04-con (a b &optional c)))
  (c nil) b (a nil))

(deftest structure-boa-test-04/1
  (let ((s (sbt-04-con 1 2)))
    (values (sbt-04-a s) (sbt-04-b s) (sbt-04-c s)))
  1 2 nil)

(deftest structure-boa-test-04/2
  (let ((s (sbt-04-con 1 2 4)))
    (values (sbt-04-a s) (sbt-04-b s) (sbt-04-c s)))
  1 2 4)


(defstruct (sbt-05 (:constructor sbt-05-con (&optional a b c)))
  (c 1) (b 2) (a 3))

(deftest structure-boa-test-05/1
  (let ((s (sbt-05-con)))
    (values (sbt-05-a s) (sbt-05-b s) (sbt-05-c s)))
  3 2 1)

(deftest structure-boa-test-05/2
  (let ((s (sbt-05-con 'x)))
    (values (sbt-05-a s) (sbt-05-b s) (sbt-05-c s)))
  x 2 1)

(deftest structure-boa-test-05/3
  (let ((s (sbt-05-con 'x 'y)))
    (values (sbt-05-a s) (sbt-05-b s) (sbt-05-c s)))
  x y 1)

(deftest structure-boa-test-05/4
  (let ((s (sbt-05-con 'x 'y 'z)))
    (values (sbt-05-a s) (sbt-05-b s) (sbt-05-c s)))
  x y z)


(defstruct (sbt-06 (:constructor sbt-06-con (&optional (a 'p) (b 'q) (c 'r))))
  (c 1) (b 2) (a 3))

(deftest structure-boa-test-06/1
  (let ((s (sbt-06-con)))
    (values (sbt-06-a s) (sbt-06-b s) (sbt-06-c s)))
  p q r)

(deftest structure-boa-test-06/2
  (let ((s (sbt-06-con 'x)))
    (values (sbt-06-a s) (sbt-06-b s) (sbt-06-c s)))
  x q r)

(deftest structure-boa-test-06/3
  (let ((s (sbt-06-con 'x 'y)))
    (values (sbt-06-a s) (sbt-06-b s) (sbt-06-c s)))
  x y r)

(deftest structure-boa-test-06/4
  (let ((s (sbt-06-con 'x 'y 'z)))
    (values (sbt-06-a s) (sbt-06-b s) (sbt-06-c s)))
  x y z)


(defstruct (sbt-07 (:constructor sbt-07-con
				 (&optional (a 'p a-p) (b 'q b-p) (c 'r c-p)
					    &aux (d (list (notnot a-p)
							  (notnot b-p)
							  (notnot c-p))))))
  a b c d)

(deftest structure-boa-test-07/1
  (let ((s (sbt-07-con)))
    (values (sbt-07-a s) (sbt-07-b s) (sbt-07-c s) (sbt-07-d s)))
  p q r (nil nil nil))

(deftest structure-boa-test-07/2
  (let ((s (sbt-07-con 'x)))
    (values (sbt-07-a s) (sbt-07-b s) (sbt-07-c s) (sbt-07-d s)))
  x q r (t nil nil))

(deftest structure-boa-test-07/3
  (let ((s (sbt-07-con 'x 'y)))
    (values (sbt-07-a s) (sbt-07-b s) (sbt-07-c s) (sbt-07-d s)))
  x y r (t t nil))

(deftest structure-boa-test-07/4
  (let ((s (sbt-07-con 'x 'y 'z)))
    (values (sbt-07-a s) (sbt-07-b s) (sbt-07-c s) (sbt-07-d s)))
  x y z (t t t))


