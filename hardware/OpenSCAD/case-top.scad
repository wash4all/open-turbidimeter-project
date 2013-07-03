/*----------------OPEN TURBIDIMETER PROJECT---------------*/
/*---------------------CASE - PART 2/2 -------------------*/
/*
Developed by WASH4All, 2013. See wash4all.org for further
information about the project.
*/
/*-------------------------LEGAL--------------------------*/

/* This device and its components are licensed under a
Creative Commons Attribution-NonCommercial-ShareAlike 3.0 
Unported  (CC BY-NC-SA 3.0) licensed. In summary,
that means you are free to share (copy, distribute, and 
transmit) and remix (adapt)the work, but you must adhere to 
the following conditions:

1. Attribution —  You must attribute the work in the manner 
specified by the author or licensor (but not in any way 
that suggests that they endorse you or your use of the work).

2. Noncommercial — You may not sell the device or derivatives
or use this work for other commercial purposes. 

3. Share Alike — If you alter, transform, or build upon this 
work, you may distribute the resulting work only under the 
same or similar license to this one.

You should have received a copy of the license with this code.
If not, see: http://creativecommons.org/licenses/by-nc-sa/3.0

*/
/*--------------------------------------------------------*/
/*-----------------------VARIABLES------------------------*/
/*--------------------------------------------------------*/

// DIMENSIONS
	$fn = 20;		//curve approximation level
	io = 1;			//smidge/fudge unit
	rc = 9; 		//minkowski radius
	rh = 13.5;		//holder radius
	l_low0 = 35; 	//lower cube length
	l_hi0 = 70; 	//upper cube length
	l_mid0 = 40;	//middle cube length
	l_scr = 35;		//Nokia screen length
	wo = 80; 		//max width
	w_scr = 41;		//Nokia screen width
	t = 2;			//wall thickness
	h1 = 20;		//depth/height
	wi = wo-2*t	;	//inner width

// SHELL DIMENSIONS
	l_mid1 = l_mid0+2*t;
	l_low1 = l_low0-2*t;
	l_hi1 = l_hi0-2*t;

// SCREW MOUNTS
	scrw_d = 3;				//screwhole depth
	scrw_r = 1;				//screwhole diameter
	base_r = 3*scrw_r;		//base screw mount radius
	base_w0 = 45;			//holder width before minkowski union
	base_h = 3.5;			//depth for base screw mounts

/*--------------------------------------------------------*/
/*-------------------------MODULES------------------------*/
/*--------------------------------------------------------*/

module handheld_case(w,rc,l_mid,l_low,l_hi,h)
{
// INTERNAL VARIABLES
	w_mid = w-rc;	//midsection width

// CREATE A SOLID CASE BY COMBINING THREE BOXES AND ROUNDING EDGES
	minkowski()
	{	sphere(rc); // EDGE ROUNDING EFFECT
		union()
		{	translate([l_mid/2+1.25*rc,-w/2+rc,0]) // TOP BLOCK
			{	cube([l_hi-rc*2,w-rc*2,h-1]);
			}
			translate([-l_mid/2+.75*rc-l_low,-w/2+rc,0]) // BOTTOM BLOCK
			{	cube([l_low-rc*2,w-rc*2,h-1]);
			}
			translate([-l_mid/2-rc,-w_mid/2+rc,0]) // MIDDLE BLOCK
			{	cube([l_mid+2*rc,w_mid-rc*2,h-5]);
			}
		}
	}
}

module wash4all_logo(angle,x1,y1,z1, outer_rad, logo_depth){
	/* logo */
	fn = 100;					//yield precisely
	io = 1;						//smidge variable for fixing waterdrop
	ld = logo_depth;			//like it says
	lgsr = outer_rad/2.667;	//wash4all logo small circle radius
	lgbr = outer_rad;			//wash4all logo big circle radius
	lgai = 45;					//wash4all logo angle iterator
	translate([x1,y1,z1]){
		rotate([0,0,angle]){
			difference(){
	
				//make cylinder (which will form the body of the negative shape)
				cylinder(r=lgbr,h=ld);
	
				//cut gear teeth
				for (i = [0:7]){		
					rotate([0,0,i*45+3])	translate([-lgbr,lgbr/2,0])	cylinder(r=lgsr, h=ld, $fn=fn, center=false);
				}
		
				//top of water drop
				translate([-lgsr/2-1,0,0])	rotate([0,0,180])	cylinder(r=lgsr-io,h=ld,$fn=3,center=false);
	
				//bottom of water drop
				translate([lgsr/2-1,0,0])	cylinder(r=lgsr-io/2,h=ld,$fn=fn,center=false);
			}
		}
	}
}

