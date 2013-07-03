/*----------------OPEN TURBIDIMETER PROJECT---------------*/
/*---------------------HOLDER - PART 1/1--- -------------------*/
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
module cuvette_holder()
{
	/* general variables */
	fn = 100;				//number of polygons used to approximate a curve
	io = 1;					//scale to whatever an iota or smidge is on your numeric scale
	cc = 5;					//correction factor to ensure that joins of orthogonal cylinders work as intended

	/* base and mounting holes */
	bw0 = 45;				//base width & depth before minkowski (edge-rounding)
	bwm = 5;				//radius of edge-rounding for base
	bw = bw0 + bwm;		//effective width and depth of cuvette holder
	bt = 1;					//base thickness
	bhw = 1;				//base hole width
	
	/* cuvette chamber */
	ch = 52;				//cuvette chamber height
	ocr = 16;				//radius of outer cylinder of cuvette chamber
	icr = 13;				//radius of inner cylinder of cuvette chamber

	/* led ports */
	lph = 25;				//height of LED ports
	ilpr = 2.5;				//inner radius of LED ports
	ilpd = 9;				//inner depth of LED ports 
	mlpr = 3.5;			//middle radius of LED ports
	mlpd = 2;				//middle depth of LED ports
	olpr = 5;				//outer radius of LED ports
	olpd = ilpd + mlpd;	//outer depth of LED ports

	/* TSL230R sensor ports --using LED height for sensor mounting height */
	isph = 3.5;				//inner height of sensor ports
	ispw = 3.5;				//inner width of sensor ports
	ispd = .75;				//inner depth of sensor ports
	msph = 11;			//middle height of sensor ports
	mspw = 6.5;			//middle width of sensor ports
	mspd = 2;				//middle depth of sensor ports
	osph = 15;				//outer height of sensor ports
	ospw = 10;				//outer width of sensor ports
	ospd = ispd + mspd;	//outer depth of sensor ports
	ossd = 5;
	osgd = 2.5;
	osgw = 6;
	
	/* construction code */
	difference(){
		union(){
			//cube([bw0,bw0,bt], center=true);
			minkowski(){cube([bw0,bw0,bt], center=true);cylinder(r=bwm,h=bt, $fn=fn);}
			translate([0,0,ch/2])							cylinder(h=ch, r1=ocr, r2=ocr, center = true, $fn=fn);
			translate([0,ocr-cc,lph])	rotate([-90,0,0])	cylinder(h=olpd+cc, r1=olpr, r2=olpr, center = false, $fn=fn);
			translate([ocr,0,lph])     	rotate([0,90,0])	cylinder(h=2*olpd, r1=olpr, r2=olpr, center = true, $fn=fn);
			translate([-ospw/2,-ocr+cc,lph-osph/2])	rotate([0,0,-90])		cube([ospd+cc, ospw, osph], center=false);
			translate([-ocr+cc,ospw/2,lph-osph/2]) 	rotate([0,0,180])	cube([ospd+cc, ospw, osph], center=false);
			translate([ocr-io,-ilpr,0])					cube([olpd+io, ilpr*2,lph], center=false);
			translate([-ilpr,ocr-io,0])					cube([ilpr*2,olpd+io,lph], center=false);
			translate([-ocr-ospd,-ospw/2,0])			cube([ospd+io,ospw,lph], center=false);
			translate([-ospw/2,-ocr-ospd-ossd,0])				cube([ospw,ospd+ossd+io,lph-msph/2-io], center=false);
			translate([-osgw/2,-ocr-ospd-ossd,lph-msph/2-io])	cube([osgw,ossd-osgd,osph], center=false);
			translate([-ocr-ospd-ossd,-ospw/2,0])				cube([ospd+ossd+io,ospw,lph-msph/2-io], center=false);
			translate([-ocr-ospd-ossd,-osgw/2,lph-msph/2-io])	cube([ossd-osgd,osgw,osph], center=false);
		}
		translate([0,ocr-cc,lph])	 	rotate([-90,0,0])  cylinder(h=ilpd+cc, r1=ilpr, r2=ilpr, center = false, $fn=fn);
		translate([0,ocr+ilpd,lph])  	rotate([-90,0,0])  cylinder(h=mlpd+io, r1=mlpr, r2=mlpr, center = false, $fn=fn);
		translate([ocr-cc,0,lph]) 		rotate([0,90,0])   cylinder(h=ilpd+cc, r1=ilpr, r2=ilpr, center = false, $fn=fn);
		translate([ocr+ilpd,0,lph])		rotate([0,90,0])   cylinder(h=mlpd+io, r1=mlpr, r2=mlpr, center = false, $fn=fn);
		translate([-ocr-ispd,mspw/2,lph-msph/2])		rotate([0,0,180])	cube([mspd+io,mspw,msph], center=false);
		translate([-ocr+cc,ispw/2,lph-isph/2]) 		rotate([0,0,180])	cube([ispd+cc+io,ispw,isph], center=false);
		translate([-mspw/2,-ocr-ispd,lph-msph/2])	rotate([0,0,-90])	cube([mspd+io, mspw,msph], center=false);
		translate([-ispw/2,-ocr+cc,lph-isph/2]) 		rotate([0,0,-90])	cube([ispd+cc,ispw,isph], center=false);
			

		translate([0,0,ch/2+2*bt])							cylinder(h=ch, r1=icr, r2=icr, center = true, $fn=fn);
		translate([bw0/2 - io,bw0/2 - io,-5]) 				cylinder(h=10, r1=bhw, r2=bhw, $fn=fn);
		translate([-bw0/2 + io,bw0/2 - io,-5]) 				cylinder(h=10, r1=bhw, r2=bhw, $fn=fn);
		translate([bw0/2 - io,-bw0/2 + io,-5]) 				cylinder(h=10, r1=bhw, r2=bhw, $fn=fn);
		translate([-bw0/2 + io,-bw0/2 + io,-5]) 			cylinder(h=10, r1=bhw, r2=bhw, $fn=fn);
	}
}

cuvette_holder();