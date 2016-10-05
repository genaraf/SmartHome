stand_height = 57;
stand_weight = 55;
stand_bottom = 3;
feet_weight = 6;
insertion_weight = 35;
stand_top = 0;
	difference()
	{
		rotate ([90,0,0]) cylinder (h = stand_height, d=stand_weight, center = true, $fn=100);
		translate([0,0,-(stand_weight + 1)/ 2])
			cube(size = [ stand_weight*2, stand_height + 1, stand_weight], center = true);      
		translate([0,0, stand_weight / 2 + stand_bottom])
		{
			cube(size = [ stand_weight*2, stand_height - feet_weight * 2, stand_weight], center = true);
			cube(size = [ insertion_weight, stand_height + 10, stand_weight], center = true);
		}
		if(stand_top > 0)
		{      
			translate([0,0, stand_weight / 2 +stand_top])
				cube(size = [ stand_weight*2, stand_height + 1, stand_weight], center = true);
		}
	}
