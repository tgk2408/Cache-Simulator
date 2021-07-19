Cache Simulator
Madhav Mittal  - CS19B029
Kshitij Raj    - CS19B061
Mandala Tejesh - CS19B062

Note: the memory address are read from a separate text file, make sure the file is in the same directory and give the name of file as input in the command line when prompted.

command line input format:
<Cache Size>
<Block Size>
<Associativity>
<Replacement Policy>
<file name containing memory traces>

Output format: (STDOUT)

<cache Size>	#Cache size
<block Size>	#Cache line size
<Associativity>
<Replacement Policy>
<Cache accesses>	    #Cache accesses
<Read accesses>	        #Read accesses
<Write accesses>	    #Write accesses
<Cache misses>	        #Cache misses
<Compulsory misses>	    #Compulsory misses
<Capacity misses>	    #Capacity misses
<Conflict misses>	    #Conflict misses
<Read misses>	        #Read misses
<Write misses>	        #Write misses
<Dirty blocks evicted>	#Dirty blocks evicted
