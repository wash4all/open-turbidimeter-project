module cuvette_holder(cuvette_brand)
{
	/* cuvette chamber and port heights*/
	ch = (cuvette_brand == "microtpi") ? 39 
		: (cuvette_brand == "vernier") ? 52
		: 52;										//cuvette chamber height

	ocr = (cuvette_brand == "microtpi") ? 16 
		: (cuvette_brand == "vernier") ? 16
		: 16;										//radius of outer cylinder of cuvette chamber

	icr = (cuvette_brand == "microtpi") ? 13.3 
		: (cuvette_brand == "vernier") ? 13
		: 13;										//radius of outer cylinder of cuvette chamber

	lph = (cuvette_brand == "microtpi") ? 18 
		: (cuvette_brand == "vernier") ? 25
		: 25;										//height of LED ports

	/* general variables */
	fn = 50;				//number of polygons used to approximate a curve
	io = 1;					//scale to whatever an iota or smidge is on your numeric scale
	cc = 5;					//correction factor to ensure that joins of orthogonal cylinders work as intended

	/* base and mounting holes */
	bw0 = 45;				//base width & depth before minkowski (edge-rounding)
	bwm = 5;				//radius of edge-rounding for base
	bw = bw0 + bwm;		//effective width and depth of cuvette holder
	bt = 1;					//base thickness
	bhw = 1;				//base hole width
	
	/* led ports */
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

module minicase_top(){
	mnbh = 30;
	cuvh = 42;
	hh = cuvh - mnbh;
	ocr = 16;
	wt = 3;
	difference(){
		union(){
			translate([-55,-32,0]) cube([97,77,wt]);
			translate([-55,42,0]) cube([97,wt,25]);
			translate([39,-32,0]) cube([wt,77,25]);
			translate([-55,-32,0]) cube([97,wt,25]);
			translate([-55,-32,0]) cube([wt,77,25]);
		
			translate([-52,-29,0]) cube([4,4,12]);
			translate([-52,38,0]) cube([4,4,12]);
			translate([35,-29,0]) cube([4,4,12]);
			translate([35,38,0]) cube([4,4,12]);
		}
		translate([3.5,9.5,0]) cylinder(r=16,h=wt);
	}
}


ocr = 16;
wt = 3;
difference(){
	union(){
		translate([-55,-32,0]) cube([90,70,wt]);
		translate([-55,35,0]) cube([90,wt,30]);
		translate([32,-32,0]) cube([wt,70,30]);
		translate([-55,-32,0]) cube([90,wt,30]);
		translate([-55,-32,0]) cube([wt,70,30]);
		translate([0,0,wt]) cuvette_holder("microtpi");
	}
	translate([-60,-8,6]) cube([10,16,3]);
}

//translate([-3.5,9.5,42]) rotate([0,180,180]) minicase_top();

//minicase_top();


