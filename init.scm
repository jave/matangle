;;;; init.scm --- init file for Matangle
;;;; written by Joakim Verona <joakim@verona.se>
;;;;
;;;; 	Copyright (C) 2012 Joakim Verona
;;;; 
;;;; This file is free software; you can redistribute it and/or
;;;; modify it under the terms of the GNU General Public
;;;; License as published by the Free Software Foundation; either
;;;; version 3 of the License, or (at your option) any later version.
;;;; 
;;;; This file is distributed in the hope that it will be useful,
;;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;;;; General Public License for more details.
;;;; 
;;;; You should have received a copy of the GNU General Public
;;;; License along with this file; if not, write to the Free Software
;;;; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

(use-modules (system repl server))
(spawn-server)
(use-modules (system foreign))
(use-modules (ice-9 q))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;stuff for colors
;;extract rgba tuple list from a number like #x5f7f5f
(define ( hex2rgb hex) 
  `(,(bit-extract hex  0 8)
    ,(bit-extract hex  9 16)
    ,(bit-extract hex  16 24)
    ,(bit-extract hex  24 32)))

;;zenburn palette
(define *palette* '( #xdcdcccaa
                     #x2b2b2baa
                     #x3f3f3faa
                     #x4f4f4faa
                     #x5f5f5faa
                     #xdca3a3aa
                     #xcc9393aa
                     #xbc8383aa
                     #xac7373aa
                     #x9c6363aa
                     #x8c5353aa
                     #xdfaf8faa
                     #xf0dfafaa
                     #xe0cf9faa
                     #xd0bf8faa
                     #x5f7f5faa))

                                        ;tango palette
(define *palette* '(#x888a85aa #xeeeeecaa #x555753aa #x729fcfaa  #x8ae234aa #x729fcfaa #xad7fa8aa  #xf57900aa #xedd400aa #x2e3436aa #x2e3436aa))

;;lookup a color in the current palette
(define ( palette-lookup a)  (hex2rgb (list-ref  *palette* a )))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; scene support(levels, if you prefer)
(use-modules (oop goops))
(define-class <scene> ())

(define-class <accordion-scene> (<scene>))

(define-class <accordion-colors-scene> (<accordion-scene>))

(define-class <accordion-numbers-scene> (<accordion-scene>))

(define-class <hexagon-numbers-scene> (<scene>))

(define-class <box2d-scene> (<scene>))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;stuff for grid generation
;; first a set of formulas are generated, each formula generate 4 entries in *vec*
;; the entires are shuffled
;;then the final grid shape is drawn from the shuffled entries


;;generate a vector like A col B col C col OP col representing a formula triangle
(define (mathvector  a b c)
  `(( ,(number->string a) ,(palette-lookup c))
    ( ,(number->string b) ,(palette-lookup c))
    ( ,(number->string(* a b)) ,(palette-lookup c))
    ( "*" ,(palette-lookup c))))

;;temporary storage for generated math triangles
(define *vec* )

