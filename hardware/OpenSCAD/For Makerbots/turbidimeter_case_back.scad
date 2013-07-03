/* general variables */
	fn = 100;				//number of polygons used to approximate a curve
	io = 1;					//scale to whatever an iota or smidge is on your numeric scale
	cc = 5;					//correction factor to ensure that joins of orthogonal cylinders work as intended

	/* cuvette holder dimensions */
	bsw = 45;				//width between screw holes
	chxf = 2;				//division factor for x-coord of cuvette holder's position relative to center of case

	/* mounting screw/bolt dimensions */
	shw = 2;				//screw head radius
	ssw = 1;				//screw shaft radius
	ssh = 12;				//screw shaft height

	/* case wall */
	ww0 = 75;				//base width before minkowski (edge-rounding)
	wd0 = 136;				//base depth before minkowski (edge-rounding)
	mnk = 2;				//minkowski radius (for corner-rounding)
	ww = ww0 + mnk;		//effective width of cuvette holder
	wh = 47;				//base height
	wt = 3;					//wall thickness
	bt = 3;					//base thickness
	bhw = 2;				//base hole width
	cutx = 8;				//distance from origin for cutting (yz) plane to separate front and back case halves
	butw = 15;				//width of buttress to reinforce case-half walls
	butd = 5;				//depth of buttress to reinforce case-half walls

	/* arduino dimensions */ 
		/*NB: aruino screwholes are numbered clockwise around the board, 
		looking at it from above and taking the barrel port as 12 o'clock. 
		X and Y coords are measured relative to the board corner nearest the barrel port,
		with the X-axis aligned with the ATMega chip, to match the axes in the case bottom SCAD file*/
	aush1y = 51;			//X-coord of 1st screwhole on the arduino uno r3 board (see above)
	aush1x = 15;			//Y-coord of 1st screwhole on the arduino uno r3 board (see above)
	aush2y = 36;			//....
	aush2x = 67;
	aush3y = 8;
	aush3x = 66;
	aush4y = 2;
	aush4x = 14;
	auby = 54;				//width of arduino uno board
	aubx = 69;				//length of arduino uno board

module screwhole() {cylinder(r1 = shw, r2=ssw, h=bt, center=false, $fn=fn);}

module case_bottom()
{
	/* construction code */
	union(){
		difference(){
			minkowski(){cube([wd0,ww0,wh], center=true);cylinder(r=mnk,h=wt, $fn=fn);}
			translate([0,0,bt])	cube([wd0-wt, ww0-wt, wh], center=true);
			
			translate([-ww0/chxf -bsw/2, -bsw/2,-wh/2]) screwhole();
			translate([-ww0/chxf +bsw/2, -bsw/2,-wh/2]) screwhole();
			translate([-ww0/chxf -bsw/2, +bsw/2,-wh/2]) screwhole();
			translate([-ww0/chxf +bsw/2, +bsw/2,-wh/2]) screwhole();
			translate([wd0/2-wt-aush1x, auby/2-aush1y, -bsw/2]) screwhole();
			translate([wd0/2-wt-aush2x, auby/2-aush2y, -bsw/2]) screwhole();
			translate([wd0/2-wt-aush3x, auby/2-aush3y, -bsw/2]) screwhole();
			translate([wd0/2-wt-aush4x, auby/2-aush4y, -bsw/2]) screwhole();
	
			translate([-cutx,-ww0,-wh/2])		cube([wd0,wd0,wd0], center=false);
		}
		translate([-cutx-butw/2,-ww0/2+wt+mnk/2,bt])	cube([butw,butd,wh], center=true);
		translate([-cutx-butw/2,ww0/2-wt-mnk/2,bt])		cube([butw,butd,wh], center=true);
	}
}

case_bottom();
