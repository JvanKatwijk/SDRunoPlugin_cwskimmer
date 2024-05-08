
---------------------------------------------------------------------
A skimmer for cw decoding
---------------------------------------------------------------------

Tuning to a CW transmission on shortwave is not always easy,
transmissions are (often very) short and by the time tuning
is correct, transmission stops.

As an alternative I developed a simple skimmer for Linux, and
derived from that, this simple plugin.

The idea is simple: take 500 FFT's per second over the incoming samples,
then select a few (up to 32) successive "bin"s, and use the data in the bin as sample.
This means 500 samples a second.

For each of the selected bins a decoder is installed, the approach
in the decoder is straightforward in detecting "space" and "data".
It is known that that no symbol in morse code takes 14 or more
space/data combinations,
so we collect up to 14 space/data combinations, extract the intersymbol
space and an estimate of the size for a dot,  and (try to) map the symbol,
i.e. space and dash/dots, using a table with the predefined symbols.

Showing a couple of samplestreams converted to symbols, usually
gives a good indication of whether or not there is a regular
cw transmission

------------------------------------------------------------------------
The plugin
-------------------------------------------------------------------------

The plugin is extremely simple, a single top line with a few
selectors and 32 lines, one for each bin - if selected.

![overview](/skimmer-1.png?raw=true)

The top line shows spinboces for selection of the number of bins, equally
divided in bins below and above the frequency of the center bin.
Up to 30 bins can be selected.

With the button next to the reset button a threshold can be set,
used to separate spaces from data.

The contents of a "bin" can be considered to be the "energy"
in the received signal on the range of frequencies the bin stands for.
If the bin is "wide" it is less likely that a small signal with a low amplitude
is detected than if the signal is caught in a "small" bin. On the other hand,
if a "bin" represents a small segment of frequencies, a strong signal will
leak to adjacent bins.

The skimmer povides a choice between 3 widths of the bins. The input has a
samplerate of 192000, using an FFT width of 512 leads to a bin width
of app 375 Hz, an FFT width of 1024 to a bin width of 187, and an FFT width
of 2048 to a bin width of 93Hz.

Of course, a bin width of 375 Hz leads to a maximum span of app 11.5 Khz,
a bin width of 187 to app 5.6 KHz and a bin width of 93 to a span of app 3 KHz.

Finally, the resulting span is shown on the top line.


