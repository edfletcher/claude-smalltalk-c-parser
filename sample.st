"A simple Smalltalk program to test the parser"

"Variable assignment"
x := 42.
y := 'Hello, Smalltalk!'.
z := #symbol.

"Unary message"
x negated.

"Binary message"
3 + 4.
x > 10.

"Keyword message"
Array new: 10.
Dictionary at: #key put: 'value'.

"Cascaded messages"
OrderedCollection new
    add: 1;
    add: 2;
    add: 3;
    yourself.

"Block with parameters"
[:a :b | a + b].

"Return statement"
^self size.

"Nested expressions"
x := (y isNil) 
    ifTrue: [0] 
    ifFalse: [y size].

"Array literal"
#(1 2 3 'text' $c #symbol).

"Complete method definition would be parsed separately, but here's an example"
"
factorial
    self = 0 ifTrue: [^1].
    self = 1 ifTrue: [^1].
    ^self * (self - 1) factorial
"