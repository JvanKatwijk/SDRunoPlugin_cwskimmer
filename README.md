
---------------------------------------------------------------------
A skimmer for cw decoding
---------------------------------------------------------------------

Tuning to a CW transmission on shortwave is not always easy,
transmissions are (often very) short and by the time tuning
is correct, transmission stops.

As an alternative I developed a simple skimmer for Linux, and
derived from that, this simple plugin.

The isdea is simple: take 500 FFT's over some region, then select a
few (up to 32) successive "bin"s, and use the data in the bin as sample.
This means 500 samples a second.

For each of the selected bins a decoder is installed, the approach
in the decoder is straightforward in detecting "space" and "data".
The morse code shows that no symbol takes 14 or more space/data combinations,
so we collect up to 14 space/data combinations, extract the intersymbol
space and (try to) map the symbol, i.e. space and dash/dots, to a
table with the predefined symbols.

Showing a dozen or so samplestreams converted to symbols, usually
gives a good indication of whether or not there is a regular
cw transmission

------------------------------------------------------------------------
The plugin
-------------------------------------------------------------------------

The plugin is extremely simple, a single top line with a few
selectors and 32 lines, one for each bin - if selected.

The top line alows selection of a center bin and a number of bins, equally
divided in bins below and above the frequency of the center bin.

The function of the rest is obvious




