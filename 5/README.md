# Hindley-Milner Type Inference for S-expressions

## Build & Run
Eventually it's Conan :)

### Set python env:

```bash
python -m venv conan_env
source conan_env/bin/activate 
pip install conan 
```

### Build 
Example for Release mode:

``` bash
conan build . 
```
### Run tests

```
./build/Release/tests 
```

### Run the binary
An input file should contain **one** S-expression 
``` bash
./build/Release/hindley_milner <INPUT_FILE>
```

For an [example from the HW description](examples/input.txt) the output file (in the same directory as the input file, so ```examples/output.txt``` in this case) will contain:

```
f : (forall (t8 t5) ((func t5 t8) (list t5) (list t8)))
```
which satisfies the actual type of f.


## Limitations

1. The parser only recognizes the following keywords:
    - ```lambda```
    - ```define```
    - ```case```
    - ```cons```
    - ```nil```
2. Syntax requirements
    - ```(lambda (param1 param2 ...) body1 body2 ...)``` where parameters are **symbols**
    - ```(define name expression)``` where name is a **symbol**
    - ```(case expr (nil nil_expr) ((cons x xs) cons_expr))```, that is, **pattern matching is limited**
    - ```define``` should **always be the top function**

Examples of valid s-expressions:
```
(define identity (lambda (x) x))

(define map (lambda (f xs) 
  (case xs 
    (nil nil) 
    ((cons x xs1) (cons (f x) (map f xs1))))))
```

Unsupported s-expressions:

```
(lambda (x) x)

(let ((x 1)) body)

(case x ((1) expr1) ...) 
```