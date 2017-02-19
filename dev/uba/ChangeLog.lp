v0.8a 10/13/2002 by Timothy Stark <sword7@speakeasy.org>
--------------------------------------------------------
* Corrected CSR address that OpenVMS can detect an LP11
  device exists or not properly.  After it was corrected
  and fixed, OpenVMS finally detected that LP11 device
  exists during autoconfiguration routines.

v0.8 5/17/02 by Timothy Stark <sword7@speakeasy.org>
----------------------------------------------------
* Initial LP11 Line Printer Interface Implementation
* Created a new create routine and removed obsolete
  create and setup routines.
* Added CancelInterrupt routine for CSR updates.
