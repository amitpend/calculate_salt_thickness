## Calculate Thickness of Subsurface Salt Deposits

This program calculates salt thickness in a whole volume (3D) and saves as a text file with format X_Location, Y_Location, Thickness. This is called horizon file in seismic processing and interpretation world.

## Execution
It expects at least one Salt Top/Base pair and is run as follows

>calculate_salt_thickness *T1.lmk B1.lmk [T2.lmk] [B2.lmk] .. .. [Tn.lmk] [Bn.lmk]* output=*outputsaltthicknessfile.lmk*

where T1.lmk / B1.lmk are input files with format ‘Inline Crossline X Y Z’. The arguments in [] are optional and output file is needed.

This code uses C++ standard template library, creates linked list of Top/Base pairs, works on that list at each location to trim unnecessary horizons and then travels the linked list to calculate salt thickness at each location. Yup. Definitely useful.

This problem is probably similar to other problems where something would be contained in something else. I am curious to know such problems.

## Detailed Explanation
https://amitpend.medium.com/calculate-thickness-of-salt-beneath-earths-surface-d549e2d60aa0
