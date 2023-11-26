Pin matching between CSchem and CPCB
====================================

A CSchem symbol with a pin labeled “pin:*A*” will automatically match
to a CPCB footprint with a pin labeled “*A*”. This is true if *A* is a
word, a number, an arbitrary symbol (such as “+”), or a combination. A
CSchem symbol with a pin labeled “pin:*N*/*A*” will match to a CPCB
footprint with a pin labeled “*N*” or a pin labeled “*A*”. It will
also match to a pin labeled “*M*/*A*”, even if *M* is not the same as
*N*. It will however not match to a pin labeled “*N*/*B*” if *B* is
not the same as *A*.

If a footprint has pins labeled “*N*/*A*” and “*M*/*A*”, both will
match to a symbol with a pin labeled “pin:*A*” (or “pin:*J*/*A*” where
*J* may or may not be equal to *N* or *M*), and the net verifier will
expect both to be connected. However, the net highlighter may fail to
highlight unconnected pins in blue. 
