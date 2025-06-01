## New features

We now have full support for `ArchiveRaw` and `Archive`.  
This means that multiple XML documents can share the same symbol table and be distributed as a single archive file.  
Each file can be addressed with a name. Right now no checks are performed about uniqueness.  

Also, reading or writing binary files is not hardened against attacks, and they will not be for quite a while.  
If you want to send PR about that they are welcome.  

We are almost ready to reach `v0.3.x`. In that new minor branch I will be refactoring the query system as outlined in https://github.com/lazy-eggplant/vs.xml/issues/4 and in the prior release notices.

### Other minor features

- Additional checks for word sizes and endianess when reading binaries, to ensure binary compatibility.

## Breaking

- The binary format is changing. Yet again. At this point it must be some kind of running joke.  
  But really, with archives being done, I don't expect any further major revision in the nearby future.