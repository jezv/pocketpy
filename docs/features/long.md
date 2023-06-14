---
icon: dot
title: Arbitrary Sized Integers
---

Unlike cpython, pkpy's `int` is of limited precision.
In 32 bit platforms, it is 30 bit;
in 64 bit platforms, it is 62 bit.

For arbitrary sized integers, we provide a builtin `long` type, just like python2's `long`.
`long` is implemented via pure python in [_long.py](https://github.com/blueloveTH/pocketpy/blob/main/python/_long.py).

```python
a = long(2)         # use long() to create a long explicitly
print(a ** 1000)
# 10715086071862673209484250490600018105614048117055336074437503883703510511249361224931983788156958581275946729175531468251871452856923140435984577574698574803934567774824230985421074605062371141877954182153046474983581941267398767559165543946077062914571196477686542167660429831652624386837205668069376L
```

!!!
This feature is still under development.
Some operations are missing, and some operations are not optimized.
!!!