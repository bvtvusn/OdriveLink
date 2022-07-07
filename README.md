# OdriveLink
This Arduino Library communicates with the Odrive BLDC controller. 
The communication supports checksums. The advantage of this library is that it does not hang if the Odrive does not respond to a command. 
Instead the callback will just not be called.
