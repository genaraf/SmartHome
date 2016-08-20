// Внутрений размер
len = 80;
width = 50;
height = 22;
p = 2;  // толщина стенок
d = 5;  // диаметер стоек
k1 = 10;// глубина отверстий в стойках
d1 = 2; // диаметер отверстий в стойках
mLen = 12.5;  // крепление длина
mHeigh = 12.5; // крепление шырина
mHd = 3; // диаметер отверстий в креплении
 
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
  translate([(len/2 -d /3) * x, (width/2 - d/3) * y, p / 2])
  	difference()
  	{
 		cylinder(h = height, d = d, center=true);
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

		translate([(len + p)/2 - 22, (width+p)/2, (height + p)/2 - 11])
 		cube([11, p+2, 8], center = true);

   }
	corn(1, 1);
	corn(-1, 1);
	corn(1, -1);
	corn(-1, -1);
   if(0)
   {
   	mounp_pos(0);
   	mounp_pos(1);
   	mounp_pos(2);
   	mounp_pos(3);
   }
 }
}


module box_hole(x,y)
{
	translate([(len/2 -d /3) * x, (width/2 - d/3) * y, 0])
  		cylinder(h = p + 0.5, d = d1, center=true);
}

module box_top() 
{
   difference() {
 		cube([len + p * 2, width + p * 2, p], center = true);
	box_hole(1, 1);
	box_hole(-1, 1);
	box_hole(1, -1);
	box_hole(-1, -1);

   }
}

box();
//translate([0, 0, (height + p + 6) / 2])
//box_top();
