iq.obj:       iq.c iq.h register.h
              cl /c /DDUMMY /AL /W3 iq.c

link9000.obj: link9000.c trans.h
              cl /c /AL /W3 link9000.c

doprint.obj:  doprint.h doprint.c
              cl /c /AL /W3 doprint.c

query.obj:    query.c
              cl /c /AL /W3 query.c

sprint.obj:   sprint.c
              cl /c /AL /W3 sprint.c

dostype.obj:  dostype.c dostype.h
              cl /c /AL /W3 dostype.c

glossary.obj: glossary.c glossary.h query.c
              cl /c /AL /W3 glossary.c

menu.obj:     menu.c menu.h
              cl /c /DDUMMY /AL /W3 menu.c

iqcomms.obj:  iqcomms.c iqtimer.h  
              cl /c /AL /W3 iqcomms.c

trans.obj:    trans.c trans.h
              cl /c /AL /W3 trans.c

dskstat.obj:  dskstat.c
              cl /c /AL /W3 dskstat.c

fviewer.obj:  fviewer.c
              cl /c /AL /W3 fviewer.c

dirlst.obj:   dirlst.c
              cl /c /AL /W3 dirlst.c

view.obj:     view.c
              cl /c /AL /W3 view.c

printer.obj:  printer.h printer.c
              cl /c /AL /W3 printer.c

drvlist.obj:  drvlist.c
              cl /c /AL /W3 drvlist.c

iq9000.obj:   iq9000.c
              cl /c /AL /W3 iq9000.c

asciitab.obj: asciitab.c
              cl /c /AL /W3 asciitab.c

shell.obj:    shell.c
              cl /c /AL /W3 shell.c

config.obj:   config.c
              cl /c /AL /W3 config.c

fsearch.obj:  fsearch.c
              cl /c /AL /W3 fsearch.c

file.obj:     file.c
              cl /c /AL /W3 file.c

setup.obj:    setup.c
              cl /c /AL /W3 setup.c

decode.obj:   decode.c
              cl /c /AL /W3 decode.c

drexists.obj: drexists.c
              cl /c /AL /W3 drexists.c

break.obj:    break.h break.c
              cl /c /AL /W3 break.c

models.obj:   models.c
              cl /c /AL /W3 models.c

comm.obj:     comm.c comm.h
              cl /c /AL /W3 comm.c

selfdir.obj:  selfdir.c
              cl /c /AL /W3 selfdir.c

doscmds.obj:  doscmds.c dostype.h
              cl /c /AL /W3 doscmds.c

palette.obj:  palette.c
                cl /c /AL /W3 palette.c

support.obj:  support.c
                cl /c /AL /W3 support.c

shwdir.obj:   shwdir.c
              cl /c /AL /W3 shwdir.c

windows.obj:  windows.c
                cl /c /AL /W3  windows.c

getkey.obj: getkey.c
                cl /c /AL /W3   getkey.c

getstr.obj: getstr.c
               cl /c /AL /W3   getstr.c 
               

iq.exe: iq.obj  setup.obj models.obj comm.obj getstr.obj menu.obj support.obj  \
        windows.obj trans.obj dskstat.obj file.obj printer.obj getkey.obj  \
        palette.obj  iqcomms.obj asciitab.obj fviewer.obj selfdir.obj \
        drvlist.obj dirlst.obj drexists.obj view.obj shell.obj \
        doscmds.obj shwdir.obj fsearch.obj config.obj break.obj \
        glossary.obj doprint.obj  dostype.obj sprint.obj query.obj iq9000.obj \
        decode.obj link9000.obj
        link @iq.lnk