/*--------------------------------------------------------*/
/*-----------------------BUILD CASE-----------------------*/
/*--------------------------------------------------------*/
difference()
{	union()
	{	difference()
		{	handheld_case(wo,rc,l_mid0,l_low0,l_hi0,h1);				// SHELL VOLUME
			translate([0,0,-t])
			{	handheld_case(wi,rc,l_mid1,l_low1,l_hi1,h1);			// INNER VOLUME
			}		 
			wash4all_logo(180,-l_mid0/2-l_low0/2-rc/4,wo/4,h1+rc-2,l_low0/3.5,20);	// LOGO
			translate([l_mid1/2+l_hi1/2,0,0]) 											
			{	cylinder(h=100,r=rh);									// CUVETTE HOLDER
			}
			translate([0,0,h1+rc-5])													
			{	cube([l_scr,w_scr,30],true);							// SCREEN
			}	
		}
		for (j = [1,-1])
		{	for (k = [1,-1])
			{	/* FILL IN EDGES UP TO BASE SURFACE FOR SCREW MOUNTS */
				intersection() // FILL IN CORNERS FOR SUPPORTS, TOP
				{	translate([l_mid1/2+l_hi1-t+io, k*(wo/2+0-t-rc/2+io), h1-rc/2])
					{	cylinder(r=base_r,h=2*h1,center=true);
					}
					translate([l_mid0/2+l_hi0-rc, k*(wo/2-rc), h1-t]) sphere(r=rc);
				}
				intersection() // FILL IN CORNERS FOR SUPPORTS, MID
				{	translate([0, k*(wi/2+0-t-rc/2+io),  h1-rc/2])
					{	cylinder(r=base_r,h=2*h1,center=true);
					}
					translate([0, k*(wi/2-rc-rc/2), h1-t-rc/2]) sphere(r=rc);
				}
				intersection() // FILL IN CORNERS FOR SUPPORTS, BOTTOM
				{	translate([-l_mid1/2-l_low1+t-io, k*(wo/2-t-rc/2+io),  h1-rc/2])
					{	cylinder(r=base_r,h=2*h1,center=true);
					}
					translate([-l_mid0/2-l_low0+rc, k*(wo/2-rc), h1-t]) sphere(r=rc);
				}
	
				translate([j*(l_scr/2+base_r/2), k*(w_scr/2+base_r/2), h1+rc/2-base_h-t])
				{	difference()									// SCREEN MOUNT
					{	cylinder(r=base_r,h=base_h); 				// SCREW MOUNTS
						cylinder(r=scrw_r,h=scrw_d+5);			// SCREW HOLES
					}
				}
				translate([l_mid1/2+l_hi1-t+io, k*(wo/2+0-t-rc/2+io), h1-rc+t-io])
				{	cylinder(r=base_r,h=base_h*2); 			// SCREW MOUNTS
				}
				translate([-l_mid1/2-l_low1+t-io, k*(wo/2-t-rc/2+io), h1-rc+t-io])
				{	cylinder(r=base_r,h=base_h*2); 			// SCREW MOUNTS
				}
				translate([0, k*(wi/2+0-t-rc/2+io), h1-rc+t-io])
				{	cylinder(r=base_r,h=base_h); 				// SCREW MOUNTS
				}
			}
		}
	}
	cube([2*(l_mid1+l_hi1+l_low1),2*wo,h1+rc/2],true);	// SLICE PLANE

	// SINKS & THRU-HOLES FOR SCREWS
	for (k = [1,-1])
	{	translate([l_mid1/2+l_hi1-t+io, k*(wo/2+0-t-rc/2+io), h1-rc/2-2])
		{	cylinder(r=scrw_r,h=base_h*5,center=true); 	// SCREW HOLES
			translate([0,0,scrw_d]) cylinder(r=base_r-1,h=base_h*5);
		}
		translate([-l_mid1/2-l_low1+t-io, k*(wo/2-t-rc/2+io), h1-rc/2-2])
		{	cylinder(r=scrw_r,h=base_h*5,center=true); 	// SCREW HOLES
			translate([0,0,scrw_d]) cylinder(r=base_r-1,h=base_h*5); 		
		}
		translate([0, k*(wi/2+0-t-rc/2+io), h1-rc/2-2])
		{	cylinder(r=scrw_r,h=base_h*5,center=true); 	// SCREW HOLES
			translate([0,0,scrw_d]) cylinder(r=base_r-1,h=base_h*5);
		}
	}
}