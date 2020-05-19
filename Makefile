CFLAGS = /MD /W3 /D_CRT_SECURE_NO_DEPRECATE

build: tema1.obj hashtable.obj
	cl.exe /Feso-cpp.exe tema1.obj hashtable.obj

tema1.obj: tema1.c      
	cl.exe $(CFLAGS) /Fotema1.obj /c tema1.c

hashtable.obj: hashtable.c     
	cl.exe $(CFLAGS) /Fohashtable.obj /c hashtable.c

clean:
	del *.obj *.exe
