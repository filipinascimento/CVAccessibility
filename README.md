#CVAccessibility
CVAccessibility v0.7.1b

Software for calculating the accessibility measurement of nodes in complex networks. Check http://arxiv.org/abs/1101.5379v3 and http://arxiv.org/abs/0802.3665 .

##Usage

####Example Usage:
Calculating the level 4 accessibilty:

```
CVAccessibility -l 4 inputNetwork.xnet outputValues.txt
```

####Options

```
Usage: CVAccessibility [options] -i|<inputnetwork> [<outputfile>]
Options:
  -l  --level h           Obtain accessibility for h steps.
                          (defaults to h=3)
  -r  --random-walk       Use random-walk dynamics. If not specified,
                          use self-avoiding dynamics.
  -b  --ball-measurement  Take the measurement over the ball instead
                          considering levels from 0 to h.
  -p  --path-diversity    Use path diversity instead of probabilities.
  -n  --normalize         Normalize accessibility with the number of
                          accessed nodes.
  -o  --output-probs      Also output the access probabilities for each
                          pair of nodes.
  -a  --output-accessed   Also output the number of accessed nodes
                          alongside acessibility.
  -s  --live-stream       Stream the output as results are obtained.
                          (note that the results may be out of order)
  -j  --parallel-jobs N   Maximum number of parallel jobs for multicore
                          calculation. (defaults to N=8)
  -h  --help              Display this usage information.
  -V  --version           Show version number and quit.
  -v  --verbose           Make the calculation more talkative.
  -q  --quiet             Don not show calculation progress.
  -S  --show-status       Always show calculation progress.
Input:
  <inputnetwork>          Path to the network file in .xnet format.
  -i  --input-stdin       Uses stdin as input instead of a file.
  <outputfile>            Path to output the results. (If not defined, 
                          the software will output to stdout)

```

##Compile

We provide binaries for both windows 64bits and Mac OS X, Linux and Win32 users need to use the provided makefiles or scripts to compile the software. Note that windows and linux versions require OpenMP for parallel support. On linux, use GCC 4.8 or later as the default compiler. On windows we recomend the use of TDD-GCC toolchain:

http://tdm-gcc.tdragon.net/download


##Input format
The software uses the XNET network format. The file is a UTF-8 encoded text file as described bellow:

```
#vertices (num of vertices) (weighted|nonweighter)
"name of vertex 0" [weight]
"name of vertex 1" [weight]
...
#edges (nonweighted|weighted) (undirected|directed)
from to [weight]
...
#v "property name" (n|v2|v3|s)
property value of vertex 0
property value of vertex 1
...
#e "property name" (n|v2|v3|s)
property value of edge 0
property value of edge 1
...
```

Example of a square graph:

```
#vertices 4 nonweighted
"Lower Left"
"Upper Left"
"Upper Right"
"Lower Right"
#edges nonweighted
0 1
0 3
1 2
2 3

```

##TODO
 - Create updated makefiles and compiler tools.
 - Update the licence to be less restrictive.
 - Code documentation.
 - Better overall documentation.

#LICENSE

<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" property="dct:title">CVAccessibility</span> by <a xmlns:cc="http://creativecommons.org/ns#" href="https://github.com/filipinascimento/CVAccessibility" property="cc:attributionName" rel="cc:attributionURL">Filipi Nascimento Silva</a> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
