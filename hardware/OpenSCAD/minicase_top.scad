module wash4all_logo(angle,x1,y1,z1, outer_rad, logo_depth){
/* logo */
fn = 100; //yield precisely
io = 1; //smidge variable for fixing waterdrop
ld = logo_depth; //like it says
lgsr = outer_rad/2.667; //wash4all logo small circle radius
lgbr = outer_rad; //wash4all logo big circle radius
lgai = 45; //wash4all logo angle iterator
translate([x1,y1,z1]){
rotate([0,0,angle]){
difference(){

//make cylinder (which will form the body of the negative shape)
cylinder(r=lgbr,h=ld);

//cut gear teeth
for (i = [0:7]){
rotate([0,0,i*45+3]) translate([-lgbr,lgbr/2,0]) cylinder(r=lgsr, h=ld, $fn=fn, center=false);
}

//top of water drop
translate([-lgsr/2-1,0,0]) rotate([0,0,180]) cylinder(r=lgsr-io,h=ld,$fn=3,center=false);

//bottom of water drop
translate([lgsr/2-1,0,0]) cylinder(r=lgsr-io/2,h=ld,$fn=fn,center=false);
}
}
}
}

mnbh = 30;
cuvh = 42;
hh = cuvh - mnbh;
ocr = 16;
wt = 3;
difference(){
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
	translate([3.5,10.5,0]) cylinder(r=16.25,h=wt);
}


wash4all_logo(180,-35,-10,0, 12, 1.5);
}