;;generates a sequence of random formula triangles
(define (make-formulas num-formulas)
  (set! *vec* '())
  (do ((j 1 (1+ j))) ((> j num-formulas))
    (set! *vec* (append *vec* (mathvector
                               (+ 1(random 10))
                               (+ 1(random 10))
                               j))))
  (set! *vec* (binary-shuffle-list *vec*)))

;;a queue of triangles
;;holds the view triangles so they can be deleted when wiping the grid
(define *triangles* (make-q))


;;the game grid
(define *grid-size* '(11 4))
(define *grid*
  (make-array '() (list 1 (car *grid-size*)) (list 1 (cadr *grid-size*))))


;;converts the *vec* linear sequence to the onscreen grid of triangles
;;these are then placed in *grid* (and redundantly in *triangles*)
(define-method  (make-grid (scene <accordion-colors-scene>))
    (make-grid-backdrop scene)

  (make-formulas 10)
  (do ((j 1 (1+ j)))
      ((> j (cadr *grid-size*) ))
    (do ((i (cond ((= j 2) 2)
                  ((= j 3) 2)
                  (#t 1))
            (1+ i)))
        ((> i (cond ((= j 2) (1- (car *grid-size*)))
                    ((= j 3) (1- (car *grid-size*)))
                    (#t (car *grid-size*)))))

      (let ( (triangle (addtriangle i j "" (cadar *vec*) 1)))
        (array-set! *grid* (append (car *vec*) (list  triangle)) i j)
        (enq! *triangles* triangle )
        (set! *vec* (cdr *vec*))))))

(define-method  (make-grid (scene <accordion-numbers-scene>))
  (make-grid-backdrop scene)

  (make-formulas 10)
  (do ((j 1 (1+ j)))
      ((> j (cadr *grid-size*) ))
    (do ((i (cond ((= j 2) 2)
                  ((= j 3) 2)
                  (#t 1))
            (1+ i)))
        ((> i (cond ((= j 2) (1- (car *grid-size*)))
                    ((= j 3) (1- (car *grid-size*)))
                    (#t (car *grid-size*)))))

      (let ( (triangle (addtriangle i j (caar *vec*) ( palette-lookup 2) 1)))
        (array-set! *grid* (append (car *vec*) (list  triangle)) i j)
        (enq! *triangles* triangle )
        (set! *vec* (cdr *vec*))))))


(define-method  (make-grid (scene <hexagon-numbers-scene>))
  (make-grid-backdrop scene)
  (make-formulas 6)
  (let ((color 1))
    (map (lambda (region)
           (set! color (1+ color))
           (map (lambda (region-coord)

                  ;;TODO refactor with make-grid-backdrop
                  ;;(enq! *triangles* (addtriangle (car region-coord) (cadr region-coord) "" (palette-lookup (1+ (modulo color 2) )) 0))
                  (let* ((i (car region-coord))
                         (j (cadr region-coord))
                         (triangle (addtriangle i j (caar *vec*) ( palette-lookup 2) 1)))
                    (array-set! *grid* (append (car *vec*) (list  triangle)) i j)
                    (enq! *triangles* triangle )
                    (set! *vec* (cdr *vec*))))
                region))
         (map (lambda (coord)
                (apply extract-formula-region-coords coord))
              (centers scene)))))


;;iterates the defined centers of the grid
;;TODO this could really also be used for make-grid which would be useful for other types of shapes like "hexagon"
(define-method  (make-grid-backdrop (scene <scene>))
  (let ((color 1))
    (map (lambda (region)
           (set! color (1+ color))
           (map (lambda (region-coord)
                  (enq! *triangles* (addtriangle (car region-coord) (cadr region-coord) "" (palette-lookup (1+ (modulo color 2) )) 0)))
                region))
         (map (lambda (coord)
                (apply extract-formula-region-coords coord))
              (centers scene)))))


(define-method (centers (scene <accordion-scene>))
  ;;TODO dont use this legacy global
  *centers*)
(define *centers*
  '((2 1)
    (4 2)
    (6 1)
    (8 2)
    (10 1)

    (2 4)

    (4 3)
    (6 4)
    (8 3)

    (10 4)))



(define-method (centers (scene <hexagon-numbers-scene>))
  '(
    (4 2)
    (6 1)
    (8 2)


    (4 3)
    (6 4)
    (8 3)))


(define-method  (make-grid (scene <box2d-scene>))
  (make-formulas 10)
  (do ((j 1 (1+ j)))
      ((> j (cadr *grid-size*) ))
    (do ((i (cond ((= j 2) 2)
                  ((= j 3) 2)
                  (#t 1))
            (1+ i)))
        ((> i (cond ((= j 2) (1- (car *grid-size*)))
                    ((= j 3) (1- (car *grid-size*)))
                    (#t (car *grid-size*)))))

      (let ( (triangle (addtriangle i j "" (cadar *vec*) 2)))
        (array-set! *grid* (append (car *vec*) (list  triangle)) i j)
        (enq! *triangles* triangle )
        (set! *vec* (cdr *vec*))))))

;;extract a triangle region from the grid consisting of 4 smaller triangles
;;region rotation depeds on position
(define (extract-formula-region x y)
  (let ((gp (lambda (x y)   (array-ref *grid* x y)))
        (y-offset  (* -1 (1+ (* 2 (- (modulo x 2) (modulo y 2)))))))
    (list (gp (1- x) y)
          (gp x y)
          (gp (1+ x) y)
          (gp  x (+ y-offset y)))))

;;refactor with extract-formula-region
(define (extract-formula-region-coords x y)
  (let ((gp (lambda (x y)   (list x y)))
        (y-offset  (* -1 (1+ (* 2 (- (modulo x 2) (modulo y 2)))))))
    (list (gp (1- x) y)
          (gp x y)
          (gp (1+ x) y)
          (gp  x (+ y-offset y)))))


(define (check-formula f)
  (apply equal? (map (lambda (x) (cadr x)) f)))

(define-method (check-all-grid-pos (scene <scene>))
  (delq #f (map (lambda (formula)  (if  ( check-formula (cdr formula)) (car formula) #f))
                (map (lambda (center)  (cons center  (extract-formula-region (car center) (cadr center )))) (centers scene)))))

(define-method (check-and-delete  (scene <scene>))
  (map (lambda (center) (formula-region-completed  (car center) (cadr center)))
       (check-all-grid-pos scene)))

(define (formula-region-completed x y)
  (let ((triangles (extract-formula-region x y)))
    (map (lambda (tri) (deltriangle (list-ref tri 2)))
         triangles))
  ;;(deltriangle (car  center) (cadr center))
  )


;;delete all triangles
(define (wipe-grid)
  (if (q-empty? *triangles*)
      #t
      (begin
        (deltriangle  (deq! *triangles*))
        (wipe-grid))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;grid interaction

;;swap two slots in grid
(define (swap-grid x1 y1 x2 y2)
  (format #t "~a ~a ~a ~a" x1 y1 x2 y2)
  (let ((t1 (array-ref *grid* x1 y1))
        (t2 (array-ref *grid* x2 y2)))
    (array-set! *grid* t2 x1 y1 )
    (array-set! *grid* t1 x2 y2)))


(define *current-scene*)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;callbacks

;;callback for the mx buttons 
(define (button-callback msg)
  (display "guile button callback ")  (display msg)
  (cond ((string=? msg "exit") (primitive-exit)) 
        ((string=? msg "new")
         (wipe-grid)
         (set! *current-scene* (cond ( (string=? "accordion-numbers" (getscenename))
                                       (make <accordion-numbers-scene>))
                                      ( (string=? "accordion-colors" (getscenename))
                                        (make <accordion-colors-scene>))
                                      ( (string=? "box2d" (getscenename))
                                        (make <box2d-scene>))
                                      ( (string=? "hexagon" (getscenename))
                                        (make <hexagon-numbers-scene>))))
         (make-grid *current-scene*))))

(addscenename "accordion-numbers")
(addscenename "accordion-colors")
(addscenename "box2d")
(addscenename "hexagon")



;;called when a triangle finishes swap animation
(define (animation-callback msg)
  (display "guile animation callback ")  (display msg)
  (check-and-delete *current-scene*))





;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;http://mumble.net/~campbell/scheme/shuffle.scm
;;stuff for shuffling a vector randomly
(use-modules (srfi srfi-1) (srfi srfi-8))
(define (flip-coin) (if (= 0 (random 2)) #t #f))

(define (binary-shuffle-list list)

  (define (bifurcate list left right)
    (if (null-list? list)
        (values left right)
        (let ((item (car list))
              (list (cdr list)))
          (if (flip-coin)
              (bifurcate list (cons item left) right)
              (bifurcate list left (cons item right))))))

  (let shuffle ((list list) (tail '()))
    (cond ((null-list? list)
           tail)
          ((null-list? (cdr list))
           (cons (car list) tail))
          ((null-list? (cddr list))
           (if (flip-coin)
               (cons (car list) (cons (cadr list) tail))
               (cons (cadr list) (cons (car list) tail))))
          (else
           (receive (left right) (bifurcate list '() '())
             (shuffle left (shuffle right tail)))))))
;;init
;;(addscenename "test")

(display "hello! ")
;;if we can get a value from a clutter combo here, C initialisation has gone well
(display (getscenename))


;;(make-grid        (make <accordion-numbers-scene>))

