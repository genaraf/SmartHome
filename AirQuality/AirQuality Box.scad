top = 2; // 0-box only, 1-top only, 2-both  
mountsV=0; // enable vertical mounts
mountsH=0; // enable horisontal mounts
len = 129;   // internal length
width = 80;	 // internal width
height = 16; // internal height

p = 2;   // walls thickness
d = 6;   // racks diameter
k1 = 16; // diameter of hole in the racks
d1 = 3;  // depth of the hole in the racks

mLen = 12.5; 	// mounts length
mHeigh = 12.5; // mounts width
mHd = 3; 		// diameter of hole in the mounts

ghd = 5;  // gril hole diameter
ghs = 2; // gril hole space
ghr = 6; // gril hole resolution 3::8

module mount()
{
  	difference()
  	{
   	union()
  	  {
  		 cube([mLen - mHeigh / 2, mHeigh, p]);
		 translate([(mLen - mHeigh /2), mHeigh /2, 0])
  		 cylinder(p, d = mHeigh);
     }
 	  translate([(mLen - mHeigh /2), mHeigh /2, 0])
  	  cylinder(p, d = mHd);
   }
}

module mounp_pos(pos)
{
   if(pos == 0)
   {
   	rotate(180, 0, 0)
		translate([ (len + p) / 2, (-mHeigh/2), -(height+p) / 2])
		mount();
   } else if(pos == 1)
   {
   	rotate(90, 0, 0)
		translate([ (width + p) / 2, (-mHeigh/2), -(height+p) / 2])
		mount();
   } else if(pos == 2)
   {
   	translate([ (len + p) / 2, (-mHeigh/2), -(height+p) / 2])
		mount();
   } else if(pos == 3)
   {
  	   rotate(-90, 0, 0)
   	translate([ (width + p) / 2, (-mHeigh/2), -(height+p) / 2])
		mount();
   }
}

module corn(x, y)
{
  translate([(len/2 -d /3) * x, (width/2 - d/3) * y, p * 2])
  	difference()
  	{
 		cylinder(h = height / 2, d = d, center=true);
   	translate([0, 0, height - k1])
  		cylinder(h = k1*2, d = d1, center=true);
   }
}


module box()
{
union() {  
   difference() {
 		cube([len + p * 2, width + p * 2, height + p], center = true);
		translate([0, 0, p])
 		cube([len, width, height], center = true);
 		// left uper bottom of box
		translate([(len /2 + p), -width / 2, -height / 2]) {
			rotate([0,90,0]) {
				translate([-9, 17, 0]) {
					// USB Input
					cube([8, 11.5, p], center = true);
				}
			}
		}
  }
	corn(1, 1);
	corn(-1, 1);
	corn(1, -1);
	corn(-1, -1);
   if(mountsV)
   {
   	mounp_pos(0);
   	mounp_pos(2);
   }
   if(mountsH)
   {
   	mounp_pos(1);
   	mounp_pos(3);
	}
 }
}


module box_hole(x,y)
{
	translate([(len/2 -d /3) * x, (width/2 - d/3) * y, 0])
  		cylinder(h = p + 0.5, d = d1, center=true);
}

module gril(l, w)
{
	
	for(i = [0: ghd + ghs:l - ghd])
	{
		for(j = [0: ghd + ghs: w - ghd])
		{
			translate([i, j, 0])
				cylinder(h = k1*20, d = ghd, center=true, $fn=ghr);	
		}
	}
} 

module box_top() 
{
   difference() {
 		cube([len + p * 2, width + p * 2, p], center = true);
		box_hole(1, 1);
		box_hole(-1, 1);
		box_hole(1, -1);
		box_hole(-1, -1);
		if(ghd > 0) {
			translate([0, -27, 0]) gril(len / 2, 60);
		}
   }
}

if((top == 0) || (top == 2))
{
	box();
} 
if(top > 0)
{
	if(top == 2)
		translate([0, 0, (height + p + 6) / 2]) box_top();
	else
		translate([0, 0, p/2]) box_top();
}
