devartRecorder
==============

I find notes from web radio stream and save them.  

I use the following libs: 

- fftw (for vamp)
- mpg123 (for mp3 to raw float conversion)
- lame (for raw float to mp3 conversion)
- chordino / NNLS (for note detection)
- VAMP (for running NNLS)
- curl (for getting mp3 streams)

I use the following command line tools: 

- sox (for normalizing audio and stats) 
- id3cp (for id3 tag copying)

There's both visual and audiory outputs for debugging, but this also runs headless (ofAppNoWindow).

license:  GPL   
(note this project uses FFTW w/ VAMP, which is GPL)
